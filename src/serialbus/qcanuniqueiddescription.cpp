// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qcanuniqueiddescription.h"
#include "qcanuniqueiddescription_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCanUniqueIdDescription
    \inmodule QtSerialBus
    \since 6.5

    \brief The QCanUniqueIdDescription class describes the rules for accessing
    a unique identifier in a \l QCanBusFrame.

    A unique identifier is used to distinguish different CAN bus frames and
    apply proper \l {QCanMessageDescription}s to encode or decode them.
    Different CAN protocols can use different parts of the CAN frame as a unique
    identifier (e.g. the DBC protocol uses the whole FrameId as a unique
    identifier).

    This class contains parameters to specify the unique identifier position
    within a CAN frame in a flexible way:

    \list
        \li The part of the frame which will be used to extract the unique
            identifier (FrameId or payload).
        \li The start bit of the unique identifier, relative to the selected
            part of the frame. The bits are counted starting from the LSB.
        \li The number of bits used to represent the unique identifier.
        \li The endian used to extract the value.
    \endlist

    Check the \l {Data Endianness Processing} section of the
    \l QCanSignalDescription documentation to see how the start bit value
    depends on the data endianness. The approach that is described there is
    also used for unique id description.

    The actual value of a unique identifier is represented by the
    \l QtCanBus::UniqueId type.

    \sa QCanMessageDescription
*/

/*!
    Creates an empty unique identifier description.
*/
QCanUniqueIdDescription::QCanUniqueIdDescription()
    : d(new QCanUniqueIdDescriptionPrivate)
{
}

/*!
    Creates a unique identifier description with the values copied from
    \a other.
*/
QCanUniqueIdDescription::QCanUniqueIdDescription(const QCanUniqueIdDescription &other)
    : d(other.d)
{
}

/*!
    Creates a unique identifier description by moving from \a other.

    \note The moved-from QCanUniqueIdDescription object can only be destroyed or
    assigned to. The effect of calling other functions than the destructor or
    one of the assignment operators is undefined.
*/
QCanUniqueIdDescription::QCanUniqueIdDescription(QCanUniqueIdDescription &&other) noexcept = default;

/*!
    Destroys this unique identifier description.
*/
QCanUniqueIdDescription::~QCanUniqueIdDescription() = default;

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCanUniqueIdDescriptionPrivate)

/*!
    Assigns the values from \a other to this unique identifier description.
*/
QCanUniqueIdDescription &QCanUniqueIdDescription::operator=(const QCanUniqueIdDescription &other)
{
    d = other.d;
    return *this;
}

/*!
    \fn QCanUniqueIdDescription &QCanUniqueIdDescription::operator=(QCanUniqueIdDescription &&other) noexcept

    Move-assigns the values from \a other to this unique identifier description.

    \note The moved-from QCanUniqueIdDescription object can only be destroyed or
    assigned to. The effect of calling other functions than the destructor or
    one of the assignment operators is undefined.
*/

/*!
    \fn bool QCanUniqueIdDescription::operator==(const QCanUniqueIdDescription &lhs, const QCanUniqueIdDescription &rhs)

    Returns \c true if all of the \a lhs object's values are the same as those
    of \a rhs. Otherwise returns \c false.
*/

/*!
    \fn bool QCanUniqueIdDescription::operator!=(const QCanUniqueIdDescription &lhs, const QCanUniqueIdDescription &rhs)

    Returns \c true if any of the \a lhs object's values are not the same as
    those of \a rhs. Otherwise returns \c false.
*/

/*!
    Returns \c true when this unique identifier description is valid and
    \c false otherwise.

    A valid unique identifier description \e must have a \l bitLength() which is
    greater than zero and does not exceed the number of bits of the
    \l QtCanBus::UniqueId type.

    \sa bitLength()
*/
bool QCanUniqueIdDescription::isValid() const
{
    static constexpr auto uidSize = sizeof(QtCanBus::UniqueId) * 8;
    return d->bitLength > 0 && d->bitLength <= uidSize;
}

/*!
    Returns the data source of the unique identifier.

    By default, \l {QtCanBus::}{FrameId} is used.

    \sa setSource(), QtCanBus::DataSource
*/
QtCanBus::DataSource QCanUniqueIdDescription::source() const
{
    return d->source;
}

/*!
    Sets the data source of the unique identifier to \a source.

    \sa source(), QtCanBus::DataSource
*/
void QCanUniqueIdDescription::setSource(QtCanBus::DataSource source)
{
    d.detach();
    d->source = source;
}

/*!
    Returns the start bit of the unique identifier in the \l source().

    \sa setStartBit(), bitLength(), setBitLength()
*/
quint16 QCanUniqueIdDescription::startBit() const
{
    return d->startBit;
}

/*!
    Sets the start bit of the unique identifier in the \l source() to \a bit.

    \sa startBit(), bitLength(), setBitLength()
*/
void QCanUniqueIdDescription::setStartBit(quint16 bit)
{
    d.detach();
    d->startBit = bit;
}

/*!
    Returns the bit length of the unique identifier.

    \sa setBitLength(), startBit(), setStartBit()
*/
quint8 QCanUniqueIdDescription::bitLength() const
{
    return d->bitLength;
}

/*!
    Sets the bit length of the unique identifier to \a length.

    \sa bitLength(), startBit(), setStartBit()
*/
void QCanUniqueIdDescription::setBitLength(quint8 length)
{
    d.detach();
    d->bitLength = length;
}

/*!
    Returns the data endian of the unique identifier.

    By default, \l {QtCanBus::}{LittleEndian} is used.

    \sa setEndian(), QtCanBus::DataEndian
*/
QtCanBus::DataEndian QCanUniqueIdDescription::endian() const
{
    return d->endian;
}

/*!
    Sets the data endian of the unique identifier to \a endian.

    \sa endian(), QtCanBus::DataEndian
*/
void QCanUniqueIdDescription::setEndian(QtCanBus::DataEndian endian)
{
    d.detach();
    d->endian = endian;
}

bool QCanUniqueIdDescription::equals(const QCanUniqueIdDescription &lhs, const QCanUniqueIdDescription &rhs)
{
    return lhs.d->source == rhs.d->source
            && lhs.d->endian == rhs.d->endian
            && lhs.d->startBit == rhs.d->startBit
            && lhs.d->bitLength == rhs.d->bitLength;
}

QCanUniqueIdDescriptionPrivate *QCanUniqueIdDescriptionPrivate::get(const QCanUniqueIdDescription &desc)
{
    return desc.d.data();
}

QT_END_NAMESPACE
