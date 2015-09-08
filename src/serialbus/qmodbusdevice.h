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

class QModBusDevicePrivate;


class Q_SERIALBUS_EXPORT QModBusDevice : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModBusDevice)

public:

    enum ApplicationDataUnit {
        NotSpecified,
        RemoteTerminalUnit,
        TCP
    };
    Q_ENUM(ApplicationDataUnit)

    enum ModBusTable {
        DiscreteInputs,
        Coils,
        InputRegisters,
        HoldingRegisters
    };
    Q_ENUM(ModBusTable)

    enum ModBusError {
        NoError,
        ReadError,
        WriteError,
        ConnectionError,
        ConfigurationError,
        UnknownError
    };
    Q_ENUM(ModBusError)

    enum ModBusDeviceState {
        UnconnectedState,
        ConnectingState,
        ConnectedState,
        ClosingState
    };
    Q_ENUM(ModBusDeviceState)

    explicit QModBusDevice(QObject *parent = 0);
    virtual ~QModBusDevice();

    bool connectDevice();
    void disconnectDevice();

    virtual bool setDevice(QIODevice *transport, ApplicationDataUnit ADU = NotSpecified) = 0;

    ModBusDeviceState state() const;

    ModBusError error() const;
    QString errorString() const;

Q_SIGNALS:
    void errorOccurred(QModBusDevice::ModBusError error);
    void stateChanged(QModBusDevice::ModBusDeviceState state);

protected:
    void setState(QModBusDevice::ModBusDeviceState newState);
    void setError(const QString &errorText, QModBusDevice::ModBusError error);
    virtual bool open() = 0;
    virtual void close() = 0;
};

Q_DECLARE_TYPEINFO(QModBusDevice::ApplicationDataUnit, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QModBusDevice::ModBusTable, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QModBusDevice::ModBusError, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QModBusDevice::ModBusDeviceState, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE
#endif // QMODBUSDEVICE_H
