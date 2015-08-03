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

#include "qmodbusslave.h"
#include "qmodbusslave_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QModBusSlave
    \inmodule QtSerialBus
    \since 5.6

    \brief The QModBusSlave class is the interface class for modbus.

    Modbus network might have multiple slaves. Slaves are read/written by a
    master device \l QModBusMaster. QModBusSlave communicates with a modbus backend providing users
    with a convenient API. The modbus backend must be specified during the object creation.
 */

/*!
    Constructs a modbus slave with the specified \a parent.
 */
QModBusSlave::QModBusSlave(QObject *parent) :
    QModBusDevice(parent)
{
}

/*!
    \fn bool setMap(QModBusSlave::ModBusTable table, quint16 size)
    Map a size for one table. If a table is not set, it's size will be zero (0).
 */

/*!
    \fn int QModBusSlave::slaveId()
    Multiple Modbus devices can be connected together on the same physical link.
    Slave id will be used to filter received messages. Each slave should have unique id.
    Returns slave id.

    \sa setSlaveId()
 */

/*!
    \fn void QModBusSlave::setSlaveId(int id)
    Multiple Modbus devices can be connected together on the same physical link.
    Slave id will be used to filter received messages. Each slave should have unique id.
    Sets \a id as slave id.

    \sa slaveId()
 */

/*!
    \fn bool setADU(QModBusSlave::ApplicationDataUnit adu)
    Set the type of transport (i.e ApplicationDataUnit) to use.
 */

/*!
    \fn bool data(QModBusSlave::ModBusTable table, quint16 address, quint16 &data)

    Read data stored in the slave. Slave has four \a tables and each have their unique
    \a address fields. \a data will be read from the desired field.
    See QModBusDevice::ModBusTable for more information about the tables.
    Returns \c false if address is outside of the map range.

    \sa QModBusDevice::ModBusTable, setData()
 */

/*!
    \fn bool setData(QModBusDevice::ModBusTable table, quint16 address, quint16 data)

    Writes data to the slave. Slave has four \a tables and each have their unique
    \a address fields. \a data will be written to the desired field.
    Returns \c false if address outside of the map range.

    \sa QModBusDevice::ModBusTable, data()
 */

QT_END_NAMESPACE
