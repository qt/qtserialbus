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
#include "qmodbusmaster_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QModbusMaster
    \inmodule QtSerialBus
    \since 5.6

    \brief The QModbusMaster class is the interface class for Modbus master device.

    QModbusMaster communicates with the Modbus backend providing users with a convenient API.
    The Modbus backend must be specified during the object creation.
*/

/*!
    Constructs a Modbus master device with the specified \a parent.
 */
QModbusMaster::QModbusMaster(QObject *parent)
    : QModbusDevice(*new QModbusMasterPrivate, parent)
{
}

/*!
    \internal
*/
QModbusMaster::~QModbusMaster()
{
}

/*!
    \internal
*/
QModbusMaster::QModbusMaster(QModbusMasterPrivate &dd, QObject *parent) :
    QModbusDevice(dd, parent)
{

}

/*!
    \fn QModbusReply *QModbusMaster::write(const QModbusDataUnit &request, int slaveId)

    Sends a request to modify the contents of the data pointed by \a request. Returns a
    new QModbusReply object, which emits the finished() signal whenever a positive response
    for the write request is received. Modbus network may have multiple slaves, each slave has
    a unique \a slaveId.
 */

/*!
    \fn QModbusReply *QModbusMaster::read(const QModbusDataUnit &request, int slaveId)

    Sends a request to read the contents of the data pointed by \a request. Returns a new QModbusReply object,
    which emits the finished() signal whenever data arrives. Modbus network may have multiple slaves,
    each slave has unique \a slaveId.
 */

QT_END_NAMESPACE
