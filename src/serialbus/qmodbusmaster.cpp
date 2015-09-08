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

#include "qmodbusmaster.h"

QT_BEGIN_NAMESPACE

/*!
    \class QModBusMaster
    \inmodule QtSerialBus
    \since 5.6

    \brief The QModBusMaster class is the interface class for modbus master device.

    QModBusMaster communicates with the modbus backend providing users with a convenient API.
    The modbus backend must be specified during the object creation.
*/

/*!
    Constructs a modbus master device with the specified \a parent.
 */
QModBusMaster::QModBusMaster(QObject *parent) :
    QModBusDevice(parent)
{
}

/*!
    \internal
*/
QModBusMaster::~QModBusMaster()
{
}

/*!
    \fn QModBusReply *QModBusMaster::write(const QModBusDataUnit &request, int slaveId)

    Sends a request to modify the contents of the data pointed by \a request. Returns a new QModbusReply object
    which emits the finished() signal whenever a positive response for the write request has been received.
    Modbus network may have multiple slaves, each slave has unique \a slaveId.
 */

/*!
    \fn QModBusReply *QModBusMaster::write(const QList<QModBusDataUnit> &requests, int slaveId)

    This is an overloaded function.

    Requests multiple data units to be written. \a requests is used to read the contents of continuous
    block from a specific table. Only one continuous block can be written with a single write operation.
 */

/*!
    \fn QModBusReply *QModBusMaster::read(QModBusDataUnit &request, int slaveId)

    Sends a request to read the contents of the data pointed by \a request. Returns a new QModBusReply object
    which emits the finished() signal whenever data arrives. Modbus network may have multiple slaves,
    each slave has unique \a slaveId.
 */

/*!
    \fn QModBusReply *QModBusMaster::read(QList<QModBusDataUnit> &requests, int slaveId)

    This is an overloaded function.

    Requests multiple data units to be read. \a requests is used to read the contents of continuous
    block from a specific table. Only one continuous block can be read with a single read operation.
 */

QT_END_NAMESPACE
