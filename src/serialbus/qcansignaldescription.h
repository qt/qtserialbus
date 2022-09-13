// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QCANSIGNALDESCRIPTION_H
#define QCANSIGNALDESCRIPTION_H

#include <QtCore/QDebug>
#include <QtCore/QExplicitlySharedDataPointer>

#include <QtSerialBus/qcancommondefinitions.h>
#include <QtSerialBus/qtserialbusglobal.h>

QT_BEGIN_NAMESPACE

class QCanSignalDescriptionPrivate;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR_WITH_EXPORT(QCanSignalDescriptionPrivate, Q_SERIALBUS_EXPORT)

class Q_SERIALBUS_EXPORT QCanSignalDescription
{
public:
    using MultiplexValues = QList<QPair<QVariant, QVariant>>;
    using MultiplexSignalValues = QHash<QString, MultiplexValues>;


    QCanSignalDescription();
    QCanSignalDescription(const QCanSignalDescription &other);
    QCanSignalDescription(QCanSignalDescription &&other) noexcept;
    ~QCanSignalDescription();

    QCanSignalDescription &operator=(const QCanSignalDescription &other);
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCanSignalDescription)

    friend bool operator==(const QCanSignalDescription &lhs, const QCanSignalDescription &rhs)
    {
        return equals(lhs, rhs);
    }
    friend bool operator!=(const QCanSignalDescription &lhs, const QCanSignalDescription &rhs)
    {
        return !equals(lhs, rhs);
    }

    void swap(QCanSignalDescription &other) noexcept { d.swap(other.d); }

    bool isValid() const;

    QString name() const;
    void setName(const QString &name);

    QString physicalUnit() const;
    void setPhysicalUnit(const QString &unit);

    QString receiver() const;
    void setReceiver(const QString &receiver);

    QString comment() const;
    void setComment(const QString &text);

    QtCanBus::DataSource dataSource() const;
    void setDataSource(QtCanBus::DataSource source);

    QtCanBus::DataEndian dataEndian() const;
    void setDataEndian(QtCanBus::DataEndian endian);

    QtCanBus::DataFormat dataFormat() const;
    void setDataFormat(QtCanBus::DataFormat format);

    quint16 startBit() const;
    void setStartBit(quint16 bit);

    quint16 bitLength() const;
    void setBitLength(quint16 length);

    double factor() const;
    void setFactor(double factor);

    double offset() const;
    void setOffset(double offset);

    double scaling() const;
    void setScaling(double scaling);

    double minimum() const;
    double maximum() const;
    void setRange(double minimum, double maximum);

    QtCanBus::MultiplexState multiplexState() const;
    void setMultiplexState(QtCanBus::MultiplexState state);

    MultiplexSignalValues multiplexSignals() const;
    void clearMultiplexSignals();
    void setMultiplexSignals(const MultiplexSignalValues &multiplexorSignals);
    void addMultiplexSignal(const QString &name, const MultiplexValues &ranges);
    void addMultiplexSignal(const QString &name, const QVariant &value);

private:
    QExplicitlySharedDataPointer<QCanSignalDescriptionPrivate> d;
    friend class QCanSignalDescriptionPrivate;

    static bool equals(const QCanSignalDescription &lhs, const QCanSignalDescription &rhs);

#ifndef QT_NO_DEBUG_STREAM
    friend QDebug operator<<(QDebug dbg, const QCanSignalDescription &sig)
    {
        return debugStreaming(dbg, sig);
    }
    static QDebug debugStreaming(QDebug dbg, const QCanSignalDescription &sig);
#endif // QT_NO_DEBUG_STREAM
};

Q_DECLARE_SHARED(QCanSignalDescription)

QT_END_NAMESPACE

#endif // QCANSIGNALDESCRIPTION_H
