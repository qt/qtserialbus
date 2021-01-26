/****************************************************************************
**
** Copyright (C) 2017 Ford Motor Company.
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
****************************************************************************/

#include "passthrucanbackend.h"

#include <QtSerialBus/qcanbus.h>
#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusfactory.h>

QT_BEGIN_NAMESPACE

class PassThruCanBusPlugin : public QObject, public QCanBusFactoryV2
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QCanBusFactory" FILE "plugin.json")
    Q_INTERFACES(QCanBusFactoryV2)

public:
    PassThruCanBusPlugin()
    {
        qRegisterMetaType<QCanBusDevice::CanBusError>();
        qRegisterMetaType<QVector<QCanBusFrame>>();
    }

    QList<QCanBusDeviceInfo> availableDevices(QString *) const override
    {
        return PassThruCanBackend::interfaces();
    }

    QCanBusDevice *createDevice(const QString &interfaceName, QString *) const override
    {
        return new PassThruCanBackend(interfaceName);
    }
};

QT_END_NAMESPACE

#include "main.moc"
