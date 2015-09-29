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
    \class QModbusDevice
    \inmodule QtSerialBus
    \since 5.6

    \brief The QModbusDevice class is the base class for Modbus classes, \l QModbusServer
    and \l QModbusClient.
 */

/*!
    Constructs a Modbus device with the specified \a parent.
 */
QModbusDevice::QModbusDevice(QObject *parent)
 : QObject(*new QModbusDevicePrivate, parent)
{
    qRegisterMetaType<QModbusRegister::RegisterType>();
}

/*!
    \internal
*/
QModbusDevice::QModbusDevice(QModbusDevicePrivate &dd, QObject *parent)
 : QObject(dd, parent)
{
    qRegisterMetaType<QModbusRegister::RegisterType>();
}

/*!
    Destroys the QModbusDevice instance
*/
QModbusDevice::~QModbusDevice()
{
}

/*!
    \internal
*/
void QModbusDevice::setPortName(const QString &name)
{
    Q_D(QModbusDevice);
    d->portName = name;
}

/*!
    \internal
*/
QString QModbusDevice::portName() const
{
    Q_D(const QModbusDevice);
    return d->portName;
}

/*!
    \enum QModbusDevice::ModBusError
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
    \enum QModbusDevice::ModBusDeviceState
    This enum describes all possible device states.

    \value UnconnectedState The device is disconnected.
    \value ConnectingState  The device is being connected.
    \value ConnectedState   The device is connected to the Modbus network.
    \value ClosingState     The device is being closed.
 */

/*!
    \fn QModbusDevice::errorOccurred(QModbusDevice::ModBusError error)

    This signal is emitted when an error of the type, \a error, occurs.
 */

/*!
    \fn void QModbusDevice::stateChanged(QModbusDevice::ModBusDeviceState state)

    This signal is emitted every time the state of the device changes.
    The new state is represented by \a state.

    \sa setState(), state()
 */

/*!
    Connects the device to the Modbus network. Returns \c true on success;
    otherwise \c false.

    This function calls \l open() as part of its implementation.
 */
bool QModbusDevice::connectDevice()
{
    Q_D(QModbusDevice);

    if (d->state != QModbusDevice::UnconnectedState)
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
void QModbusDevice::disconnectDevice()
{
    setState(QModbusDevice::ClosingState);

    //Unconnected is set by backend -> might be delayed by event loop
    close();
}

/*!
    Sets the state of the device to \a newState. Modbus device implementations
    must use this function to update the device state.
 */
void QModbusDevice::setState(QModbusDevice::ModBusDeviceState newState)
{
    Q_D(QModbusDevice);

    if (newState == d->state)
        return;

    d->state = newState;
    emit stateChanged(newState);
}

/*!
    Returns the current state of the device.

    \sa setState(), stateChanged()
 */
QModbusDevice::ModBusDeviceState QModbusDevice::state() const
{
    return d_func()->state;
}

/*!
    Sets the error state of the device. ModBus device implementations
    must use this function in case of an error to set the \a error type and
    a descriptive \a errorText.

    \sa QModbusDevice::ModBusError
 */
void QModbusDevice::setError(const QString &errorText, QModbusDevice::ModBusError error)
{
    Q_D(QModbusDevice);

    d->error = error;
    d->errorString = errorText;
    emit errorOccurred(error);
}

/*!
    Returns the error state of the device.

    \sa QModbusDevice::ModBusError
 */
QModbusDevice::ModBusError QModbusDevice::error() const
{
    return d_func()->error;
}

/*!
    Returns descriptive error text for the device error.

    \sa QModbusDevice::ModBusError
 */
QString QModbusDevice::errorString() const
{
    return d_func()->errorString;
}

/*!
    \fn bool QModbusDevice::open()

    This function is called by connectDevice(). Subclasses must provide
    an implementation that returns \c true on successful Modbus connection
    or \c false otherwise.

    The implementation must ensure that the instance's \l state()
    is set to \l QModbusDevice::ConnectedState upon success; otherwise
    \l QModbusDevice::UnconnectedState.

    \sa connectDevice()
 */

/*!
    \fn void QModbusDevice::close()

    This function is responsible for closing the Modbus connection.
    The implementation must ensure that the instance's
    \l state() is set to \l QModbusDevice::UnconnectedState.

    \sa disconnectDevice()
 */

/*!
    \internal
    \fn quint8 QModbusDevicePrivate::calculateLRC(const char *data, qint32 len) const

    Returns the LRC checksum of the first \a len bytes of \a data. The checksum is independent of
    the byte order (endianness).
*/

/*!
    \internal
    bool QModbusDevicePrivate::checkLRC(const char *data, qint32 len, quint8 lrc) const

    Returns true if the LRC checksum of the first \a len bytes of \a data match the given \a lrc;
    otherwise returns false.
*/

/*!
    \internal
    \fn quint8 QModbusDevicePrivate::calculateCRC(const char *data, qint32 len) const

    Returns the CRC checksum of the first \a len bytes of \a data.

    \note The code used by the function was generated with pycrc. There is no copyright assigned
    to the generated code, however, the author of the script requests to show the line stating
    that the code was generated by pycrc (see implementation).
*/

/*!
    \internal
    bool QModbusDevicePrivate::checkCRC(const char *data, qint32 len, quint8 crc) const

    Returns true if the CRC checksum of the first \a len bytes of \a data match the given \a crc;
    otherwise returns false.
*/

QT_END_NAMESPACE
