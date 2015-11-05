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

#include <QtCore/qobject.h>
#include <QtCore/private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QModbusReplyPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QModbusReply)
public:
    QModbusReplyPrivate() Q_DECL_EQ_DEFAULT;

    QModbusDataUnit m_unit;
    int m_slaveAddress = 0xff;
    bool m_finished = false;
    QModbusPdu::ExceptionCode m_protocolError = QModbusPdu::ExtendedException;
    QModbusReply::ReplyError m_error = QModbusReply::NoError;
    QString m_errorText;
};

/*!
    \class QModbusReply
    \inmodule QtSerialBus
    \since 5.6

    \brief The QModbusReply class contains the data for a request sent with
    a \l QModbusClient derived class.
 */

/*!
    \enum QModbusPdu::ReplyError

    This enum describes all the possible error conditions.

    \value NoError                              No error has occurred.
    \value ProtocolError                        The request was answered with a Modbus exception.
                                                The exact Modbus exception code can be retrieved
                                                using \l protocolError().
    \value TimeoutError                         The Modbus request was not answered within the given
                                                \l QModbusClient::timeout().
    \value ReplyAbortedError                    The reply was aborted due to a disconnection of
                                                the \l QModbusClient.
    \value UnknownError                         An unknown error has occurred.
    \value WriteError                           The request could not be written/sent to the remote party.
*/

/*!
    Constructs a QModbusReply object with the specified \a parent.

    The reply will be send to the Modbus client represented by
    \a slaveAddress.
 */
QModbusReply::QModbusReply(int slaveAddress, QObject *parent) :
    QObject(*new QModbusReplyPrivate, parent)
{
    Q_D(QModbusReply);
    d->m_slaveAddress = slaveAddress;
}

/*!
    Returns \c true when the reply has finished or was aborted.
*/
bool QModbusReply::isFinished() const
{
    Q_D(const QModbusReply);
    return d->m_finished;
}

/*!
    Sets whether or not this reply has finished to \a isFinished.

    If \a isFinished is \a true, this will cause the \l finished() signal to be emitted.

    If the operation completed successfully, \l setResult() should be called before
    this function. If an error occurred, \l setError() should be used instead.
*/
void QModbusReply::setFinished(bool isFinished)
{
    Q_D(QModbusReply);
    d->m_finished = isFinished;
    if (isFinished)
        emit finished();
}

/*!
    \fn void QModbusReply::finished()

    This signal is emitted when the reply has finished processing. The reply may still have
    returned with an error.

    After this signal is emitted, there will be no more updates to the reply's data.

    \note Do not delete the object in the slot connected to this signal. Use deleteLater().

    You can also use \l isFinished() to check if a QNetworkReply has finished even before
    you receive the \l finished() signal.

    \sa isFinished(), error()
 */

/*!
    Returns the result of a standard register read request.

    Combined read/write requests send via \l QModbusClient::sendReadWriteRequest()
    contain the values read from the server/slave instance.

    If the request has not finished, has failed with an error or
    was a write request then the returned \l QModbusDataUnit instance is invalid.
 */
QModbusDataUnit QModbusReply::result() const
{
    Q_D(const QModbusReply);
    return d->m_unit;
}

/*!
    Sets the results of a read/write request to a Modbus register.
  */
void QModbusReply::setResult(const QModbusDataUnit &unit)
{
    Q_D(QModbusReply);
    d->m_unit = unit;
}

/*!
    Returns the slave address that this reply object targets.
*/
int QModbusReply::slaveAddress() const
{
    Q_D(const QModbusReply);
    return d->m_slaveAddress;
}

/*!
    Returns the Modbus exception code that this reply ended up with. It implies
    that the reply finished with a \l ProtocolError.

    \note This function always returns \l QModbusPdu::ExtendedException if \l error()
    is not set to \l ProtocolError.
*/
QModbusPdu::ExceptionCode QModbusReply::protocolError() const
{
    Q_D(const QModbusReply);
    return d->m_protocolError;
}

/*!
    \fn void QModbusReply::errorOccurred(QModbusReply::ReplyError error)

    This signal is emitted when an error has been detected in the processing of this reply.
    The \l finished() signal will probably follow.

    The error will be described by the error code \a error. If errorString is not empty
    it will contain a textual description of the error. In case of a \l ProtocolError
    the \l protocolError() function can be used to obtain the exact type of Modbus exception
    as defined by the Modbus specification.

    Note: Do not delete this reply object in the slot connected to this signal.
    Use \l deleteLater() instead.
*/

/*!
    Returns the error state of this reply.
*/
QModbusReply::ReplyError QModbusReply::error() const
{
    Q_D(const QModbusReply);
    return d->m_error;
}

/*!
    Sets the Modbus exception error state of this reply to \a error and the textual representation of
    the error to \a errorText. This function implicitly sets \l error() to \l ProtocolError.

    This causes the \l errorOccurred() and \l finished() signals to be emitted,
    in that order.
*/
void QModbusReply::setProtocolError(QModbusPdu::ExceptionCode error, const QString &errorText)
{
    Q_D(QModbusReply);
    d->m_error = ProtocolError;
    d->m_protocolError = error;
    d->m_errorText = errorText;
    emit errorOccurred(ProtocolError);
    setFinished(true);
}

/*!
    Sets the error state of this reply to \a error and the textual representation of
    the error to \a errorText.

    This will also cause the \l errorOccurred() and \l finished() signals to be emitted,
    in that order.
*/
void QModbusReply::setError(QModbusReply::ReplyError error, const QString &errorText)
{
    Q_D(QModbusReply);
    d->m_error = error;
    d->m_errorText = errorText;
    emit errorOccurred(error);
    setFinished(true);
}

/*!
    Returns the textual representation of the error state of this reply.

    If no error has occurred this will return an empty string. It is possible
    that an error occurred which has no associated textual representation,
    in which case this will also return an empty string.

    \sa error(), errorOccurred(), setError()
*/
QString QModbusReply::errorText() const
{
    Q_D(const QModbusReply);
    return d->m_errorText;
}

#include "moc_qmodbusreply.cpp"

QT_END_NAMESPACE
