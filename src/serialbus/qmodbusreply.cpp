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
    QObject(parent)
{

}

/*!
    \fn QModBusReply::exceptionOccurred(RequestException exception)

    This signal is emitted when when request is aborted and exception of the type
    \a exception is received.

    \sa QModBusReply::exception()
 */

/*!
    \fn QModBusReply::finished()

    This signal is emitted when request is successfully executed.

    \sa QModBusReply::isFinished()
 */

/*!
    \enum QModBusReply::RequestException
    This enum describes all the possible exception causes.

    \value NoException      No exception has occurred.
 */

/*!
    Returns the \l RequestException that was found during the processing of this request.
    If no exception was found, returns \l NoError.
 */
QModBusReply::RequestException QModBusReply::exception() const
{
    return NoException;
}

/*!
    Returns \c true when the reply has finished or was aborted.

    \sa QModBusReply::finished()
 */
bool QModBusReply::isFinished() const
{
    return false;
}

/*!
    Returns \c true when the request is still processing and the reply has not
    finished or was aborted yet.
 */
bool QModBusReply::isRunning() const
{
    return false;
}

/*!
    Returns data units read/written if QModBusReply is finished.
    Otherwise returns empty QVector.
 */
QVector<QModBusDataUnit> QModBusReply::result() const
{
    return QVector<QModBusDataUnit>();
}

QT_END_NAMESPACE
