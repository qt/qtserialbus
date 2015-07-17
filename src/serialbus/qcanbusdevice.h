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

#include <QtSerialBus/qcanframe.h>
#include <QtSerialBus/qserialbusbackend.h>

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QCanBusDevicePrivate;

//TODO for this class: review const member functions for this class
class Q_SERIALBUS_EXPORT QCanBusDevice : public QObject
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

    explicit QCanBusDevice(QObject *parent = 0);

    virtual void setConfigurationParameter(const QString &key, const QVariant &value) = 0;
    virtual QVariant configurationParameter(const QString &key) const = 0;
    virtual QVector<QString> configurationKeys() const = 0;

    virtual bool writeFrame(const QCanFrame &frame) = 0;
    virtual QCanFrame readFrame() = 0;
    virtual qint64 availableFrames() const = 0;

    //TODO currently assumes unbuffered write. Add support for buffered writes
    // qint64 framesToWrite() const
    // signal: void framesWritten(qint64 framesCount)

    // TODO rename these once QIODevice dependency has been removed
    bool connectDevice();
    void disconnectDevice();

    CanBusDeviceState state() const;

    CanBusError error() const;
    QString errorString() const;

Q_SIGNALS:
    void errorOccurred(QCanBusDevice::CanBusError);
    void frameReceived();
    void stateChanged(QCanBusDevice::CanBusDeviceState state);

protected:
    void setState(QCanBusDevice::CanBusDeviceState newState);
    void setError(const QString &errorText, QCanBusDevice::CanBusError);

    virtual bool open() = 0;
    virtual void close() = 0;
};

Q_DECLARE_TYPEINFO(QCanBusDevice::CanBusError, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QCanBusDevice::CanBusDeviceState, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

#endif // QCANBUSDEVICE_H
