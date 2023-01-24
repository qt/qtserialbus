// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QCANMESSAGEDESCRIPTION_HELPERS_H
#define QCANMESSAGEDESCRIPTION_HELPERS_H

#include <QtSerialBus/qcanmessagedescription.h>

#include "qcansignaldescription_helpers.h"

QT_BEGIN_NAMESPACE

inline bool operator==(const QCanMessageDescription &lhs,
                       const QCanMessageDescription &rhs) noexcept
{
    return lhs.uniqueId() == rhs.uniqueId()
            && lhs.name() == rhs.name()
            && lhs.size() == rhs.size()
            && lhs.transmitter() == rhs.transmitter()
            && lhs.comment() == rhs.comment()
            && lhs.signalDescriptions() == rhs.signalDescriptions();
}

inline bool operator!=(const QCanMessageDescription &lhs,
                       const QCanMessageDescription &rhs) noexcept
{
    return !(lhs == rhs);
}

QT_END_NAMESPACE

#endif // QCANMESSAGEDESCRIPTION_HELPERS_H
