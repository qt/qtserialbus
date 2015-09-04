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
    \class QModBusReply
    \inmodule QtSerialBus
    \since 5.6

    \brief The QCanBusReply class contains the data and address for the request sent with
    QModBusMaster.
 */

/*!
    Constructs a QModBusReply object with \a parent.
 */
QModBusReply::QModBusReply(QObject *parent) :
    QObject(parent),
    errorType(NoError),
    finish(false)
{

}

/*!
    \enum QModBusReply::RequestError
    This enum describes the type of error that has occurred.

    \value NoError              No error has occurred.
    \value IllegalFunction      The received request is not an allowable action for the slave.
    \value IllegalDataAddress   The received data address in the query is not an allowable address for
                                the slave.
    \value IllegalDataValue     The contained value in the request data field is not an allowable
                                value for the slave.
    \value SlaveFailure         An unrecoverable error occurred while the slave was attempting to perform
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
    \fn QModBusReply::errorOccurred(RequestError error)

    This signal is emitted when when request is aborted and error of the type
    \a error is received.

    \sa QModBusReply::error()
 */

/*!
    \fn QModBusReply::finished()

    This signal is emitted when request is successfully executed.

    \sa QModBusReply::isFinished()
 */

/*!
    \fn void QModBusReply::setFinished()

    Sets the reply as finished.
    After having this set the replies data must not change.
 */

/*!
    \fn void QModBusReply::setError(RequestError errorCode, const QString &errorString)

    Sets the error condition to be \a errorCode. The human-readable message is set with \a errorString.
 */

/*!
    Returns the \l RequestError that was found during the processing of this request.
    If no error was found, returns \l NoError.
 */
QModBusReply::RequestError QModBusReply::error() const
{
    return errorType;
}

/*!
    Returns a human-readable description of the error that occurred. Returns empty string
    if no error has occurred.

    \sa error()
 */
QString QModBusReply::errorString() const
{
    return errorText;
}

/*!
    Returns \c true when the reply has finished or was aborted.

    \sa QModBusReply::finished()
 */
bool QModBusReply::isFinished() const
{
    return finish;
}

/*!
    Returns \c true when the request is still processing and the reply has not
    finished or was aborted yet.
 */
bool QModBusReply::isRunning() const
{
    return !finish;
}

/*!
    Returns data units read/written if QModBusReply is finished.
    Otherwise returns empty QVector.
 */
QList<QModBusDataUnit> QModBusReply::result() const
{
    return payload;
}

QT_END_NAMESPACE
