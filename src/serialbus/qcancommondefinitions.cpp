// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qcancommondefinitions.h"

#include <QtCore/qendian.h>

#ifndef QT_NO_DEBUG_STREAM
#include <QtCore/QDebug>
#endif // QT_NO_DEBUG_STREAM


QT_BEGIN_NAMESPACE

/*!
    \namespace QtCanBus
    \inmodule QtSerialBus
    \since 6.5
    \brief The QtCanBus namespace provides some commons enums that are used in
    the CAN bus handling part of the QtSerialPort module.
*/

/*!
    \enum QtCanBus::DataSource

    This enum represents the placement of the data within the CAN frame.

    \value Payload The data will be extracted from the payload.
    \value FrameId The data will be extracted from the frame ID.
*/

/*!
    \enum QtCanBus::DataFormat

    This enum represents the possible data formats. The format defines how the
    value will be extracted from its source.

    \value SignedInteger The signal value is a signed integer.
    \value UnsignedInteger The signal value is an unsigned integer.
    \value Float The signal value is float.
    \value Double The signal value is double.
    \value AsciiString The signal value is an ASCII string.
*/

/*!
    \enum QtCanBus::MultiplexState

    This enum represents the possible multiplex states of a signal.

    \value None The signal is not used in multiplexing.
    \value MultiplexorSwitch The signal is used as a multiplexor switch, which
                             means that other signals depend on the values of
                             this signal.
    \value MultiplexedSignal The signal is multiplexed by some switch, and
                             therefore its value can only be extracted when the
                             switch has a specific value.
    \value SwitchAndSignal The multiplexor switch of the signal must have the
                           value that enables us to use this signal. When used,
                           the signal also acts as a multiplexor switch for
                           other multiplexed signals.
*/

/*!
    \typealias QtCanBus::UniqueId
*/

QtCanBus::UniqueId qbswap(QtCanBus::UniqueId src)
{
    const auto uintval = qbswap_helper(qToUnderlying(src));
    return QtCanBus::UniqueId{uintval};
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, QtCanBus::DataSource source)
{
    QDebugStateSaver saver(dbg);
    switch (source) {
    case QtCanBus::DataSource::Payload:
        dbg << "Payload";
        break;
    case QtCanBus::DataSource::FrameId:
        dbg << "FrameId";
        break;
    }
    return dbg;
}

QDebug operator<<(QDebug dbg, QtCanBus::DataFormat format)
{
    QDebugStateSaver saver(dbg);
    switch (format) {
    case QtCanBus::DataFormat::UnsignedInteger:
        dbg << "UnsignedInteger";
        break;
    case QtCanBus::DataFormat::SignedInteger:
        dbg << "SignedInteger";
        break;
    case QtCanBus::DataFormat::Float:
        dbg << "Float";
        break;
    case QtCanBus::DataFormat::Double:
        dbg << "Double";
        break;
    case QtCanBus::DataFormat::AsciiString:
        dbg << "ASCII";
        break;
    }
    return dbg;
}

QDebug operator<<(QDebug dbg, QtCanBus::MultiplexState state)
{
    QDebugStateSaver saver(dbg);
    switch (state) {
    case QtCanBus::MultiplexState::None:
        dbg << "None";
        break;
    case QtCanBus::MultiplexState::MultiplexorSwitch:
        dbg << "MultiplexorSwitch";
        break;
    case QtCanBus::MultiplexState::MultiplexedSignal:
        dbg << "MultiplexedSignal";
        break;
    case QtCanBus::MultiplexState::SwitchAndSignal:
        dbg << "SwitchAndSignal";
        break;
    }
    return dbg;
}

QDebug operator<<(QDebug dbg, QtCanBus::UniqueId uid)
{
    dbg << qToUnderlying(uid);
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM

QT_END_NAMESPACE
