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

#include "qmodbusserver.h"
#include "qmodbusserver_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QModbusSlave
    \inmodule QtSerialBus
    \since 5.6

    \brief The QModbusSlave class is the interface class for Modbus.

    Modbus networks can have multiple slaves. Slaves are read/written by a
    master device represented by \l QModbusMaster. QModbusSlave communicates
    with a Modbus backend, providing users with a convenient API.
    The Modbus backend must be specified during the object creation.
 */

/*!
    Constructs a Modbus slave with the specified \a parent.
 */
QModbusSlave::QModbusSlave(QObject *parent) :
    QModbusDevice(*new QModbusSlavePrivate, parent)
{
}

/*!
    \internal
*/
QModbusSlave::~QModbusSlave()
{
}

/*!
    \internal
 */
QModbusSlave::QModbusSlave(QModbusSlavePrivate &dd, QObject *parent) :
    QModbusDevice(dd, parent)
{
}

/*!
    \fn bool QModbusSlave::setMap(const QModbusRegister &newRegister) = 0

    Sets the registered map structure for requests from other ModBus masters to \a newRegister.
    The register values are initialized with zero. Returns \c true on success; otherwise \c false.

    If this function is not called before connecting, a default register with zero
    entries is setup.

    \note Calling this function discards any register value that was previously set.
 */

/*!
    \fn int QModbusSlave::slaveId() const
    Multiple Modbus devices can be connected together on the same physical link.
    Slave id is a unique identifier that each slave must have, and it is used
    to filter out incoming messages.

    Returns slave id.

    \sa setSlaveId()
 */

/*!
    \fn void QModbusSlave::setSlaveId(int id)
    Multiple Modbus devices can be connected together on the same physical link.
    So it is important that each slave is identified by a unique id.

    Sets \a id as slave id.

    \sa slaveId()
 */

/*!
    \fn bool QModbusSlave::data(QModbusRegister::RegisterType table, quint16 address, quint16 *data)

    Reads data stored in the slave. Slave has four tables (\a table) and each have a unique
    \a address field, which is used to read \a data from the desired field.
    See QModbusRegister::RegisterType for more information about the different tables.
    Returns \c false if address is outside of the map range.

    \sa QModbusRegister::RegisterType, setData()
 */

/*!
    \fn bool QModbusSlave::setData(QModbusRegister::RegisterType table, quint16 address, quint16 data)

    Writes data to the slave. Slave has four tables (\a table) and each have a unique
    \a address field, which is used to write \a data to the desired field.
    Returns \c false if address outside of the map range.

    \sa QModbusRegister::RegisterType, data()
 */

/*!
    \fn void QModbusSlave::slaveRead()

    This signal is emitted when master has read one or more fields of data from the slave.
 */

/*!
    \fn void QModbusSlave::slaveWritten(QModbusRegister::RegisterType table, int address, int size)

    This signal is emitted when master has written one or more fields of data to the slave.
    Signal contains information about the fields that were written:
    \list
     \li \a table that was written,
     \li \a address of the first field that was written,
     \li and \a size of the consecutive fields that were written starting from \a address.
    \endlist
 */

QT_END_NAMESPACE
