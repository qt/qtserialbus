/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QMODBUSDEVICE_H
#define QMODBUSDEVICE_H

#include <QtCore/qobject.h>
#include <QtCore/qiodevice.h>
#include <QtSerialBus/qtserialbusglobal.h>

QT_BEGIN_NAMESPACE

class QModbusDevicePrivate;

class Q_SERIALBUS_EXPORT QModbusDevice : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModbusDevice)

public:
    enum Error {
        NoError,
        ReadError,
        WriteError,
        ConnectionError,
        ConfigurationError,
        TimeoutError,
        ProtocolError,
        ReplyAbortedError,
        UnknownError
    };
    Q_ENUM(Error)

    enum State {
        UnconnectedState,
        ConnectingState,
        ConnectedState,
        ClosingState
    };
    Q_ENUM(State)

    enum ConnectionParameter {
        SerialPortNameParameter,
        SerialParityParameter,
        SerialBaudRateParameter,
        SerialDataBitsParameter,
        SerialStopBitsParameter,

        NetworkPortParameter,
        NetworkAddressParameter
    };
    Q_ENUM(ConnectionParameter)

    enum IntermediateError
    {
        ResponseCrcError,
        ResponseRequestMismatch
    };
    Q_ENUM(IntermediateError)

    explicit QModbusDevice(QObject *parent = nullptr);
    ~QModbusDevice();

    QVariant connectionParameter(ConnectionParameter parameter) const;
    void setConnectionParameter(ConnectionParameter parameter, const QVariant &value);

    bool connectDevice();
    void disconnectDevice();

    State state() const;

    Error error() const;
    QString errorString() const;

    QIODevice *device() const;

Q_SIGNALS:
    void errorOccurred(QModbusDevice::Error error);
    void stateChanged(QModbusDevice::State state);

protected:
    QModbusDevice(QModbusDevicePrivate &dd, QObject *parent = nullptr);

    void setState(QModbusDevice::State newState);
    void setError(const QString &errorText, QModbusDevice::Error error);
    virtual bool open() = 0;
    virtual void close() = 0;
};

Q_DECLARE_TYPEINFO(QModbusDevice::Error, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QModbusDevice::State, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QModbusDevice::ConnectionParameter, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QModbusDevice::IntermediateError, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

#endif // QMODBUSDEVICE_H
