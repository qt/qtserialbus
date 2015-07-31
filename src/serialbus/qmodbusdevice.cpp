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

QT_BEGIN_NAMESPACE

/*!
    \class QModBusDevice
    \inmodule QtSerialBus
    \since 5.6

    \brief The QModBusDevice class is the base class for Modbus classes \l QModBusSlave
    and \l QModBusMaster.
 */
QModBusDevice::QModBusDevice(QObject *parent)
 : QObject(*new QModBusDevicePrivate, parent)
{

}

/*!
    \enum QModBusDevice::ApplicationDataUnit
    This enum describes different Modbus ADU types.

    \value NotSpecified         ADU not specified
    \value RemoteTerminalUnit   Usually used with serial port connection
    \value TCP                  Usually used with TCP connection
 */

/*!
    \enum QModBusDevice::ModBusTable
    This enum describes different primary tables used in Modbus.

    \value DiscreteInputs       Table of read only bits
    \value Coils                Table of read/write bits
    \value InputRegisters       Table of read only 16 bit words
    \value HoldingRegisters     Table of read/write 16 bit words
 */

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
    \fn QModBusDevice::errorOccurred(CanModError error)

    This signal is emitted when an error of the type \a error occurs.
 */

/*!
    \fn void QModBusDevice::stateChanged(QModBusDevice::CanModDeviceState state)

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
    Sets the state of the device to \a newState. ModBus device implementations
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
    \fn bool QModBusDevice::open()

    This function is called by connectDevice(). Subclasses must provide
    an implementation which returns \c true if the modbus connection
    could be established; otherwise \c false.

    The implementation must ensure that upon success the instance's \l state()
    is set to \l QModBusDevice::ConnectedState; otherwise
    \l QModBusDevice::UnconnectedState.

    \sa connectDevice()
 */

/*!
    \fn void QModBusDevice::close()

    This function is responsible for closing the modbus connection.
    The implementation must ensure that the instance's
    \l state() is set to \l QModBusDevice::UnconnectedState.

    \sa disconnectDevice()
 */

QT_END_NAMESPACE
