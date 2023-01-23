// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QCANCOMMONDEFINITIONS_H
#define QCANCOMMONDEFINITIONS_H

#include <QtCore/qsysinfo.h>
#include <QtCore/qtconfigmacros.h>
#include <QtCore/qtypes.h>

#include <QtSerialBus/qtserialbusglobal.h>

QT_BEGIN_NAMESPACE

namespace QtCanBus {

enum class DataSource : quint8 {
    Payload = 0,
    FrameId,
};

enum class DataFormat : quint8 {
    SignedInteger = 0,
    UnsignedInteger,
    Float,
    Double,
    AsciiString,
};

enum class MultiplexState : quint8 {
    None = 0x00,
    MultiplexorSwitch = 0x01,
    MultiplexedSignal = 0x02,
    SwitchAndSignal = MultiplexorSwitch | MultiplexedSignal,
};

enum class UniqueId : quint32 {};

} // namespace QtCanBus

Q_SERIALBUS_EXPORT QtCanBus::UniqueId qbswap(QtCanBus::UniqueId src);

#ifndef QT_NO_DEBUG_STREAM

class QDebug;

Q_SERIALBUS_EXPORT QDebug operator<<(QDebug dbg, QtCanBus::DataSource source);
Q_SERIALBUS_EXPORT QDebug operator<<(QDebug dbg, QtCanBus::DataFormat format);
Q_SERIALBUS_EXPORT QDebug operator<<(QDebug dbg, QtCanBus::MultiplexState state);
Q_SERIALBUS_EXPORT QDebug operator<<(QDebug dbg, QtCanBus::UniqueId uid);

#endif // QT_NO_DEBUG_STREAM

QT_END_NAMESPACE

#endif // QCANCOMMONDEFINITIONS_H
