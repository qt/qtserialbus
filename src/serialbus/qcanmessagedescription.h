// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QCANMESSAGEDESCRIPTION_H
#define QCANMESSAGEDESCRIPTION_H

#include <QtCore/QDebug>
#include <QtCore/QExplicitlySharedDataPointer>

#include <QtSerialBus/qcancommondefinitions.h>
#include <QtSerialBus/qtserialbusglobal.h>

QT_BEGIN_NAMESPACE

class QCanSignalDescription;

class QCanMessageDescriptionPrivate;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR_WITH_EXPORT(QCanMessageDescriptionPrivate, Q_SERIALBUS_EXPORT)

class Q_SERIALBUS_EXPORT QCanMessageDescription
{
public:
    QCanMessageDescription();
    QCanMessageDescription(const QCanMessageDescription &other);
    QCanMessageDescription(QCanMessageDescription &&other) noexcept = default;
    ~QCanMessageDescription() = default;

    QCanMessageDescription &operator=(const QCanMessageDescription &other);
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCanMessageDescription)

    friend bool operator==(const QCanMessageDescription &lhs, const QCanMessageDescription &rhs)
    {
        return equals(lhs, rhs);
    }
    friend bool operator!=(const QCanMessageDescription &lhs, const QCanMessageDescription &rhs)
    {
        return !equals(lhs, rhs);
    }

    void swap(QCanMessageDescription &other) noexcept { d.swap(other.d); }

    bool isValid() const;

    QtCanBus::UniqueId uniqueId() const;
    void setUniqueId(QtCanBus::UniqueId id);

    QString name() const;
    void setName(const QString &name);

    quint8 size() const;
    void setSize(quint8 size);

    QString transmitter() const;
    void setTransmitter(const QString &transmitter);

    QString comment() const;
    void setComment(const QString &text);

    QList<QCanSignalDescription> signalDescriptions() const;
    QCanSignalDescription signalDescriptionForName(const QString &name) const;
    void clearSignalDescriptions();
    void addSignalDescription(const QCanSignalDescription &description);
    void setSignalDescriptions(const QList<QCanSignalDescription> &descriptions);

private:
    QExplicitlySharedDataPointer<QCanMessageDescriptionPrivate> d;
    friend class QCanMessageDescriptionPrivate;

    static bool equals(const QCanMessageDescription &lhs, const QCanMessageDescription &rhs);

    friend void qHash(const QCanMessageDescription &desc, size_t seed) noexcept = delete;

#ifndef QT_NO_DEBUG_STREAM
    friend QDebug operator<<(QDebug dbg, const QCanMessageDescription &msg)
    {
        return debugStreaming(dbg, msg);
    }
    static QDebug debugStreaming(QDebug dbg, const QCanMessageDescription &msg);
#endif // QT_NO_DEBUG_STREAM
};

Q_DECLARE_SHARED(QCanMessageDescription)

QT_END_NAMESPACE

#endif // QCANMESSAGEDESCRIPTION_H
