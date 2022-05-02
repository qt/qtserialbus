/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

#ifndef QCANBUSDEVICE_P_H
#define QCANBUSDEVICE_P_H

#include <QtCore/qmutex.h>
#include <QtSerialBus/qcanbusdevice.h>

#include <private/qobject_p.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

typedef QPair<QCanBusDevice::ConfigurationKey, QVariant > ConfigEntry;

class QCanBusDevicePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QCanBusDevice)
public:
    QCanBusDevicePrivate() {}

    QCanBusDevice::CanBusError lastError = QCanBusDevice::CanBusError::NoError;
    QCanBusDevice::CanBusDeviceState state = QCanBusDevice::UnconnectedState;
    QString errorText;

    QList<QCanBusFrame> incomingFrames;
    QMutex incomingFramesGuard;
    QList<QCanBusFrame> outgoingFrames;
    QList<ConfigEntry> configOptions;

    bool waitForReceivedEntered = false;
    bool waitForWrittenEntered = false;

    std::function<void()> m_resetControllerFunction;
    std::function<QCanBusDevice::CanBusStatus()> m_busStatusGetter;
};

QT_END_NAMESPACE

#endif // QCANBUSDEVICE_P_H
