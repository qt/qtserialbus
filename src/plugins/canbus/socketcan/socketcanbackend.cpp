/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "socketcanbackend.h"
#include <QtSerialBus/qcanbusdevice.h>

#include <QtCore/qdebug.h>
#include <QtCore/qdatastream.h>
#include <QtCore/qsocketnotifier.h>

#include <linux/can/error.h>
#include <linux/can/raw.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/time.h>

QT_BEGIN_NAMESPACE

SocketCanBackend::SocketCanBackend(const QString &name) :
    canSocket(-1),
    notifier(0),
    canSocketName(name)
{
    resetConfigurations();
}

SocketCanBackend::~SocketCanBackend()
{
    close();
}

void SocketCanBackend::resetConfigurations()
{
    QCanBusDevice::setConfigurationParameter(
                QCanBusDevice::LoopbackKey, true);
    QCanBusDevice::setConfigurationParameter(
                QCanBusDevice::ReceiveOwnKey, false);
    QCanBusDevice::setConfigurationParameter(
                QCanBusDevice::ErrorFilterKey, QCanBusFrame::NoError);
    QCanBusDevice::setConfigurationParameter(
                QCanBusDevice::RawFilterKey, QList<QVariant>());
}

bool SocketCanBackend::open()
{
    if (canSocket == -1) {
        if (!connectSocket()) {
            close(); // sets UnconnectedState
            return false;
        }
    }

    setState(QCanBusDevice::ConnectedState);
    return true;
}

void SocketCanBackend::close()
{
    ::close(canSocket);
    canSocket = -1;

    resetConfigurations();

    setState(QCanBusDevice::UnconnectedState);
}

void SocketCanBackend::setConfigurationParameter(int key, const QVariant &value)
{

    if (key == QCanBusDevice::LoopbackKey) {
        const int loopback = value.toBool() ? 1 : 0;
        if (setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback)) < 0) {
            setError(qt_error_string(errno),
                     QCanBusDevice::CanBusError::ConfigurationError);
            return;
        }
    } else if (key == QCanBusDevice::ReceiveOwnKey) {
        const int receiveOwnMessages = value.toBool() ? 1 : 0;
        if (setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS,
                       &receiveOwnMessages, sizeof(receiveOwnMessages)) < 0) {
            setError(qt_error_string(errno),
                     QCanBusDevice::CanBusError::ConfigurationError);
            return;
        }
    } else if (key == QCanBusDevice::ErrorFilterKey) {
        const int errorMask = value.value<QCanBusFrame::FrameErrors>();
        if (setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_ERR_FILTER,
                       &errorMask, sizeof(errorMask)) < 0) {
            setError(qt_error_string(errno),
                     QCanBusDevice::CanBusError::ConfigurationError);
            return;
        }
    } else if (key == QCanBusDevice::RawFilterKey) {
        const QList<QVariant> filterList = value.toList();
        const int size = filterList.size();
        if (size == 0)
            setError(QStringLiteral("ERROR SocketCanBackend: \"CanFilter\" "
                                    "QList<QVariant> empty or not valid"),
                     QCanBusDevice::CanBusError::ConfigurationError);
        can_filter filters[size];
        for (int i = 0; i < size; i++) {
            can_filter filter;
            const QHash<QString, QVariant> filterHash = filterList.at(i).toHash();
            bool ok = true;
            filter.can_id = filterHash.value("FilterId").toInt(&ok);
            if (!ok) {
                setError(QStringLiteral("ERROR SocketCanBackend: \"CanFilter\" "
                                        "FilterId key not found or value is not valid in index: ")
                                        + QString::number(i),
                         QCanBusDevice::CanBusError::ConfigurationError);
                return;
            }
            filter.can_mask = filterHash.value("CanMask").toInt(&ok);
            if (!ok) {
                setError(QStringLiteral("ERROR SocketCanBackend: \"CanFilter\" "
                                        "CanMask key not found or value is not valid in index:")
                                        + QString::number(i),
                         QCanBusDevice::CanBusError::ConfigurationError);
                return;
            }
            filters[i] = filter;
        }
        if (setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_FILTER, filters, sizeof(filters)) < 0)
            setError(qt_error_string(errno),
                     QCanBusDevice::CanBusError::ConfigurationError);
    } else {
        setError(QStringLiteral("SocketCanBackend: No such configuration as")
                   + key + QStringLiteral("in SocketCanBackend"),
                 QCanBusDevice::CanBusError::ConfigurationError);
        return;
    }

    QCanBusDevice::setConfigurationParameter(key, value);
}

bool SocketCanBackend::connectSocket()
{
    struct sockaddr_can address;
    struct ifreq interface;

    if ((canSocket = socket(PF_CAN, SOCK_RAW | SOCK_NONBLOCK, CAN_RAW)) < 0) {
        setError(qt_error_string(errno),
                 QCanBusDevice::CanBusError::ConnectionError);
        return false;
    }

    strcpy(interface.ifr_name, canSocketName.toLatin1().data());
    if (ioctl(canSocket, SIOCGIFINDEX, &interface) < 0) {
        setError(qt_error_string(errno),
                 QCanBusDevice::CanBusError::ConnectionError);
        return false;
    }

    address.can_family  = AF_CAN;
    address.can_ifindex = interface.ifr_ifindex;

    if (bind(canSocket, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) < 0) {
        setError(qt_error_string(errno),
                 QCanBusDevice::CanBusError::ConnectionError);
        return false;
    }

    const int fd_frames = 1;
    if (setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &fd_frames, sizeof(fd_frames)) < 0) {
        setError(qt_error_string(errno),
                 QCanBusDevice::CanBusError::ConnectionError);
        return false;
    }

    if (notifier)
        delete notifier;

    notifier = new QSocketNotifier(canSocket, QSocketNotifier::Read, this);
    connect(notifier.data(), &QSocketNotifier::activated,
            this, &SocketCanBackend::readSocket);

    return true;
}

bool SocketCanBackend::writeFrame(const QCanBusFrame &newData)
{
    if (state() != ConnectedState)
        return false;

    canfd_frame frame;
    frame.can_id = newData.frameId();
    if (newData.hasExtendedFrameFormat())
        frame.can_id |= CAN_EFF_FLAG;
    if (newData.frameType() == QCanBusFrame::ErrorFrame) {
        frame.can_id = (uint)(newData.error() & QCanBusFrame::AnyError);
        frame.can_id |= CAN_ERR_FLAG;
    }
    if (newData.frameType() == QCanBusFrame::RemoteRequestFrame)
        frame.can_id |= CAN_RTR_FLAG;

    frame.len = newData.payload().size();
    for (int i = 0; i < frame.len ; i++)
        frame.data[i] = newData.payload().at(i);

    const qint64 bytesWritten = ::write(canSocket, &frame, CANFD_MTU);
    if (bytesWritten < 0) {
        setError(qt_error_string(errno),
                 QCanBusDevice::CanBusError::WriteError);
        return false;
    }

    return true;
}

QString SocketCanBackend::interpretErrorFrame(const QCanBusFrame &errorFrame)
{
    if (errorFrame.frameType() != QCanBusFrame::ErrorFrame)
        return QString();

    // the payload may contain the error details
    const QByteArray data = errorFrame.payload();
    QString errorMsg;

    if (errorFrame.error() & QCanBusFrame::TransmissionTimeoutError)
        errorMsg += QStringLiteral("TX timout\n");

    if (errorFrame.error() & QCanBusFrame::MissingAcknowledgmentError)
        errorMsg += QStringLiteral("Received no ACK on transmission\n");

    if (errorFrame.error() & QCanBusFrame::BusOffError)
        errorMsg += QStringLiteral("Bus off\n");

    if (errorFrame.error() & QCanBusFrame::BusError)
        errorMsg += QStringLiteral("Bus error\n");

    if (errorFrame.error() & QCanBusFrame::ControllerRestartError)
        errorMsg += QStringLiteral("Controller restarted\n");

    if (errorFrame.error() & QCanBusFrame::UnknownError)
        errorMsg += QStringLiteral("Unknown error\n");

    if (errorFrame.error() & QCanBusFrame::LostArbitrationError) {
        errorMsg += QStringLiteral("Lost arbitration:\n");
        if (data.size() >= 1) {
            errorMsg += QString::number(data.at(0), 16);
            errorMsg += QStringLiteral(" bit\n");
        }
    }

    if (errorFrame.error() & QCanBusFrame::ControllerError) {
        errorMsg += QStringLiteral("Controller problem:\n");
        if (data.size() >= 2) {
            char b = data.at(1) ;
            if (b & CAN_ERR_CRTL_RX_OVERFLOW)
                errorMsg += QStringLiteral(" RX buffer overflow\n");
            if (b & CAN_ERR_CRTL_TX_OVERFLOW)
                errorMsg += QStringLiteral(" TX buffer overflow\n");
            if (b & CAN_ERR_CRTL_RX_WARNING)
                errorMsg += QStringLiteral(" reached warning level for RX errors\n");
            if (b & CAN_ERR_CRTL_TX_WARNING)
                errorMsg += QStringLiteral(" reached warning level for TX errors\n");
            if (b & CAN_ERR_CRTL_RX_PASSIVE)
                errorMsg += QStringLiteral(" reached error passive status RX\n");
            if (b & CAN_ERR_CRTL_TX_PASSIVE)
                errorMsg += QStringLiteral(" reached error passive status TX\n");

            if (b == CAN_ERR_CRTL_UNSPEC)
                errorMsg += QStringLiteral(" Unspecified error\n");
        }
    }

    if (errorFrame.error() & QCanBusFrame::TransceiverError) {
        errorMsg = QStringLiteral("Transceiver status:");
        if (data.size() >= 5) {
            char b = data.at(4);
            if (b & CAN_ERR_TRX_CANH_NO_WIRE)
                errorMsg += QStringLiteral(" CAN-transceiver CANH no wire\n");
            if (b & CAN_ERR_TRX_CANH_SHORT_TO_BAT)
                errorMsg += QStringLiteral(" CAN-transceiver CANH short to bat\n");
            if (b & CAN_ERR_TRX_CANH_SHORT_TO_VCC)
                errorMsg += QStringLiteral(" CAN-transceiver CANH short to vcc\n");
            if (b & CAN_ERR_TRX_CANH_SHORT_TO_GND)
                errorMsg += QStringLiteral(" CAN-transceiver CANH short to ground\n");
            if (b & CAN_ERR_TRX_CANL_NO_WIRE)
                errorMsg += QStringLiteral(" CAN-transceiver CANL no wire\n");
            if (b & CAN_ERR_TRX_CANL_SHORT_TO_BAT)
                errorMsg += QStringLiteral(" CAN-transceiver CANL short to bat\n");
            if (b & CAN_ERR_TRX_CANL_SHORT_TO_VCC)
                errorMsg += QStringLiteral(" CAN-transceiver CANL short to vcc\n");
            if (b & CAN_ERR_TRX_CANL_SHORT_TO_GND)
                errorMsg += QStringLiteral(" CAN-transceiver CANL short to ground\n");
            if (b & CAN_ERR_TRX_CANL_SHORT_TO_CANH)
                errorMsg += QStringLiteral(" CAN-transceiver CANL short to CANH\n");

            if (b == CAN_ERR_TRX_UNSPEC)
                errorMsg += QStringLiteral(" unspecified\n");
        }

    }

    if (errorFrame.error() & QCanBusFrame::ProtocolViolationError) {
        errorMsg += QStringLiteral("Protocol violation:\n");
        if (data.size() > 3) {
            char b = data.at(2);
            if (b & CAN_ERR_PROT_BIT)
                errorMsg += QStringLiteral(" single bit error\n");
            if (b & CAN_ERR_PROT_FORM)
                errorMsg += QStringLiteral(" frame format error\n");
            if (b & CAN_ERR_PROT_STUFF)
                errorMsg += QStringLiteral(" bit stuffing error\n");
            if (b & CAN_ERR_PROT_BIT0)
                errorMsg += QStringLiteral(" unable to send dominant bit\n");
            if (b & CAN_ERR_PROT_BIT1)
                errorMsg += QStringLiteral(" unable to send recessive bit\n");
            if (b & CAN_ERR_PROT_OVERLOAD)
                errorMsg += QStringLiteral(" bus overload\n");
            if (b & CAN_ERR_PROT_ACTIVE)
                errorMsg += QStringLiteral(" active error announcement\n");
            if (b & CAN_ERR_PROT_TX)
                errorMsg += QStringLiteral(" error occurred on transmission\n");

            if (b == CAN_ERR_PROT_UNSPEC)
                errorMsg += QStringLiteral(" unspecified\n");
        }
        if (data.size() > 4) {
            char b = data.at(3);
            if (b == CAN_ERR_PROT_LOC_SOF)
                errorMsg += QStringLiteral(" start of frame\n");
            if (b == CAN_ERR_PROT_LOC_ID28_21)
                errorMsg += QStringLiteral(" ID bits 28 - 21 (SFF: 10 - 3)\n");
            if (b == CAN_ERR_PROT_LOC_ID20_18)
                errorMsg += QStringLiteral(" ID bits 20 - 18 (SFF: 2 - 0 )\n");
            if (b == CAN_ERR_PROT_LOC_SRTR)
                errorMsg += QStringLiteral(" substitute RTR (SFF: RTR)\n");
            if (b == CAN_ERR_PROT_LOC_IDE)
                errorMsg += QStringLiteral(" identifier extension\n");
            if (b == CAN_ERR_PROT_LOC_ID17_13)
                errorMsg += QStringLiteral(" ID bits 17-13\n");
            if (b == CAN_ERR_PROT_LOC_ID12_05)
                errorMsg += QStringLiteral(" ID bits 12-5\n");
            if (b == CAN_ERR_PROT_LOC_ID04_00)
                errorMsg += QStringLiteral(" ID bits 4-0\n");
            if (b == CAN_ERR_PROT_LOC_RTR)
                errorMsg += QStringLiteral(" RTR\n");
            if (b == CAN_ERR_PROT_LOC_RES1)
                errorMsg += QStringLiteral(" reserved bit 1\n");
            if (b == CAN_ERR_PROT_LOC_RES0)
                errorMsg += QStringLiteral(" reserved bit 0\n");
            if (b == CAN_ERR_PROT_LOC_DLC)
                errorMsg += QStringLiteral(" data length code\n");
            if (b == CAN_ERR_PROT_LOC_DATA)
                errorMsg += QStringLiteral(" data section\n");
            if (b == CAN_ERR_PROT_LOC_CRC_SEQ)
                errorMsg += QStringLiteral(" CRC sequence\n");
            if (b == CAN_ERR_PROT_LOC_CRC_DEL)
                errorMsg += QStringLiteral(" CRC delimiter\n");
            if (b == CAN_ERR_PROT_LOC_ACK)
                errorMsg += QStringLiteral(" ACK slot\n");
            if (b == CAN_ERR_PROT_LOC_ACK_DEL)
                errorMsg += QStringLiteral(" ACK delimiter\n");
            if (b == CAN_ERR_PROT_LOC_EOF)
                errorMsg += QStringLiteral(" end of frame\n");
            if (b == CAN_ERR_PROT_LOC_INTERM)
                errorMsg += QStringLiteral(" Intermission\n");

            if (b == CAN_ERR_PROT_LOC_UNSPEC)
                errorMsg += QStringLiteral(" unspecified\n");
        }
    }

    // cut trailing \n
    if (!errorMsg.isEmpty())
        errorMsg.chop(1);

    return errorMsg;
}

void SocketCanBackend::readSocket()
{
    while (true) {
        struct canfd_frame frame;
        int bytesReceived = 0;

        bytesReceived = ::read(canSocket, &frame, CANFD_MTU);

        if (bytesReceived <= 0) {
            break;
        } else if (!bytesReceived == CANFD_MTU) {
            setError(QStringLiteral("ERROR SocketCanBackend: invalid can frame"),
                     QCanBusDevice::CanBusError::ReadError);
            continue;
        }

        struct timeval timeStamp;
        if (ioctl(canSocket, SIOCGSTAMP, &timeStamp) < 0) {
            setError(qt_error_string(errno),
                     QCanBusDevice::CanBusError::ReadError);
            timeStamp.tv_sec = 0;
            timeStamp.tv_usec = 0;
        }

        QCanBusFrame::TimeStamp stamp;
        stamp.setSeconds(timeStamp.tv_sec);
        stamp.setMicroSeconds(timeStamp.tv_usec);

        QCanBusFrame bufferedFrame;
        bufferedFrame.setTimeStamp(stamp);

        bufferedFrame.setExtendedFrameFormat(frame.can_id & CAN_EFF_FLAG);
        if (frame.can_id & CAN_RTR_FLAG)
            bufferedFrame.setFrameType(QCanBusFrame::RemoteRequestFrame);
        if (frame.can_id & CAN_ERR_FLAG)
            bufferedFrame.setFrameType(QCanBusFrame::ErrorFrame);

        bufferedFrame.setFrameId(frame.can_id & CAN_EFF_MASK);

        QByteArray load;
        for (int i = 0; i < frame.len ; i++)
            load.insert(i, frame.data[i]);
        bufferedFrame.setPayload(load);

        enqueueReceivedFrame(bufferedFrame);
    }
}

QT_END_NAMESPACE
