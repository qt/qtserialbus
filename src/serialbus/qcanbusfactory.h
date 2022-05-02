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

#ifndef QCANBUSFACTORY_H
#define QCANBUSFACTORY_H

#include <QtCore/qstringlist.h>
#include <QtSerialBus/qtserialbusglobal.h>
#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusdeviceinfo.h>

QT_BEGIN_NAMESPACE

class Q_SERIALBUS_EXPORT QCanBusFactory
{
public:
    virtual QCanBusDevice *createDevice(const QString &interfaceName,
                                QString *errorMessage) const = 0;
    virtual QList<QCanBusDeviceInfo> availableDevices(QString *errorMessage) const = 0;

protected:
    virtual ~QCanBusFactory();
};

Q_DECLARE_INTERFACE(QCanBusFactory, "org.qt-project.Qt.QCanBusFactory")

QT_END_NAMESPACE

#endif // QCANBUSFACTORY_H

