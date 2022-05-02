/****************************************************************************
**
** Copyright (C) 2017 Denis Shienkov <denis.shienkov@gmail.com>
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

#include "vectorcanbackend.h"

#include <QtSerialBus/qcanbus.h>
#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusfactory.h>

#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(QT_CANBUS_PLUGINS_VECTORCAN, "qt.canbus.plugins.vectorcan")

class VectorCanBusPlugin : public QObject, public QCanBusFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QCanBusFactory" FILE "plugin.json")
    Q_INTERFACES(QCanBusFactory)

public:
    QList<QCanBusDeviceInfo> availableDevices(QString *errorMessage) const override
    {
        if (Q_UNLIKELY(!VectorCanBackend::canCreate(errorMessage)))
            return QList<QCanBusDeviceInfo>();

        return VectorCanBackend::interfaces();
    }

    QCanBusDevice *createDevice(const QString &interfaceName, QString *errorMessage) const override
    {
        QString errorReason;
        if (Q_UNLIKELY(!VectorCanBackend::canCreate(&errorReason))) {
            qCWarning(QT_CANBUS_PLUGINS_VECTORCAN, "%ls", qUtf16Printable(errorReason));
            if (errorMessage)
                *errorMessage = errorReason;
            return nullptr;
        }

        auto device = new VectorCanBackend(interfaceName);
        return device;
    }
};

QT_END_NAMESPACE

#include "main.moc"
