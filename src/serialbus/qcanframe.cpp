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
