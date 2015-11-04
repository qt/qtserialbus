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
#ifndef QMODBUSDEVICE_H
#define QMODBUSDEVICE_H

#include <QtSerialBus/qserialbusglobal.h>

#include <QtCore/qobject.h>
#include <QtCore/qiodevice.h>

QT_BEGIN_NAMESPACE

class QModbusDevicePrivate;


class Q_SERIALBUS_EXPORT QModbusDevice : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModbusDevice)

public:
    // TODO: Temporarily added until plugin system removed.
    enum ModbusConnection {
        Serial,
        Tcp
    };

    enum ModbusError { //TODO review -> some values are reported via QModbusReply (e.g. WriteError)
        NoError,
        ReadError,
        WriteError,
        ConnectionError,
        ConfigurationError,
        TimeoutError,
        UnknownError
    };
    Q_ENUM(ModbusError)

    enum ModbusDeviceState {
        UnconnectedState,
        ConnectingState,
        ConnectedState,
        ClosingState
    };
    Q_ENUM(ModbusDeviceState)

    explicit QModbusDevice(QObject *parent = 0);
    virtual ~QModbusDevice();

    //TODO should become part of connect call in RTS specific sub class
    void setPortName(const QString& name);
    QString portName() const;

    virtual bool connectDevice(); //TODO remove virtual, workaround
    void disconnectDevice();

    ModbusDeviceState state() const;

    ModbusError error() const;
    QString errorString() const;

Q_SIGNALS:
    void errorOccurred(QModbusDevice::ModbusError error);
    void stateChanged(QModbusDevice::ModbusDeviceState state);

protected:
    QModbusDevice(QModbusDevicePrivate &dd, QObject *parent = Q_NULLPTR);

    void setState(QModbusDevice::ModbusDeviceState newState);
    void setError(const QString &errorText, QModbusDevice::ModbusError error);
    virtual bool open() = 0;
    virtual void close() = 0;
};

Q_DECLARE_TYPEINFO(QModbusDevice::ModbusError, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QModbusDevice::ModbusDeviceState, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

#endif // QMODBUSDEVICE_H
