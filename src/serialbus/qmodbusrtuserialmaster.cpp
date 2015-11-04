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

#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_MODBUS)
Q_DECLARE_LOGGING_CATEGORY(QT_MODBUS_LOW)

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
    d->setupSerialPort();
}

/*!
    \internal
*/
QModbusRtuSerialMaster::~QModbusRtuSerialMaster()
{
    close();
}

/*!
    \internal
*/
QModbusRtuSerialMaster::QModbusRtuSerialMaster(QModbusRtuSerialMasterPrivate &dd, QObject *parent)
    : QModbusClient(dd, parent)
{
    Q_D(QModbusRtuSerialMaster);
    d->setupSerialPort();
}

/*!
     \reimp
 */
bool QModbusRtuSerialMaster::open()
{
    if (state() == QModbusDevice::ConnectedState)
        return true;

    Q_D(QModbusRtuSerialMaster);

    d->responseBuffer.clear();

    d->m_serialPort->setPortName(portName());
    if (d->m_serialPort->open(QIODevice::ReadWrite))
        setState(QModbusDevice::ConnectedState);

    return (state() == QModbusDevice::ConnectedState);
}

/*!
     \reimp
 */
void QModbusRtuSerialMaster::close()
{
    Q_D(QModbusRtuSerialMaster);

    if (d->m_serialPort->isOpen())
        d->m_serialPort->close();

    qCDebug(QT_MODBUS_LOW) << "Aborted replies:" << d->m_queue.count();

    while (!d->m_queue.isEmpty()) {
        // Finish each open reply and forget them
        QModbusRtuSerialMasterPrivate::QueueElement elem = d->m_queue.dequeue();
        if (!elem.reply.isNull()) {
            elem.reply->setError(QModbusReply::ReplyAbortedError,
                                 tr("Reply aborted due to connection closure."));
            elem.reply->setFinished(true);
        }
    }

    setState(QModbusDevice::UnconnectedState);
}


QModbusReply *QModbusRtuSerialMaster::sendReadRequest(const QModbusDataUnit &read, int slaveAddress)
{
    Q_D(QModbusRtuSerialMaster);

    //TODO implement timeout behavior

    if (!d->m_serialPort->isOpen() || state() != QModbusDevice::ConnectedState) {
        setError(tr("Device not connected."), QModbusDevice::ConnectionError);
        qCWarning(QT_MODBUS) << "RTU master is not connected";
        return Q_NULLPTR;
    }

    QModbusRequest request = d->createReadRequest(read);
    if (!request.isValid())
        return Q_NULLPTR;

    return d->enqueueRequest(request, slaveAddress, read);
}

QModbusReply *QModbusRtuSerialMaster::sendWriteRequest(const QModbusDataUnit &write, int slaveAddress)
{
    Q_D(QModbusRtuSerialMaster);

    //TODO implement timeout behavior

    if (!d->m_serialPort->isOpen() || state() != QModbusDevice::ConnectedState) {
        setError(tr("Device not connected."), QModbusDevice::ConnectionError);
        qCWarning(QT_MODBUS) << "RTU master is not connected";
        return Q_NULLPTR;
    }

    QModbusRequest request = d->createWriteRequest(write);
    if (!request.isValid())
        return Q_NULLPTR;

    return d->enqueueRequest(request, slaveAddress, write);
}

QModbusReply *QModbusRtuSerialMaster::sendReadWriteRequest(const QModbusDataUnit &read,
                                                           const QModbusDataUnit &write,
                                                           int slaveAddress)
{
    Q_D(QModbusRtuSerialMaster);

    //TODO implement timeout behavior

    if (!d->m_serialPort->isOpen() || state() != QModbusDevice::ConnectedState) {
        setError(tr("Device not connected."), QModbusDevice::ConnectionError);
        qCWarning(QT_MODBUS) << "RTU master is not connected";
        return Q_NULLPTR;
    }

    QModbusRequest request = d->createRWRequest(read, write);
    if (!request.isValid())
        return Q_NULLPTR;

    return d->enqueueRequest(request, slaveAddress, read); // only need to remember read
}

QT_END_NAMESPACE
