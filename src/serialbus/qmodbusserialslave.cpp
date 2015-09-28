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

#include "qmodbusserialslave.h"
#include "qmodbusserialslave_p.h"

#include <QtSerialBus/qmodbus.h>


QT_BEGIN_NAMESPACE

/*!
    \class QModbusSerialSlave
    \inmodule QtSerialBus
    \since 5.6

    \brief The QModbusSerialSlave class represents a Modbus slave/server
    that uses a serial port for its communication with the master/client.

    Communication via Modbus requires the interaction between a single
    master/client instance and multiple slaves/server. This class
    provides the slave implementation via a serial port.

    Since multiple slave instances can interact with a master at the same time
    (using a serial bus), slaves are identified by their \l slaveId().
*/

/*!
    Constructs a serial Modbus slave with the specified \a parent.
 */
QModbusSerialSlave::QModbusSerialSlave(QObject *parent)
    : QModbusServer(*new QModbusSerialSlavePrivate, parent)
{
    Q_D(QModbusSerialSlave);
    d->setupMaster();
}

/*!
    \internal
 */
QModbusSerialSlave::~QModbusSerialSlave()
{
}

/*!
    \reimp
 */
bool QModbusSerialSlave::connectDevice()
{
    // reimp to avoid QModbusDevice implementaion
    // we want to use a forwarded version.

    Q_D(QModbusSerialSlave);

    if (!d->pluginMaster)
        return false;

    d->pluginMaster->setPortName(portName());

    return d->pluginMaster->connectDevice();
}

/*!
    \internal
 */
QModbusSerialSlave::QModbusSerialSlave(QModbusSerialSlavePrivate &dd,
                                       QObject *parent)
    : QModbusServer(dd, parent)
{
    Q_D(QModbusSerialSlave);
    d->setupMaster();
}


/*!
    \reimp
 */
bool QModbusSerialSlave::setMap(const QModbusRegister &newRegister)
{
    Q_D(QModbusSerialSlave);

    if (!d->pluginMaster)
        return false;

    return d->pluginMaster->setMap(newRegister);
}

/*!
    \reimp
 */
void QModbusSerialSlave::setSlaveId(int id)
{
    Q_D(QModbusSerialSlave);

    if (!d->pluginMaster)
        return;

    d->pluginMaster->setSlaveId(id);
}

/*!
    \reimp
 */
int QModbusSerialSlave::slaveId() const
{
    Q_D(const QModbusSerialSlave);

    if (!d->pluginMaster)
        return -1;

    return d->pluginMaster->slaveId();
}

/*!
    \reimp
 */
bool QModbusSerialSlave::data(QModbusRegister::RegisterType table,
                              quint16 address, quint16 *data)
{
    Q_D(QModbusSerialSlave);

    if (!d->pluginMaster)
        return false;

    return d->pluginMaster->data(table, address, data);
}

/*!
    \reimp
 */
bool QModbusSerialSlave::setData(QModbusRegister::RegisterType table,
                                 quint16 address, quint16 data)
{
    Q_D(QModbusSerialSlave);

    if (!d->pluginMaster)
        return false;

    return d->pluginMaster->setData(table, address, data);
}

/*!
    \reimp
 */
bool QModbusSerialSlave::open()
{
    return false;
}

/*!
    \reimp
 */
void QModbusSerialSlave::close()
{
    Q_D(QModbusSerialSlave);

    if (!d->pluginMaster)
        return;

    d->pluginMaster->disconnectDevice();
}


void QModbusSerialSlavePrivate::handleStateChanged(
                                    QModbusDevice::ModBusDeviceState state)
{
    Q_Q(QModbusSerialSlave);
    q->setState(state);
}

void QModbusSerialSlavePrivate::handleErrorOccurred(
                                    QModbusDevice::ModBusError)
{
    Q_Q(QModbusSerialSlave);
    q->setError(pluginMaster ? pluginMaster->errorString() : QString(),
                error);
}

#include "moc_qmodbusserialslave.cpp"

QT_END_NAMESPACE
