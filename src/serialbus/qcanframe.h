/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
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

#ifndef QCANFRAME_H
#define QCANFRAME_H

#include <QtSerialBus/qserialbusglobal.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE


//TODO: stream operator for this class
//TODO: review ctors for this class
class QCanFrame
{
public:
    class TimeStamp {
    public:
        Q_DECL_CONSTEXPR TimeStamp(qint64 s = 0, qint64 usec = 0) Q_DECL_NOTHROW
            : secs(s), usecs(usec) {}
        Q_DECL_CONSTEXPR qint64 seconds() const Q_DECL_NOTHROW { return secs; }
        Q_DECL_CONSTEXPR qint64 microSeconds() const Q_DECL_NOTHROW { return usecs; }

        inline void setSeconds(qint64 s) { secs = s; }
        inline void setMicroSeconds(qint64 usec) { usecs = usec; }
    private:
        qint64 secs;
        qint64 usecs;
    };

    QCanFrame(qint32 identifier = 0, QByteArray data = "") :
        id(identifier), load(data) {}
    inline void setFrameId(qint32 newFrameId) { id = newFrameId; }
    inline void setPayload(const QByteArray &data) { load = data; }
    inline void setTimeStamp(const TimeStamp &ts) { stamp = ts; }
    inline qint32 frameId() const { return id; }
    QByteArray payload() const { return load; }
    TimeStamp timeStamp() const { return stamp; }

private:
    qint32 id;
    QByteArray load;
    TimeStamp stamp;
};

Q_DECLARE_TYPEINFO(QCanFrame, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(QCanFrame::TimeStamp, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

#endif // QCANFRAME_H
