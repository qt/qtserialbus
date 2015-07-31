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

bool PeakCanBackendPrivate::open()
{
    Q_Q(PeakCanBackend);

    // TODO: Maybe need to move the PCAN_BAUD_500K to the configuration parameters?
    if (TPCANStatus st = ::CAN_Initialize(channelIndex, PCAN_BAUD_500K, 0, 0, 0) != PCAN_ERROR_OK) {
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

// TODO: Implement me
bool PeakCanBackendPrivate::setConfigurationParameter(int key, const QVariant &value)
{
    Q_UNUSED(key);
    Q_UNUSED(value);

    return false;
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

QString PeakCanBackendPrivate::systemErrorString(int errorCode)
{
    QByteArray buffer(2048, 0);
    ::CAN_GetErrorText(errorCode, 0, reinterpret_cast<LPSTR>(buffer.data()));
    return QString::fromWCharArray(reinterpret_cast<const wchar_t *>(buffer.constData()));
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

    if (payload.size() > int(sizeof(message.DATA))) {
        qWarning("Impossible to write the message with unacceptable payload size %d, ignored",
                 payload.size());
    } else {
        message.ID = frame.frameId();
        message.LEN = payload.size();
        message.MSGTYPE = frame.hasExtendedFrameFormat() ? PCAN_MESSAGE_EXTENDED : PCAN_MESSAGE_STANDARD;

        ::memcpy(message.DATA, payload.constData(), sizeof(message.DATA));
        if (TPCANStatus st = ::CAN_Write(channelIndex, &message) != PCAN_ERROR_OK) {
            q->setError(systemErrorString(st), QCanBusDevice::WriteError);
        } else {
            // TODO: Emit the future signal that the frame has been written
        }
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

QT_END_NAMESPACE
