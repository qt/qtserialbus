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
#include "qcanbusdevice.h"

#include <QtCore/qdebug.h>
#include <QtCore/qdatastream.h>
#include <QtCore/qsocketnotifier.h>

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
    configuration.append(QPair<QString, QVariant>(QStringLiteral("LoopBack"), 1));
    configuration.append(QPair<QString, QVariant>(QStringLiteral("ReceiveOwnMessages"), 0));
    configuration.append(QPair<QString, QVariant>(QStringLiteral("ErrorMask"), 0));
    configuration.append(QPair<QString, QVariant>(QStringLiteral("CanFilter"), QList<QVariant>()));
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

void SocketCanBackend::insertInConfigurations(const QString &key, const QVariant &value)
{
    for (int i = 0; i < configuration.size(); i++) {
        if (configuration.at(i).first == key) {
            QPair<QString, QVariant> conf(key, value);
            configuration[i] = conf;
            return;
        }
    }
}

void SocketCanBackend::setConfigurationParameter(const QString &key, const QVariant &value)
{
    if (key == QStringLiteral("Loopback")) {
        const int loopback = value.toBool() ? 1 : 0;
        if (setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback)) < 0) {
            setError(qt_error_string(errno),
                     QCanBusDevice::CanBusError::ConfigurationError);
            return;
        }
    } else if (key == QStringLiteral("ReceiveOwnMessages")) {
        const int receiveOwnMessages = value.toBool() ? 1 : 0;
        if (setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS,
                       &receiveOwnMessages, sizeof(receiveOwnMessages)) < 0) {
            setError(qt_error_string(errno),
                     QCanBusDevice::CanBusError::ConfigurationError);
            return;
        }
    } else if (key == QStringLiteral("ErrorMask")) {
        const int errorMask = value.toInt();
        if (setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_ERR_FILTER,
                       &errorMask, sizeof(errorMask)) < 0) {
            setError(qt_error_string(errno),
                     QCanBusDevice::CanBusError::ConfigurationError);
            return;
        }
    } else if (key == QStringLiteral("CanFilter")) {
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
    insertInConfigurations(key, value);
}

QVariant SocketCanBackend::configurationParameter(const QString &key) const
{
    for (int i = 0; i < configuration.size(); i++)
        if (configuration.at(i).first == key)
            return configuration.at(i).second;

    return QVariant();
}

QVector<QString> SocketCanBackend::configurationKeys() const
{
    QVector<QString> keys;
    for (int i = 0; i < configuration.size(); i++)
        keys.append(configuration.at(i).first);

    return keys;
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

qint64 SocketCanBackend::availableFrames() const
{
    return frameBuffer.size();
}

QCanFrame SocketCanBackend::readFrame()
{
    if (state() != ConnectedState)
        return QCanFrame();

    if (frameBuffer.isEmpty())
        return QCanFrame(); //TODO add concept of invalid CanFrame

    return frameBuffer.takeFirst();
}

bool SocketCanBackend::writeFrame(const QCanFrame &newData)
{
    if (state() != ConnectedState)
        return false;

    canfd_frame frame;
    frame.can_id = newData.frameId();
    if (newData.hasExtendedFrameFormat())
        frame.can_id |= CAN_EFF_FLAG;
    if (newData.frameType() == QCanFrame::ErrorFrame)
        frame.can_id |= CAN_ERR_FLAG;
    if (newData.frameType() == QCanFrame::RemoteRequestFrame)
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

        QCanFrame::TimeStamp stamp;
        stamp.setSeconds(timeStamp.tv_sec);
        stamp.setMicroSeconds(timeStamp.tv_usec);

        QCanFrame bufferedFrame;
        bufferedFrame.setTimeStamp(stamp);

        bufferedFrame.setExtendedFrameFormat(frame.can_id & CAN_EFF_FLAG);
        if (frame.can_id & CAN_RTR_FLAG)
            bufferedFrame.setFrameType(QCanFrame::RemoteRequestFrame);
        if (frame.can_id & CAN_ERR_FLAG)
            bufferedFrame.setFrameType(QCanFrame::ErrorFrame);

        bufferedFrame.setFrameId(frame.can_id & CAN_EFF_MASK);

        QByteArray load;
        for (int i = 0; i < frame.len ; i++)
            load.insert(i, frame.data[i]);
        bufferedFrame.setPayload(load);

        frameBuffer.append(bufferedFrame);
    }
    emit frameReceived();
}

QT_END_NAMESPACE
