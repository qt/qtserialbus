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

#include "qmodbusdevice.h"
#include "qmodbusdevice_p.h"

#include <QtSerialBus/qmodbusregister.h>

QT_BEGIN_NAMESPACE

/*!
    \class QModBusDevice
    \inmodule QtSerialBus
    \since 5.6

    \brief The QModBusDevice class is the base class for Modbus classes, \l QModBusSlave
    and \l QModBusMaster.
 */

/*!
    Constructs a Modbus device with the specified \a parent.
 */
QModBusDevice::QModBusDevice(QObject *parent)
 : QObject(*new QModBusDevicePrivate, parent)
{
    qRegisterMetaType<QModBusRegister::RegisterType>();
}

/*!
    \internal
*/
QModBusDevice::QModBusDevice(QModBusDevicePrivate &dd, QObject *parent)
 : QObject(dd, parent)
{
    qRegisterMetaType<QModBusRegister::RegisterType>();
}

/*!
    Destroys the QModBusDevice instance
*/
QModBusDevice::~QModBusDevice()
{
}

void QModBusDevice::setPortName(const QString &name)
{
    Q_D(QModBusDevice);
    d->portName = name;
}

QString QModBusDevice::portName() const
{
    Q_D(const QModBusDevice);
    return d->portName;
}

/*!
    \enum QModBusDevice::ModBusError
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
    \enum QModBusDevice::ModBusDeviceState
    This enum describes all possible device states.

    \value UnconnectedState The device is disconnected.
    \value ConnectingState  The device is being connected.
    \value ConnectedState   The device is connected to the Modbus network.
    \value ClosingState     The device is being closed.
 */

/*!
    \fn QModBusDevice::errorOccurred(QModBusDevice::ModBusError error)

    This signal is emitted when an error of the type, \a error, occurs.
 */

/*!
    \fn void QModBusDevice::stateChanged(QModBusDevice::ModBusDeviceState state)

    This signal is emitted every time the state of the device changes.
    The new state is represented by \a state.

    \sa setState(), state()
 */

/*!
    Connects the device to the Modbus network. Returns \c true on success;
    otherwise \c false.

    This function calls \l open() as part of its implementation.
 */
bool QModBusDevice::connectDevice()
{
    Q_D(QModBusDevice);

    if (d->state != QModBusDevice::UnconnectedState)
        return false;

    setState(ConnectingState);

    if (!open()) {
        setState(UnconnectedState);
        return false;
    }

    //Connected is set by backend -> might be delayed by event loop
    return true;
}

/*!
    Disconnects the device.

    This function calls \l close() as part of its implementation.
 */
void QModBusDevice::disconnectDevice()
{
    setState(QModBusDevice::ClosingState);

    //Unconnected is set by backend -> might be delayed by event loop
    close();
}

/*!
    Sets the state of the device to \a newState. Modbus device implementations
    must use this function to update the device state.
 */
void QModBusDevice::setState(QModBusDevice::ModBusDeviceState newState)
{
    Q_D(QModBusDevice);

    if (newState == d->state)
        return;

    d->state = newState;
    emit stateChanged(newState);
}

/*!
    Returns the current state of the device.

    \sa setState(), stateChanged()
 */
QModBusDevice::ModBusDeviceState QModBusDevice::state() const
{
    return d_func()->state;
}

/*!
    Sets the error state of the device. ModBus device implementations
    must use this function in case of an error to set the \a error type and
    a descriptive \a errorText.

    \sa QModBusDevice::ModBusError
 */
void QModBusDevice::setError(const QString &errorText, QModBusDevice::ModBusError error)
{
    Q_D(QModBusDevice);

    d->error = error;
    d->errorString = errorText;
    emit errorOccurred(error);
}

/*!
    Returns the error state of the device.

    \sa QModBusDevice::ModBusError
 */
QModBusDevice::ModBusError QModBusDevice::error() const
{
    return d_func()->error;
}

/*!
    Returns descriptive error text for the device error.

    \sa QModBusDevice::ModBusError
 */
QString QModBusDevice::errorString() const
{
    return d_func()->errorString;
}

/*!
    \fn bool QModBusDevice::open()

    This function is called by connectDevice(). Subclasses must provide
    an implementation that returns \c true on successful Modbus connection
    or \c false otherwise.

    The implementation must ensure that the instance's \l state()
    is set to \l QModBusDevice::ConnectedState upon success; otherwise
    \l QModBusDevice::UnconnectedState.

    \sa connectDevice()
 */

/*!
    \fn void QModBusDevice::close()

    This function is responsible for closing the Modbus connection.
    The implementation must ensure that the instance's
    \l state() is set to \l QModBusDevice::UnconnectedState.

    \sa disconnectDevice()
 */

QT_END_NAMESPACE
