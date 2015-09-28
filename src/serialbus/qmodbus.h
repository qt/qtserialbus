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

#ifndef QMODBUS_H
#define QMODBUS_H

#include <QtSerialBus/qserialbusglobal.h>
#include <QtSerialBus/qmodbusclient.h>
#include <QtSerialBus/qmodbusserver.h>

#include <QtCore/qobject.h>
#include <QtCore/qiodevice.h>

QT_BEGIN_NAMESPACE

class Q_SERIALBUS_EXPORT QModbus : public QObject
{
    Q_OBJECT

public:
    static QModbus *instance();
    QList<QByteArray> plugins() const;

    QModbusServer *createSlave(const QByteArray &plugin,
        QModbusDevice::ModBusConnection type = QModbusDevice::Serial) const;
    QModbusClient *createMaster(const QByteArray &plugin,
        QModbusDevice::ModBusConnection type = QModbusDevice::Serial) const;

private:
    QModbus(QObject *parent = 0);

    Q_DISABLE_COPY(QModbus)
};

QT_END_NAMESPACE

#endif // QMODBUS_H
