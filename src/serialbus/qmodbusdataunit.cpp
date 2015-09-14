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

#include "qmodbusdataunit.h"

#include <QtCore/qdatastream.h>

QT_BEGIN_NAMESPACE

/*!
    \class QModBusDataUnit
    \inmodule QtSerialBus
    \since 5.6

    \brief QModBusDataUnit is a container class representing
    single bit and 16 bit word entries in the Modbus register.

    \l QModBusDataUnit can be used for read and write operations.
    The entries are addressed the starting \l startAddress()
    and the \l {valueCount()}{number} of contiguous entries.
    \l registerType() determines which register is used for the
    operations. Not that some registers are read-only registers.

    The actual \l values() are either bit or 16 bit based.
    \l{QModBusDevice::}{DiscreteInputs} and \l{QModBusDevice::}{Coils}
    only accept single bits. Therefore 0 is intepreted as \c 0 and
    anything else \c 1.
 */

/*!
    \fn QModBusDataUnit::QModBusDataUnit(QModBusDevice::ModBusTable registerType, int dataAddress, quint16 initValue)

    Constructs a unit of data for \a registerType.
    Address of the data is \a dataAddress and value of the data is \a initValue.
 */

/*!
    \fn QModBusDataUnit::QModBusDataUnit(QModBusDevice::ModBusTable registerType, int startAddress, const QVector<quint16> &data)

    Constructs a unit of data for \a registerType. The \a data block is placed
    into the register starting at \a startAddress. \l valueCount() is implied
    by the size of \a data.
 */

/*!
    \fn QModBusDataUnit::QModBusDataUnit(QModBusDevice::ModBusTable registerType)

    Constructs a unit of data for \a registerType. \l startAddress() is set to \c 0
    and \l values() is empty.
 */

/*!
    \fn void QModBusDataUnit::setRegisterType(QModBusDevice::ModBusTable registerType)

    Sets the \a registerType.

    \sa registerType(), QModBusDevice::ModBusTable
 */

/*!
    \fn QModBusDevice::ModBusTable QModBusDataUnit::registerType() const

    Returns the type of the register.

    \sa setRegisterType(), QModBusDevice::ModBusTable
 */

/*!
    \fn void QModBusDataUnit::setStartAddress(int address)

    Sets the \a address of the data unit.

    \sa startAddress()
 */

/*!
    \fn int QModBusDataUnit::startAddress() const

    Returns the address of data unit in the register.

    \sa setStartAddress()
 */

/*!
    \fn void QModBusDataUnit::setValues(const QVector<quint16> &values)

    Sets the \a values of the data unit. \l{QModBusDevice::}{DiscreteInputs}
    and \l{QModBusDevice::}{Coils} tables only accept single bit
    value, so 0 is interpreted as 0 and anything else as 1.

    \sa values()
 */

/*!
    \fn QVector<quint16> QModBusDataUnit::values() const

    Returns the data in the data unit. \l{QModBusDevice::}{DiscreteInputs}
    and \l{QModBusDevice::}{Coils} tables only accept single bit
    value, so 0 is interpreted as 0 and anything else as 1.

    \sa setValues()
 */

/*!
    \fn int QModBusDataUnit::valueCount() const

    Returns the size of \l values().

    This function may contain a count that is larger than \l values() size.
    Since this class is used to request data from the remote data register
    the \l valueCount() can be used to indicate the size of the register's
    data block. Once the request has returned the \l valueCount() is equal to
    the size of \l values().

    \sa setValueCount()
 */

/*!
    \fn void QModBusDataUnit::setValueCount(int newCount)

    Sets the size of the requested register's data block to \a newCount.

    This may be different from \l values() size as this function is used
    to indicated the size of a data request. Only once the data request
    has been processed \l valueCount() is equal to the size of \l values().
 */

QT_END_NAMESPACE
