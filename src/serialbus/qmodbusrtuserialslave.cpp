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

#include "qmodbusrtuserialslave.h"
#include "qmodbusrtuserialslave_p.h"

#include <QtSerialBus/qmodbus.h>


QT_BEGIN_NAMESPACE

/*!
    \class QModbusRtuSerialSlave
    \inmodule QtSerialBus
    \since 5.6

    \brief The QModbusRtuSerialSlave class represents a Modbus server
    that uses a serial port for its communication with the Modbus client.

    Communication via Modbus requires the interaction between a single Modbus client instance and
    multiple Modbus server. This class provides the Modbus server implementation via a serial port.

    Since multiple Modbus server instances can interact with a Modbus client at the same time
    (using a serial bus), servers are identified by their \l slaveId().
*/

/*!
    Constructs a serial Modbus slave with the specified \a parent.
 */
QModbusRtuSerialSlave::QModbusRtuSerialSlave(QObject *parent)
    : QModbusServer(*new QModbusRtuSerialSlavePrivate, parent)
{
    Q_D(QModbusRtuSerialSlave);
    d->setupMaster();
}

/*!
    \internal
 */
QModbusRtuSerialSlave::~QModbusRtuSerialSlave()
{
}

/*!
    \reimp
 */
bool QModbusRtuSerialSlave::connectDevice()
{
    // reimp to avoid QModbusDevice implementation
    // we want to use a forwarded version.

    Q_D(QModbusRtuSerialSlave);

    if (!d->pluginMaster)
        return false;

    d->pluginMaster->setPortName(portName());

    return d->pluginMaster->connectDevice();
}

/*!
    \internal
 */
QModbusRtuSerialSlave::QModbusRtuSerialSlave(QModbusRtuSerialSlavePrivate &dd, QObject *parent)
    : QModbusServer(dd, parent)
{
    Q_D(QModbusRtuSerialSlave);
    d->setupMaster();
}

/*!
    \reimp
 */
bool QModbusRtuSerialSlave::setMap(const QModbusDataUnitMap &map)
{
    Q_D(QModbusRtuSerialSlave);

    if (!d->pluginMaster)
        return false;

    return d->pluginMaster->setMap(map);
}

/*!
    \reimp
 */
void QModbusRtuSerialSlave::setSlaveId(int id)
{
    Q_D(QModbusRtuSerialSlave);

    if (!d->pluginMaster)
        return;

    d->pluginMaster->setSlaveId(id);
}

/*!
    \reimp
 */
int QModbusRtuSerialSlave::slaveId() const
{
    Q_D(const QModbusRtuSerialSlave);

    if (!d->pluginMaster)
        return -1;

    return d->pluginMaster->slaveId();
}

/*!
    \reimp
 */
bool QModbusRtuSerialSlave::data(QModbusDataUnit::RegisterType table, quint16 address,
    quint16 *data)
{
    Q_D(QModbusRtuSerialSlave);

    if (!d->pluginMaster)
        return false;

    return d->pluginMaster->data(table, address, data);
}

/*!
    \reimp
 */
bool QModbusRtuSerialSlave::setData(QModbusDataUnit::RegisterType table, quint16 address,
    quint16 data)
{
    Q_D(QModbusRtuSerialSlave);

    if (!d->pluginMaster)
        return false;

    return d->pluginMaster->setData(table, address, data);
}

/*!
    \reimp
 */
bool QModbusRtuSerialSlave::open()
{
    return false;
}

/*!
    \reimp
 */
void QModbusRtuSerialSlave::close()
{
    Q_D(QModbusRtuSerialSlave);

    if (!d->pluginMaster)
        return;

    d->pluginMaster->disconnectDevice();
}

void QModbusRtuSerialSlavePrivate::handleStateChanged(QModbusDevice::ModbusDeviceState state)
{
    Q_Q(QModbusRtuSerialSlave);
    q->setState(state);
}

void QModbusRtuSerialSlavePrivate::handleErrorOccurred(QModbusDevice::ModbusError error)
{
    Q_Q(QModbusRtuSerialSlave);
    q->setError(pluginMaster ? pluginMaster->errorString() : QString(), error);
}

#include "moc_qmodbusrtuserialslave.cpp"

QT_END_NAMESPACE
