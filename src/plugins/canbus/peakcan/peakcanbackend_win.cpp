/****************************************************************************
**
** Copyright (C) 2015 Denis Shienkov <denis.shienkov@gmail.com>
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

#include "peakcanbackend_p.h"
#include "symbols_win_p.h"

#include <QtCore/qwineventnotifier.h>
#include <QtCore/qtimer.h>
#include <QtCore/qcoreevent.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

#ifndef LINK_LIBPCANBASIC
Q_GLOBAL_STATIC(QLibrary, pcanLibrary)
#endif

bool PeakCanBackend::canCreate(QString *errorReason)
{
#ifdef LINK_LIBPCANBASIC
    return true;
#else
    static bool symbolsResolved = resolveSymbols(pcanLibrary());
    if (!symbolsResolved) {
        *errorReason = tr("The PCAN runtime library is not found");
        return false;
    }
    return true;
#endif
}

class IncomingEventNotifier : public QWinEventNotifier
{
public:
    explicit IncomingEventNotifier(PeakCanBackendPrivate *d, QObject *parent)
        : QWinEventNotifier(parent)
        , dptr(d)
    {
    }

protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE
    {
        if (e->type() == QEvent::WinEventAct) {
            dptr->canReadNotification();
            return true;
        }
        return QWinEventNotifier::event(e);
    }

private:
    PeakCanBackendPrivate *dptr;
};

class OutgoingEventNotifier : public QTimer
{
public:
    OutgoingEventNotifier(PeakCanBackendPrivate *d, QObject *parent)
        : QTimer(parent)
        , dptr(d)
    {
    }

protected:
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE
    {
        if (e->timerId() == timerId()) {
            dptr->canWriteNotification();
            return;
        }
        QTimer::timerEvent(e);
    }

private:
    PeakCanBackendPrivate *dptr;
};

PeakCanBackendPrivate::PeakCanBackendPrivate(PeakCanBackend *q)
    : q_ptr(q)
    , isOpen(false)
    , channelIndex(PCAN_NONEBUS)
    , incomingEventHandle(INVALID_HANDLE_VALUE)
{
}

struct BitrateItem
{
    int bitrate;
    int code;
};

struct BitrateLessFunctor
{
    bool operator()( const BitrateItem &item1, const BitrateItem &item2) const
    {
        return item1.bitrate < item2.bitrate;
    }
};

static int bitrateCodeFromBitrate(int bitrate)
{
    static const BitrateItem bitratetable[] = {
        { 5000, PCAN_BAUD_5K },
        { 10000, PCAN_BAUD_10K },
        { 20000, PCAN_BAUD_20K },
        { 33000, PCAN_BAUD_33K },
        { 47000, PCAN_BAUD_47K },
        { 50000, PCAN_BAUD_50K },
        { 83000, PCAN_BAUD_83K },
        { 95000, PCAN_BAUD_95K },
        { 100000, PCAN_BAUD_100K },
        { 125000, PCAN_BAUD_125K },
        { 250000, PCAN_BAUD_250K },
        { 500000, PCAN_BAUD_500K },
        { 800000, PCAN_BAUD_800K },
        { 1000000, PCAN_BAUD_1M }
    };

    static const BitrateItem *endtable = bitratetable + (sizeof(bitratetable) / sizeof(*bitratetable));

    const BitrateItem item = { bitrate , 0 };
    const BitrateItem *where = std::lower_bound(bitratetable, endtable, item, BitrateLessFunctor());
    return where != endtable ? where->code : -1;
}

bool PeakCanBackendPrivate::open()
{
    Q_Q(PeakCanBackend);

    const int bitrate = q->configurationParameter(QCanBusDevice::BitRateKey).toInt();
    const int bitrateCode = bitrateCodeFromBitrate(bitrate);

    if (TPCANStatus st = ::CAN_Initialize(channelIndex, bitrateCode, 0, 0, 0) != PCAN_ERROR_OK) {
        q->setError(systemErrorString(st), QCanBusDevice::ConnectionError);
        return false;
    }

    if (!enableReadNotification()) {
        if (TPCANStatus st = ::CAN_Uninitialize(channelIndex) != PCAN_ERROR_OK)
            q->setError(systemErrorString(st), QCanBusDevice::ConnectionError);
        return false;
    }

    isOpen = true;
    return true;
}

void PeakCanBackendPrivate::close()
{
    Q_Q(PeakCanBackend);

    enableWriteNotification(false);

    if (incomingEventNotifier) {
        incomingEventNotifier->setEnabled(false);
        incomingEventNotifier->deleteLater();
    }

    if (incomingEventHandle != INVALID_HANDLE_VALUE) {
        ::CloseHandle(incomingEventHandle);
        incomingEventHandle = INVALID_HANDLE_VALUE;
    }

    if (outgoingEventNotifier)
        outgoingEventNotifier->deleteLater();

    if (TPCANStatus st = ::CAN_Uninitialize(channelIndex) != PCAN_ERROR_OK)
        emit q->setError(systemErrorString(st), QCanBusDevice::ConnectionError);

    isOpen = false;
}

bool PeakCanBackendPrivate::setConfigurationParameter(int key, const QVariant &value)
{
    Q_Q(PeakCanBackend);

    switch (key) {
    case QCanBusDevice::BitRateKey:
        return verifyBitRate(value.toInt());
    default:
        q->setError(PeakCanBackend::tr("Unsuported configuration key"), QCanBusDevice::ConfigurationError);
        return false;
    }
}

static int channelIndexFromName(const QString &interfaceName)
{
    if (interfaceName == QStringLiteral("usbbus1"))
        return PCAN_USBBUS1;
    else if (interfaceName == QStringLiteral("usbbus2"))
        return PCAN_USBBUS2;
    else if (interfaceName == QStringLiteral("usbbus3"))
        return PCAN_USBBUS3;
    else if (interfaceName == QStringLiteral("usbbus4"))
        return PCAN_USBBUS4;
    else if (interfaceName == QStringLiteral("usbbus5"))
        return PCAN_USBBUS5;
    else if (interfaceName == QStringLiteral("usbbus6"))
        return PCAN_USBBUS6;
    else if (interfaceName == QStringLiteral("usbbus7"))
        return PCAN_USBBUS7;
    else if (interfaceName == QStringLiteral("usbbus8"))
        return PCAN_USBBUS8;
    else // TODO: Add other indexes here
        return PCAN_NONEBUS;
}

void PeakCanBackendPrivate::setupChannel(const QString &interfaceName)
{
    channelIndex = channelIndexFromName(interfaceName);
}

// Calls only when the device is closed
void PeakCanBackendPrivate::setupDefaultConfigurations()
{
    Q_Q(PeakCanBackend);

    q->setConfigurationParameter(QCanBusDevice::BitRateKey, 500000);
}

QString PeakCanBackendPrivate::systemErrorString(int errorCode)
{
    QByteArray buffer(256, 0);
    if (::CAN_GetErrorText(errorCode, 0, reinterpret_cast<LPSTR>(buffer.data())) != PCAN_ERROR_OK)
        return PeakCanBackend::tr("Unable to retrieve an error string");
    return QString::fromLatin1(buffer);
}

void PeakCanBackendPrivate::enableWriteNotification(bool enable)
{
    Q_Q(PeakCanBackend);

    if (outgoingEventNotifier) {
        if (enable) {
            if (!outgoingEventNotifier->isActive())
                outgoingEventNotifier->start();
        } else {
            outgoingEventNotifier->stop();
        }
    } else if (enable) {
        outgoingEventNotifier = new OutgoingEventNotifier(this, q);
        outgoingEventNotifier->setInterval(0);
        outgoingEventNotifier->start();
    }
}

void PeakCanBackendPrivate::canWriteNotification()
{
    Q_Q(PeakCanBackend);

    if (outgoingFrames.isEmpty()) {
        enableWriteNotification(false);
        return;
    }

    const QCanBusFrame frame = outgoingFrames.takeFirst();
    const QByteArray payload = frame.payload();

    TPCANMsg message;
    ::memset(&message, 0, sizeof(message));

    message.ID = frame.frameId();
    message.LEN = payload.size();
    message.MSGTYPE = frame.hasExtendedFrameFormat() ? PCAN_MESSAGE_EXTENDED : PCAN_MESSAGE_STANDARD;

    if (frame.frameType() == QCanBusFrame::RemoteRequestFrame)
        message.MSGTYPE |= PCAN_MESSAGE_RTR; // we do not care about the payload
    else
        ::memcpy(message.DATA, payload.constData(), sizeof(message.DATA));

    if (TPCANStatus st = ::CAN_Write(channelIndex, &message) != PCAN_ERROR_OK) {
        q->setError(systemErrorString(st), QCanBusDevice::WriteError);
    } else {
        // TODO: Emit the future signal that the frame has been written
    }

    if (!outgoingFrames.isEmpty())
        enableWriteNotification(true);
}

bool PeakCanBackendPrivate::enableReadNotification()
{
    Q_Q(PeakCanBackend);

    if (incomingEventHandle == INVALID_HANDLE_VALUE) {
        incomingEventHandle = ::CreateEvent(Q_NULLPTR, FALSE, FALSE, Q_NULLPTR);
        if (!incomingEventHandle) {
            q->setError(qt_error_string(::GetLastError()), QCanBusDevice::ReadError);
            return false;
        }
    }

    if (TPCANStatus st = ::CAN_SetValue(channelIndex, PCAN_RECEIVE_EVENT, &incomingEventHandle, sizeof(incomingEventHandle))
            != PCAN_ERROR_OK) {
        q->setError(systemErrorString(st), QCanBusDevice::ReadError);
        return false;
    }

    if (!incomingEventNotifier) {
        incomingEventNotifier = new IncomingEventNotifier(this, q);
        incomingEventNotifier->setHandle(incomingEventHandle);
        incomingEventNotifier->setEnabled(true);
    }

    return true;
}

void PeakCanBackendPrivate::canReadNotification()
{
    Q_Q(PeakCanBackend);

    forever {
        TPCANMsg message;
        ::memset(&message, 0, sizeof(message));
        TPCANTimestamp timestamp;
        ::memset(&timestamp, 0, sizeof(timestamp));

        if (TPCANStatus st = ::CAN_Read(channelIndex, &message, &timestamp) != PCAN_ERROR_OK) {
            if (st != PCAN_ERROR_XMTFULL)
                q->setError(systemErrorString(st), QCanBusDevice::ReadError);
            break;
        }

        QCanBusFrame frame(message.ID, QByteArray(reinterpret_cast<const char *>(message.DATA), int(message.LEN)));
        frame.setTimeStamp(QCanBusFrame::TimeStamp(timestamp.millis / 1000, timestamp.micros));

        q->enqueueReceivedFrame(frame);
    }

    // re-trigger the read event
    enableReadNotification();
}

bool PeakCanBackendPrivate::verifyBitRate(int bitrate)
{
    Q_Q(PeakCanBackend);

    if (isOpen) {
        q->setError(PeakCanBackend::tr("Impossible to reconfigure bitrate for the opened device"),
                    QCanBusDevice::ConfigurationError);
        return false;
    }

    if (bitrateCodeFromBitrate(bitrate) == -1) {
        q->setError(PeakCanBackend::tr("Unsupported bitrate value"),
                    QCanBusDevice::ConfigurationError);
        return false;
    }

    return true;
}

QT_END_NAMESPACE
