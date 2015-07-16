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

#ifndef QCANBUSDEVICE_H
#define QCANBUSDEVICE_H

#include <QtSerialBus/qserialbusdevice.h>
#include <QtSerialBus/qcanframe.h>

QT_BEGIN_NAMESPACE

class QCanBusDevicePrivate;

//TODO for this class: review const member functions for this class
class Q_SERIALBUS_EXPORT QCanBusDevice : public QSerialBusDevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QCanBusDevice)
    Q_DISABLE_COPY(QCanBusDevice)

public:
    enum CanBusError {
        NoError,
        ReadError,
        WriteError,
        ConnectionError,
        ConfigurationError,
        UnknownError
    };
    Q_ENUM(CanBusError)

    enum CanBusDeviceState {
        UnconnectedState,
        ConnectingState,
        ConnectedState,
        ClosingState
    };
    Q_ENUM(CanBusDeviceState)

    explicit QCanBusDevice(QSerialBusBackend *backend, QObject *parent = 0);
    void setConfigurationParameter(const QString &key, const QVariant &value);
    QVariant configurationParameter(const QString &key) const;
    QVector<QString> configurationKeys() const;

    void writeFrame(const QCanFrame &frame);
    QCanFrame readFrame();
    CanBusError error() const;
    qint64 availableFrames() const;
    //TODO currently assumes unbuffered write. Add support for buffered writes
    // qint64 framesToWrite() const
    // signal: void framesWritten(qint64 framesCount)

    // TODO rename these once QIODevice dependency has been removed
    bool connectDevice();
    void disconnectDevice();

    CanBusDeviceState state() const;

Q_SIGNALS:
    void errorOccurred(QCanBusDevice::CanBusError);
    void frameReceived();
    void stateChanged(QCanBusDevice::CanBusDeviceState state);

protected:
    void setState(QCanBusDevice::CanBusDeviceState newState);

private Q_SLOTS:
    void setError(QString, int);
    void updateState(QCanBusDevice::CanBusDeviceState newState);
};

Q_DECLARE_TYPEINFO(QCanBusDevice::CanBusError, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QCanBusDevice::CanBusDeviceState, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

#endif // QCANBUSDEVICE_H
