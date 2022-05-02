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

#ifndef QCANBUS_H
#define QCANBUS_H

#include <QtCore/qobject.h>
#include <QtSerialBus/qtserialbusglobal.h>
#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusdeviceinfo.h>

QT_BEGIN_NAMESPACE

class Q_SERIALBUS_EXPORT QCanBus : public QObject
{
    Q_OBJECT

public:
    static QCanBus *instance();
    QStringList plugins() const;

    QList<QCanBusDeviceInfo> availableDevices(const QString &plugin, QString *errorMessage = nullptr) const;

    QCanBusDevice *createDevice(const QString &plugin,
                                const QString &interfaceName,
                                QString *errorMessage = nullptr) const;

private:
    QCanBus(QObject *parent = nullptr);

    Q_DISABLE_COPY(QCanBus)
};

QT_END_NAMESPACE

#endif // QSERIALBUS_H
