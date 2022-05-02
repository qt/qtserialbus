/****************************************************************************
**
** Copyright (C) 2017 Andre Hartmann <aha_1980@gmx.de>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef SYSTECCANBACKEND_P_H
#define SYSTECCANBACKEND_P_H

#include "systeccanbackend.h"
#include "systeccan_symbols_p.h"

#if defined(Q_OS_WIN32)
#  include <qt_windows.h>
#endif

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class QEvent;
class QSocketNotifier;
class QWinEventNotifier;
class QTimer;

class IncomingEventHandler : public QObject
{
    // no Q_OBJECT macro!
public:
    explicit IncomingEventHandler(SystecCanBackendPrivate *systecPrivate, QObject *parent) :
        QObject(parent),
        dptr(systecPrivate) { }

    void customEvent(QEvent *event) override;

private:
       SystecCanBackendPrivate * const dptr;
};

class SystecCanBackendPrivate
{
    Q_DECLARE_PUBLIC(SystecCanBackend)

public:
    SystecCanBackendPrivate(SystecCanBackend *q);

    bool open();
    void close();
    void eventHandler(QEvent *event);
    bool setConfigurationParameter(QCanBusDevice::ConfigurationKey key, const QVariant &value);
    bool setupChannel(const QString &interfaceName);
    void setupDefaultConfigurations();
    QString systemErrorString(int errorCode);
    void enableWriteNotification(bool enable);
    void startWrite();
    void readAllReceivedMessages();
    bool verifyBitRate(int bitrate);
    void resetController();
    QCanBusDevice::CanBusStatus busStatus();

    SystecCanBackend * const q_ptr;

    tUcanHandle handle = 0;
    quint8 device = 255;
    quint8 channel = 255;

    QTimer *outgoingEventNotifier = nullptr;
    IncomingEventHandler *incomingEventHandler = nullptr;
};

QT_END_NAMESPACE

#endif // SYSTECCANBACKEND_P_H
