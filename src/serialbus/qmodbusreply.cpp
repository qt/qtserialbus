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

#include "qmodbusreply.h"

QT_BEGIN_NAMESPACE

/*!
    \class QModbusReply
    \inmodule QtSerialBus
    \since 5.6

    \brief The QCanBusReply class contains the data and address for the request sent with
    QModbusMaster.
 */

/*!
    Constructs a QModbusReply object with the \a parent.
 */
QModbusReply::QModbusReply(QObject *parent) :
    QObject(parent),
    errorType(NoError),
    finish(false)
{

}

/*!
    \enum QModbusReply::RequestError
    This enum describes the type of error that has occurred.

    \value NoError              No error has occurred.
    \value IllegalFunction      The received request is not an allowable action for the slave.
    \value IllegalDataAddress   The received data address in the query is not an allowable address for
                                the slave.
    \value IllegalDataValue     The contained value in the request data field is not an allowable
                                value for the slave.
    \value SlaveFailure         An irrecoverable error occurred while the slave was attempting to perform
                                the requested action.
    \value Acknowledge          Specialized use in conjunction with programming commands.
    \value SlaveBusy            The slave is engaged in processing a long–duration program command.
    \value MemoryParity         Indicates that the extended file area failed to pass a consistency check.
    \value GatewayUnavailable   Indicates that the gateway was unable to allocate an internal communication
                                path from the input port to the output port for processing the request.
    \value NoResponse           Indicates that no response was obtained from the target device.
    \value InvalidCRC           CRC error occurred.
    \value InvalidError         An unknown error occurred.
 */

/*!
    \fn QModbusReply::errorOccurred(RequestError error)

    This signal is emitted when a request is aborted and error of the type
    \a error is received.

    \sa QModbusReply::error()
 */

/*!
    \fn QModbusReply::finished()

    This signal is emitted when a request is successfully completed.

    \sa QModbusReply::isFinished()
 */

/*!
    \fn void QModbusReply::setFinished()

    Sets the reply as finished.
    After having this set the reply's data must not change.
 */

/*!
    \fn void QModbusReply::setError(RequestError errorCode, const QString &errorString)

    Sets the error condition to be \a errorCode. The human-readable message is set to \a errorString.
 */

/*!
    Returns the \l RequestError that was found while processing of the request.
    If no error was found, returns \l NoError.
 */
QModbusReply::RequestError QModbusReply::error() const
{
    return errorType;
}

/*!
    Returns a human-readable description of the error that occurred. Returns empty string
    if no error has occurred.

    \sa error()
 */
QString QModbusReply::errorString() const
{
    return errorText;
}

/*!
    Returns \c true when the reply has finished or was aborted.

    \sa QModbusReply::finished()
 */
bool QModbusReply::isFinished() const
{
    return finish;
}

/*!
    Returns \c true if the request is still being processed and the reply
    has not finished or is not aborted yet.
 */
bool QModbusReply::isRunning() const
{
    return !finish;
}

/*!
    Returns data units read/written if QModbusReply is finished.
    Otherwise returns an empty QVector.
 */
QList<QModbusDataUnit> QModbusReply::result() const
{
    return payload;
}

QT_END_NAMESPACE
