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

#include "qcanbusdevice.h"
#include "qcanbusdevice_p.h"
#include <QtSerialBus/qserialbusbackend.h>

#include "qcanframe.h"

#include <QtCore/qdebug.h>
#include <QtCore/qdatastream.h>

#define SOCKET_CAN_MTU 72

QT_BEGIN_NAMESPACE

/*!
    \class QCanBusDevice
    \inmodule QtSerialBus
    \since 5.6

    \brief The QCanBusDevice class is the interface class for CAN bus.

    QCanBusDevice communicates with a CAN backend providing users with a convenient API.
    The CAN backend must be specified during the object creation.
 */

/*!
    \enum QCanBusDevice::CanBusError
    This enum describes all the possible error conditions.

    \value NoError              No errors have occurred.
    \value ReadError            An error occurred during a read operation.
    \value WriteError           An error occurred during a write operation.
    \value ConnectionError      An error occurred when attempting to open the backend.
    \value ConfigurationError   An error occurred when attempting to set a configuration
                                parameter.
    \value UnknownError         An unknown error occurred.
 */

/*!
    \enum QCanBusDevice::CanBusDeviceState
    This enum describes all possible device states.

    \value UnconnectedState The device is disconnected.
    \value ConnectingState  The device is being connected.
    \value ConnectedState   The device is connected to the CAN bus.
    \value ClosingState     The device is being closed.
 */

/*!
    \fn QCanBusDevice::errorOccurred(CanBusError error)

    This signal is emitted when an error of the type \a error occurs.
 */

/*!
    Constructs a serial bus device initialized with the \a backend to use and with the specified
    \a parent.

    QCanBusDevice takes ownership of \a backend.
 */
QCanBusDevice::QCanBusDevice(QSerialBusBackend *backend, QObject *parent) :
    QObject(*new QCanBusDevicePrivate, parent)
{
    d_func()->pluginBackend = backend;
    connect(backend, &QSerialBusBackend::error, this, &QCanBusDevice::setError);
    connect(backend, &QSerialBusBackend::frameReceived,
            this, &QCanBusDevice::frameReceived);
    connect(backend, &QSerialBusBackend::stateChanged,
            this, &QCanBusDevice::updateState);
}

void QCanBusDevice::setError(QString errorString, int errorId)
{
    Q_D(QCanBusDevice);
    d->setError(errorString, errorId);
}

//TODO Remove once QSerialBusBackend is gone
void QCanBusDevice::updateState(int newState)
{
    setState(static_cast<QCanBusDevice::CanBusDeviceState>(newState));
}

/*!
    Sets the configuration parameters for the bus backend. The key-value pair is backend-specific.

    The following table lists the supported \a key and \a value pairs for the SocketCAN backend:

    \table
        \header
            \li Key
            \li Value
        \row
            \li Loopback
            \li bool
        \row
            \li ReceiveOwnMessages
            \li bool
        \row
            \li ErrorMask
            \li int
        \row
            \li CanFilter
            \li QList<QVariant filter>
    \endtable

    \c CanFilter contains a list of filters. Each filter consists of QHash<QString, QVariant>.
    The following table lists the key-value pairs for the filters:

    \table
        \header
            \li Key
            \li Value
        \row
            \li FilterId
            \li int
        \row
            \li CanMask
            \li int
    \endtable

    \sa configurationParameter()
 */
void QCanBusDevice::setConfigurationParameter(const QString &key, const QVariant &value)
{
    d_func()->pluginBackend->setConfigurationParameter(key, value);
}

/*!
    Returns the current value assigned to the configuration parameter \a key.

    \sa setConfigurationParameter()
 */
QVariant QCanBusDevice::configurationParameter(const QString &key) const
{
    return d_func()->pluginBackend->configurationParameter(key);
}

/*!
    Returns the list of keys used by the backend.
 */
QVector<QString> QCanBusDevice::configurationKeys() const
{
    return d_func()->pluginBackend->configurationKeys();
}

/*!
    Writes \a frame to the CAN bus.

    \sa readFrame()
 */
void QCanBusDevice::writeFrame(const QCanFrame &frame)
{
    Q_D(QCanBusDevice);

    if (d->state != ConnectedState)
        return;

    d_func()->pluginBackend->writeFrame(frame);
}

/*!
   Reads the information contained in one CAN frame from the backend.

   \sa writeFrame()
 */
QCanFrame QCanBusDevice::readFrame()
{
    Q_D(QCanBusDevice);
    if (d->state != ConnectedState)
        return QCanFrame();

    return d_func()->pluginBackend->nextFrame();
}

/*!
    Returns the last error that has occurred. The error value is always set to last error that
    occurred and it is never reset.

    \sa errorString()
 */
QCanBusDevice::CanBusError QCanBusDevice::error() const
{
    return d_func()->lastError;
}

/*!
    Returns a human-readable description of the last device error that occurred.

    \sa error()
 */
QString QCanBusDevice::errorString() const
{
    Q_D(const QCanBusDevice);

    if (d->lastError == QCanBusDevice::NoError)
        return QString();

    return d->errorText;
}

/*!
    Returns the number of available frames.
 */
qint64 QCanBusDevice::availableFrames() const
{
    return d_func()->pluginBackend->availableFrames();
}

bool QCanBusDevice::connectDevice()
{
    Q_D(QCanBusDevice);

    if (d->state != QCanBusDevice::UnconnectedState)
        return false;

    setState(ConnectingState);

    if (!d->pluginBackend->open()) {
        setState(UnconnectedState);
        return false;
    }

    //Connected is set by backend -> might be delayed by event loop
    return true;
}

void QCanBusDevice::disconnectDevice()
{
    setState(QCanBusDevice::ClosingState);

    //Unconnected is set by backend -> might be delayed by event loop
    d_func()->pluginBackend->close();
}

/*!
    Returns the current state of the device.

    \sa setState()
 */
QCanBusDevice::CanBusDeviceState QCanBusDevice::state() const
{
    return d_func()->state;
}

/*!
    Sets the state of the device. CAN bus implementations
    must use this function to update the device state.
 */
void QCanBusDevice::setState(QCanBusDevice::CanBusDeviceState newState)
{
    Q_D(QCanBusDevice);

    if (newState == d->state)
        return;

    d->state = newState;
    emit stateChanged(newState);
}

void QCanBusDevicePrivate::setError(const QString &errorString, int errorId)
{
    Q_Q(QCanBusDevice);

    errorText = errorString;

    switch (errorId) {
    case QCanBusDevice::CanBusError::ReadError:
        lastError = QCanBusDevice::CanBusError::ReadError;
        break;
    case QCanBusDevice::CanBusError::WriteError:
        lastError = QCanBusDevice::CanBusError::WriteError;
        break;
    case QCanBusDevice::CanBusError::ConnectionError:
        lastError = QCanBusDevice::CanBusError::ConnectionError;
        break;
    case QCanBusDevice::CanBusError::ConfigurationError:
        lastError = QCanBusDevice::CanBusError::ConfigurationError;
        break;
    case QCanBusDevice::CanBusError::UnknownError:
    default:
        lastError = QCanBusDevice::CanBusError::UnknownError;
        break;
    }
    emit q->errorOccurred(lastError);
}

QT_END_NAMESPACE
