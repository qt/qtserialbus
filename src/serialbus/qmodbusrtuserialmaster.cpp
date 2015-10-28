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
    // TODO: Implement!
}

/*!
    \internal
 */
QModbusRtuSerialMaster::~QModbusRtuSerialMaster()
{
}

/*!
    \internal
*/
QModbusRtuSerialMaster::QModbusRtuSerialMaster(QModbusRtuSerialMasterPrivate &dd, QObject *parent)
    : QModbusClient(dd, parent)
{
    // TODO: Implement!
}

QModbusReplyEx *QModbusRtuSerialMaster::sendReadRequest(const QModbusDataUnit &read, int slaveId)
{
    // TODO: Implement!
    Q_UNUSED(read)
    Q_UNUSED(slaveId)
    return Q_NULLPTR;
}

QModbusReplyEx *QModbusRtuSerialMaster::sendWriteRequest(const QModbusDataUnit &write, int slaveId)
{
    // TODO: Implement!
    Q_UNUSED(write)
    Q_UNUSED(slaveId)
    return Q_NULLPTR;
}

QModbusReplyEx *QModbusRtuSerialMaster::sendReadWriteRequest(const QModbusDataUnit &read,
                                                         const QModbusDataUnit &write, int slaveId)
{
    // TODO: Implement!
    Q_UNUSED(read)
    Q_UNUSED(write)
    Q_UNUSED(slaveId)
    return Q_NULLPTR;
}

QT_END_NAMESPACE
