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

#ifndef QCANBUSFRAME_H
#define QCANBUSFRAME_H

#include <QtCore/qmetatype.h>
#include <QtCore/qobject.h>
#include <QtSerialBus/qserialbusglobal.h>

QT_BEGIN_NAMESPACE

class QDataStream;

class QCanBusFrame
{
public:
    class TimeStamp {
    public:
        Q_DECL_CONSTEXPR TimeStamp(qint64 s = 0, qint64 usec = 0) Q_DECL_NOTHROW
            : secs(s), usecs(usec) {}
        Q_DECL_CONSTEXPR qint64 seconds() const Q_DECL_NOTHROW { return secs; }
        Q_DECL_CONSTEXPR qint64 microSeconds() const Q_DECL_NOTHROW { return usecs; }

        Q_DECL_RELAXED_CONSTEXPR void setSeconds(qint64 s) Q_DECL_NOTHROW { secs = s; }
        Q_DECL_RELAXED_CONSTEXPR void setMicroSeconds(qint64 usec) Q_DECL_NOTHROW { usecs = usec; }
    private:
        qint64 secs;
        qint64 usecs;
    };

    enum FrameType {
        UnknownFrame        = 0x0,
        DataFrame           = 0x1,
        ErrorFrame          = 0x2,
        RemoteRequestFrame  = 0x3,
        InvalidFrame        = 0x4
    };

    explicit QCanBusFrame(QCanBusFrame::FrameType type) :
        canId(0x0),
        isExtendedFrame(0x0),
        version(0x0)
    {
        setFrameType(type);
    }

    enum FrameError {
        NoError                     = 0,
        TransmissionTimeoutError    = (1 << 0),
        LostArbitrationError        = (1 << 1),
        ControllerError             = (1 << 2),
        ProtocolViolationError      = (1 << 3),
        TransceiverError            = (1 << 4),
        MissingAcknowledgmentError  = (1 << 5),
        BusOffError                 = (1 << 6),
        BusError                    = (1 << 7),
        ControllerRestartError      = (1 << 8),
        UnknownError                = (1 << 9),
        AnyError                    = 0x1FFFFFFFU
        //only 29 bits usable
    };
    Q_DECLARE_FLAGS(FrameErrors, FrameError)
    Q_FLAGS(FrameErrors)

    explicit QCanBusFrame(quint32 identifier = 0, const QByteArray &data = QByteArray()) :
        canId(identifier & 0x1FFFFFFFU),
        format(DataFrame),
        isExtendedFrame((identifier & 0x1FFFF800U) ? 0x1 : 0x0),
        version(0x0),
        load(data)
    {
        Q_UNUSED(extra);
        Q_UNUSED(reserved);
    }

    bool isValid() const
    {
        if (format == InvalidFrame)
            return false;

        // long id used, but extended flag not set
        if (!isExtendedFrame && (canId & 0x1FFFF800U))
            return false;

        // maximum permitted payload size in CAN FD
        if (load.length() > 64)
            return false;

        return true;
    }

    FrameType frameType() const
    {
        switch (format) {
        case 0x1: return DataFrame;
        case 0x2: return ErrorFrame;
        case 0x3: return RemoteRequestFrame;
        case 0x4: return InvalidFrame;
        // no default to trigger warning
        }

        return UnknownFrame;
    }

    void setFrameType(FrameType newFormat)
    {
        switch (newFormat) {
        case DataFrame:
            format = 0x1; return;
        case ErrorFrame:
            format = 0x2; return;
        case RemoteRequestFrame:
            format = 0x3; return;
        case UnknownFrame:
            format = 0x0; return;
        case InvalidFrame:
            format = 0x4; return;
        }
    }

    inline bool hasExtendedFrameFormat() const { return (isExtendedFrame & 0x1); }
    inline void setExtendedFrameFormat(bool isExtended)
    {
        isExtendedFrame = (isExtended & 0x1);
    }

    inline quint32 frameId() const
    {
        if (format == ErrorFrame)
            return 0;
        return (canId & 0x1FFFFFFFU);
    }
    inline void setFrameId(quint32 newFrameId)
    {
        canId = (newFrameId & 0x1FFFFFFFU);
        setExtendedFrameFormat(isExtendedFrame || (newFrameId & 0x1FFFF800U));
    }

    inline void setPayload(const QByteArray &data) { load = data; }
    inline void setTimeStamp(const TimeStamp &ts) { stamp = ts; }

    QByteArray payload() const { return load; }
    TimeStamp timeStamp() const { return stamp; }

    QCanBusFrame::FrameErrors error() const
    {
        if (format != ErrorFrame)
            return QCanBusFrame::FrameErrors(QCanBusFrame::NoError);

        return FrameErrors(canId & 0x1FFFFFFFU);
    }
    void setError(QCanBusFrame::FrameErrors e)
    {
        if (format != ErrorFrame)
            return;
        canId = (e & AnyError);
    }

#ifndef QT_NO_DATASTREAM
    friend Q_SERIALBUS_EXPORT QDataStream &operator<<(QDataStream &, const QCanBusFrame &);
    friend Q_SERIALBUS_EXPORT QDataStream &operator>>(QDataStream &, QCanBusFrame &);
#endif

private:
    quint32 canId:29; // acts as container for error codes too
    quint8 format:3; // max of 8 frame types

    quint8 isExtendedFrame:1;
    quint8 version:5;
    quint8 extra: 2; // unused

    // reserved for future use
    quint8 reserved[3];

    QByteArray load;
    TimeStamp stamp;
};

Q_DECLARE_TYPEINFO(QCanBusFrame, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(QCanBusFrame::FrameError, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QCanBusFrame::FrameType, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QCanBusFrame::TimeStamp, Q_PRIMITIVE_TYPE);

Q_DECLARE_OPERATORS_FOR_FLAGS(QCanBusFrame::FrameErrors)

#ifndef QT_NO_DATASTREAM
Q_SERIALBUS_EXPORT QDataStream &operator<<(QDataStream &, const QCanBusFrame &);
Q_SERIALBUS_EXPORT QDataStream &operator>>(QDataStream &, QCanBusFrame &);
#endif

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QCanBusFrame::FrameType)
Q_DECLARE_METATYPE(QCanBusFrame::FrameErrors)

#endif // QCANBUSFRAME_H
