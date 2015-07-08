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

#include "qcanframe.h"

#include <QtCore/qdatastream.h>

QT_BEGIN_NAMESPACE

/*!
    \class QCanFrame
    \inmodule QtSerialBus
    \since 5.6

    \brief QCanFrame is a container class representing a single CAN frame.

    \l QCanBusDevice can use QCanFrame for read and write operations. It contains the frame
    identifier and the data payload. QCanFrame contains the timestamp of the moment it was read.

    \sa QCanFrame::TimeStamp
 */

/*!
    \fn QCanFrame::QCanFrame(qint32 identifier, QByteArray data)

    Constructs a CAN frame using \a identifier as the frame identifier and \a data as the payload.
 */

/*!
    \fn QCanFrame::setFrameId(qint32 newFrameId)

    Sets the identifier of the CAN frame to \a newFrameId. The maximum size of a CAN frame
    identifier is 11 bits, which can be extended to 29 bits by supporting the \e {CAN extended frame
    format}.

    \sa frameId()
 */

/*!
    \fn QCanFrame::setPayload(const QByteArray &data)

    Sets \a data as the payload for the CAN frame. The maximum size of payload is 8 bytes, which can
    be extended to 64 bytes by supporting \e {Flexible Data-Rate}.

    \sa payload()
 */

/*!
    \fn QCanFrame::setTimeStamp(const TimeStamp &ts)

    Sets \a ts as the timestamp for the CAN frame. Usually, this function is not needed, because the
    timestamp is created during the read operation and not needed during the write operation.

    \sa QCanFrame::TimeStamp
 */

/*!
    \fn qint32 QCanFrame::frameId() const

    Returns the CAN frame identifier.

    \sa setFrameId()
 */

/*!
    \fn QByteArray QCanFrame::payload() const

    Returns the data payload of the frame.

    \sa setPayload()
 */

/*!
    \fn TimeStamp QCanFrame::timeStamp() const

    Returns the timestamp of the frame.

    \sa QCanFrame::TimeStamp, QCanFrame::setTimeStamp()
 */

/*!
    \class QCanFrame::TimeStamp
    \inmodule QtSerialBus
    \since 5.6

    \brief The TimeStamp class provides timestamp information with microsecond precision.
 */

/*!
    \fn TimeStamp::TimeStamp(qint64 s, qint64 usec)

    Constructs a TimeStamp in seconds, \a s, and microseconds, \a usec.
 */

/*!
    \fn qint64 TimeStamp::seconds() const

    Returns the seconds of the timestamp.

    \sa TimeStamp::setSeconds()
 */

/*!
    \fn qint64 TimeStamp::microSeconds() const

    Returns the microseconds of the timestamp.

    \sa TimeStamp::setMicroSeconds
 */

/*!
    \fn TimeStamp::setSeconds(qint64 s)

    Sets the seconds in the timestamp type to \a s.

    \sa TimeStamp::seconds()
 */

/*!
    \fn TimeStamp::setMicroSeconds(qint64 usec)

    Sets the microseconds in the timestamp type to \a usec.

    \sa TimeStamp::microSeconds
 */

#ifndef QT_NO_DATASTREAM

/*! \relates QCanFrame

    Writes frame \a frame to the stream \a out and returns a reference
    to the stream.
*/
QDataStream &operator<<(QDataStream &out, const QCanFrame &frame)
{
    out << frame.frameId();
    out << frame.payload();
    const QCanFrame::TimeStamp stamp = frame.timeStamp();
    out << stamp.seconds();
    out << stamp.microSeconds();
    return out;
}

/*! \relates QCanFrame

    Reads a frame into \a frame from the stream \a in and returns a
    reference to the stream.
*/
QDataStream &operator>>(QDataStream &in, QCanFrame &frame)
{
    qint32 frameId;
    QByteArray payload;
    qint64 seconds;
    qint64 microSeconds;
    in >> frameId >> payload >> seconds >> microSeconds;
    frame.setFrameId(frameId);
    frame.setPayload(payload);
    frame.setTimeStamp(QCanFrame::TimeStamp(seconds, microSeconds));
    return in;
}

#endif // QT_NO_DATASTREAM

QT_END_NAMESPACE
