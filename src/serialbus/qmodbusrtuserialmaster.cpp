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

#include "qmodbusrtuserialmaster.h"
#include "qmodbusrtuserialmaster_p.h"

#include <QtSerialBus/qmodbus.h>

QT_BEGIN_NAMESPACE

/*!
    \class QModbusRtuSerialMaster
    \inmodule QtSerialBus
    \since 5.6

    \brief The QModbusRtuSerialMaster class represents a Modbus client
    that uses a serial bus for its communication with the Modbus server.

    Communication via Modbus requires the interaction between a single
    Modbus client instance and multiple Modbus servers. This class
    provides the server implementation via a serial port.
*/

/*!
    Constructs a serial Modbus master with the specified \a parent.
 */
QModbusRtuSerialMaster::QModbusRtuSerialMaster(QObject *parent)
    : QModbusClient(*new QModbusRtuSerialMasterPrivate, parent)
{
    Q_D(QModbusRtuSerialMaster);
    d->setupMaster();
}

/*!
    \internal
 */
QModbusRtuSerialMaster::~QModbusRtuSerialMaster()
{
}

/*!
    Connects the Modbus master to a serial port. \a deviceName
    represents the serial port to which the Modbus servers are connected.

    The function returns \c true on success; otherwise \c false.

    \sa disconnectDevice()
 */
bool QModbusRtuSerialMaster::connectDevice(const QString &deviceName)
{
    Q_D(QModbusRtuSerialMaster);

    if (!d->pluginMaster)
        return false;

    // TODO remove setPortName() usage
    d->pluginMaster->setPortName(deviceName);

    // TODO Implement QModbusRtuSerialMaster::connectDevice(QString)

    return d->pluginMaster->connectDevice();
}

/*!
    \internal
*/
QModbusRtuSerialMaster::QModbusRtuSerialMaster(QModbusRtuSerialMasterPrivate &dd, QObject *parent)
    : QModbusClient(dd, parent)
{
    Q_D(QModbusRtuSerialMaster);
    d->setupMaster();
}

/*!
    \reimp
 */
bool QModbusRtuSerialMaster::open()
{
    // TODO remove later on
    // The function is not needed anymore as connectDevice(QString)
    // does not rely on it anymore.
    return false;
}

/*!
    \reimp
 */
void QModbusRtuSerialMaster::close()
{
    Q_D(QModbusRtuSerialMaster);

    if (!d->pluginMaster)
        return;

    d->pluginMaster->disconnectDevice();
}

// forward the state changes
void QModbusRtuSerialMasterPrivate::handleStateChanged(QModbusDevice::ModBusDeviceState state)
{
    Q_Q(QModbusRtuSerialMaster);
    q->setState(state);
}

// forward the error changes
void QModbusRtuSerialMasterPrivate::handleErrorOccurred(QModbusDevice::ModBusError error)
{
    Q_Q(QModbusRtuSerialMaster);
    q->setError(pluginMaster ? pluginMaster->errorString() : QString(), error);
}

/*!
    \reimp
 */
QModbusReply *QModbusRtuSerialMaster::write(const QModbusDataUnit &request, int slaveId)
{
    Q_D(QModbusRtuSerialMaster);

    if (!d->pluginMaster)
        return 0;

    return d->pluginMaster->write(request, slaveId);
}

/*!
    \reimp
 */
QModbusReply *QModbusRtuSerialMaster::read(const QModbusDataUnit &request, int slaveId)
{
    Q_D(QModbusRtuSerialMaster);

    if (!d->pluginMaster)
        return 0;

    return d->pluginMaster->read(request, slaveId);

}

/*
    TODO: implement
*/
QModbusReplyEx *QModbusRtuSerialMaster::sendRequest(const QModbusDataUnit &request, int slaveId)
{
    Q_UNUSED(request)
    Q_UNUSED(slaveId)
    return Q_NULLPTR;
}

#include "moc_qmodbusrtuserialmaster.cpp"

QT_END_NAMESPACE
