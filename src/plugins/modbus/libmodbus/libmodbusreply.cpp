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

#include "libmodbusreply.h"

#include <QtCore/qdebug.h>
#include <QtCore/qdatastream.h>

QT_BEGIN_NAMESPACE

void RequestThread::read()
{
    modbus_set_slave(context, slaveId);

    switch (table) {
    case QModBusDevice::DiscreteInputs:
    case QModBusDevice::Coils:
            readBits();
        break;
    case QModBusDevice::InputRegisters:
    case QModBusDevice::HoldingRegisters:
            readBytes();
        break;
    default:
        //error
        break;
    }
}

void RequestThread::readBits()
{
    quint8 bits[size];
    if (table == QModBusDevice::DiscreteInputs)
        modbus_read_input_bits(context, startAddress, size, bits);
    else
        modbus_read_bits(context, startAddress, size, bits);
    QByteArray readBits;
    for (int i = 0; i < size; i++)
        readBits[i] = bits[i];
    emit ready(readBits);
}

void RequestThread::readBytes()
{
    quint16 bytes[size];
    if (table == QModBusDevice::InputRegisters)
        modbus_read_input_registers(context, startAddress, size, bytes);
    else //Holding Register
        modbus_read_registers(context, startAddress, size, bytes);

    QByteArray readBytes;
    QDataStream packer(&readBytes, QIODevice::WriteOnly);
    for (int i = 0; i < size; i++)
        packer << bytes[i];

    emit ready(readBytes);
}

void Reply::read(QModBusDevice::ModBusTable table, quint16 startAddress,
                quint16 size, int slaveId, modbus_t *context)
{
    request = new RequestThread();
    request->table = table;
    this->table = table;
    request->startAddress = startAddress;
    this->startAddress = startAddress;
    request->size = size;
    request->slaveId = slaveId;
    request->context = context;
    request->moveToThread(&thread);
    connect(this, &Reply::startRead, request.data(), &RequestThread::read);
    connect(&thread, &QThread::finished, request.data(), &QObject::deleteLater);
    connect(request.data(), &RequestThread::ready, this, &Reply::setPayload);
    thread.start();
    emit startRead();
}

void Reply::setFinished(bool finished)
{
    Q_UNUSED(finished);
}

void Reply::setError(QModBusReply::RequestException exceptionCode, const QString &errorString)
{
    Q_UNUSED(exceptionCode);
    Q_UNUSED(errorString);
}

void Reply::setPayload(QByteArray load)
{
    if (table == QModBusDevice::InputRegisters || table == QModBusDevice::HoldingRegisters) {
        for (int i = 0; i < load.size(); i = i + 2) {
            QModBusDataUnit unit(table, startAddress + i);
            quint16 result = (load.at(i) << 8) + load.at(i + 1);
            unit.setValue(result);
            payload.append(unit);
        }
    } else { //Coils and Discrete Inputs
        for (int i = 0; i < load.size(); i++) {
            QModBusDataUnit unit(table, startAddress + i, load.at(i));
            payload.append(unit);
        }
    }
    finish = true;
    emit finished();
}

QT_END_NAMESPACE
