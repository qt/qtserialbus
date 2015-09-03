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

#include "qcanbusframe.h"

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

//TODO unwrap the RawFilterKey value. List inside lists are too complex & unintuitive

/*!
    \enum QCanBusDevice::ConfigurationKey
    This enum describes the possible configuration options for
    the CAN bus connection.

    \value RawFilterKey     This configuration determines the type of CAN bus frames
                            which the current device accepts. The expected value
                            is \c QList<QVariant>.
    \value ErrorFilterKey   This key defines the type of error which should be
                            forwarded via the current connection. The associated
                            value should be of type \l QCanBusFrame::FrameErrors.
    \value LoopbackKey      This key defines whether the can bus device should
                            operate in loopback mode. The expected value for this
                            key is \c bool.
    \value ReceiveOwnKey    This key defines whether this CAN device can send messages.
                            The expected value for this key is \c bool.
    \value BitRateKey       This key defines the bitrate in bits per second.
    \value UserKey          This key defines the range where custom keys start. It's most
                            common purpose is to permit platform specific configuration
                            options.
 */

/*!
    \fn QCanBusDevice::errorOccurred(CanBusError error)

    This signal is emitted when an error of the type \a error occurs.
 */

/*!
    Constructs a serial bus device with the specified \a parent.
 */
QCanBusDevice::QCanBusDevice(QObject *parent) :
    QObject(*new QCanBusDevicePrivate, parent)
{
}


/*!
    Sets the human readable description of the last device error to
    \a errorText. \a errorId categorizes the type of error.

    CAN bus implementations must use this function to update the device's
    error state.

    \sa error(), errorOccurred()
 */
void QCanBusDevice::setError(const QString &errorText, CanBusError errorId)
{
    Q_D(QCanBusDevice);

    d->errorText = errorText;
    d->lastError = errorId;

    emit errorOccurred(errorId);
}

/*!
    Appends \a newFrames to the internal list of frames which can be
    accessed using \l readFrame() and emits the \l framesReceived()
    signal.

    Subclasses must call this function when they receive frames.

 */
void QCanBusDevice::enqueueReceivedFrames(const QVector<QCanBusFrame> &newFrames)
{
    Q_D(QCanBusDevice);

    if (newFrames.isEmpty())
        return;

    d->incomingFramesGuard.lock();
    d->incomingFrames.append(newFrames);
    d->incomingFramesGuard.unlock();
    emit framesReceived();
}

/*!
    Sets the configuration parameter \a key for the CAN bus connection
    to \a value. The potential keys are represented by \l ConfigurationKey.

    A paramenter can be unset by setting an invalid \l QVariant.

    \note In most cases, configuration changes only take effect
    after a reconnect.

    \sa configurationParameter()
 */
void QCanBusDevice::setConfigurationParameter(int key,
                                              const QVariant &value)
{
    Q_D(QCanBusDevice);

    for (int i = 0; i < d->configOptions.size(); i++) {
        if (d->configOptions.at(i).first == key) {
            if (value.isValid()) {
                ConfigEntry entry = d->configOptions.at(i);
                entry.second = value;
                d->configOptions.replace(i, entry);
            } else {
                d->configOptions.remove(i);
            }
            return;
        }
    }

    if (!value.isValid())
        return;

    ConfigEntry newEntry(key, value);
    d->configOptions.append(newEntry);
}

/*!
    Returns the current value assigned to the \l ConfigurationKey \a key; otherwise
    an invalid \l QVariant.

    \sa setConfigurationParameter(), configurationKeys()
 */
QVariant QCanBusDevice::configurationParameter(int key) const
{
    Q_D(const QCanBusDevice);

    foreach (const ConfigEntry &e, d->configOptions) {
        if (e.first == key)
            return e.second;
    }

    return QVariant();
}

/*!
    Returns the list of keys used by the CAN bus connection.

    The the meaning of the keys is equivalent to \l ConfigurationKey.
    If a key is not explicitly mentioned the platform's
    default setting for the relevant key is used.
 */
QVector<int> QCanBusDevice::configurationKeys() const
{
    Q_D(const QCanBusDevice);

    QVector<int> result;
    foreach (const ConfigEntry &e, d->configOptions)
        result.append(e.first);

    return result;
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
    \fn qint64 QCanBusDevice::framesAvailable() const

    Returns the number of available frames. If no frames are available,
    this function returns 0.

    \sa readFrame()
*/
qint64 QCanBusDevice::framesAvailable() const
{
    return d_func()->incomingFrames.size();
}

/*!
    \fn bool QCanBusDevice::open()

    This function is called by connectDevice(). Subclasses must provide
    an implementation which returns \c true if the CAN bus connection
    could be established; otherwise \c false.

    The implementation must ensure that upon success the instance's \l state()
    is set to \l QCanBusDevice::ConnectedState; otherwise
    \l QCanBusDevice::UnconnectedState.

    \sa connectDevice()
 */

/*!
    \fn void QCanBusDevice::close()

    This function is responsible for closing the CAN bus connection.
    The implementation must ensure that the instance's
    \l state() is set to \l QCanBusDevice::UnconnectedState.

    \sa disconnectDevice()
*/

/*!
    \fn void QCanBusDevice::framesReceived()

    This signal is emitted when one or more frames have been received.
    The frames should be read using \l readFrame() and \l framesAvailable().
 */

/*!
    Returns the next \l QCanBusFrame from the queue; otherwise returns
    an empty QCanBusFrame. The returned frame is removed from the queue.

    The queue operates according to the FIFO principle.

    \sa framesAvailable()
 */
QCanBusFrame QCanBusDevice::readFrame()
{
    Q_D(QCanBusDevice);

    if (d->state != ConnectedState)
        return QCanBusFrame(QCanBusFrame::InvalidFrame);

    QMutexLocker locker(&d->incomingFramesGuard);

    if (d->incomingFrames.isEmpty())
        return QCanBusFrame(QCanBusFrame::InvalidFrame);

    return d->incomingFrames.takeFirst();
}

/*!
    \fn bool QCanBusDevice::writeFrame(const QCanBusFrame &frame)

    Writes \a frame to the CAN bus and returns \c true on success;
    otherwise \c false.
 */

/*!
    \fn QString interpretErrorFrame(const QCanBusFrame &frame)

    Interprets \a frame as error frame and returns a human readable
    description of the error.

    If \a frame is not an error frame, the return string is empty.
 */

/*!
    Connects the device to the CAN bus. Returns \c true on success;
    otherwise \c false.

    This function calls \l open() as part of its implementation.
 */
bool QCanBusDevice::connectDevice()
{
    Q_D(QCanBusDevice);

    if (d->state != QCanBusDevice::UnconnectedState)
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
    Disconnects the device from the CAN bus.

    This function calls \l close() as part of its implementation.
 */
void QCanBusDevice::disconnectDevice()
{
    setState(QCanBusDevice::ClosingState);

    //Unconnected is set by backend -> might be delayed by event loop
    close();
}

/*!
    \fn void QCanBusDevice::stateChanged(QCanBusDevice::CanBusDeviceState state)

    This signal is emitted every time the state of the device changes.
    The new state is represented by \a state.

    \sa setState(), state()
 */

/*!
    Returns the current state of the device.

    \sa setState(), stateChanged()
 */
QCanBusDevice::CanBusDeviceState QCanBusDevice::state() const
{
    return d_func()->state;
}

/*!
    Sets the state of the device to \a newState. CAN bus implementations
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

QT_END_NAMESPACE
