// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QCANUNIQUEIDDESCRIPTION_H
#define QCANUNIQUEIDDESCRIPTION_H

#include <QtCore/QExplicitlySharedDataPointer>

#include <QtSerialBus/qcancommondefinitions.h>
#include <QtSerialBus/qtserialbusglobal.h>

QT_BEGIN_NAMESPACE

class QCanUniqueIdDescriptionPrivate;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR_WITH_EXPORT(QCanUniqueIdDescriptionPrivate, Q_SERIALBUS_EXPORT)

class Q_SERIALBUS_EXPORT QCanUniqueIdDescription
{
public:
    QCanUniqueIdDescription();
    QCanUniqueIdDescription(const QCanUniqueIdDescription &other);
    QCanUniqueIdDescription(QCanUniqueIdDescription &&other) noexcept;
    ~QCanUniqueIdDescription();

    QCanUniqueIdDescription &operator=(const QCanUniqueIdDescription &other);
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCanUniqueIdDescription)

    friend bool operator==(const QCanUniqueIdDescription &lhs, const QCanUniqueIdDescription &rhs)
    {
        return equals(lhs, rhs);
    }
    friend bool operator!=(const QCanUniqueIdDescription &lhs, const QCanUniqueIdDescription &rhs)
    {
        return !equals(lhs, rhs);
    }

    inline void swap(QCanUniqueIdDescription &other) noexcept { d.swap(other.d); }

    bool isValid() const;

    QtCanBus::DataSource source() const;
    void setSource(QtCanBus::DataSource source);

    quint16 startBit() const;
    void setStartBit(quint16 bit);

    quint8 bitLength() const;
    void setBitLength(quint8 length);

    QtCanBus::DataEndian endian() const;
    void setEndian(QtCanBus::DataEndian endian);

private:
    QExplicitlySharedDataPointer<QCanUniqueIdDescriptionPrivate> d;
    friend class QCanUniqueIdDescriptionPrivate;

    static bool equals(const QCanUniqueIdDescription &lhs, const QCanUniqueIdDescription &rhs);
};

QT_END_NAMESPACE

#endif // QCANUNIQUEIDDESCRIPTION_H
