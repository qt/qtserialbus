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

void RequestThread::write()
{
    modbus_set_slave(context, slaveId);

    switch (table) {
    case QModBusDevice::Coils:
        writeBits();
        break;
    case QModBusDevice::HoldingRegisters:
        writeBytes();
        break;
    default:
        break;//internal error
    }
}

void RequestThread::writeBits()
{
    quint8 bits[values.size()];
    for (int i = 0; i < values.size(); i++) {
        if (values.at(i))
            bits[i] = true;
        else
            bits[i] = false;
    }
    modbus_write_bits(context, startAddress, values.size(), bits);

    emit writeReady();
}

void RequestThread::writeBytes()
{
    quint16 bytes[values.size()];
    for (int i = 0; i < values.size(); i++)
        bytes[i] = values.at(i);
    modbus_write_registers(context, startAddress, values.size(), bytes);

    emit writeReady();
}

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
        //internal error
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
    QByteArray bitsRead;
    for (int i = 0; i < size; i++)
        bitsRead[i] = bits[i];
    emit readReady(bitsRead);
}

void RequestThread::readBytes()
{
    quint16 bytes[size];
    if (table == QModBusDevice::InputRegisters)
        modbus_read_input_registers(context, startAddress, size, bytes);
    else //Holding Register
        modbus_read_registers(context, startAddress, size, bytes);

    QByteArray bytesRead;
    QDataStream packer(&bytesRead, QIODevice::WriteOnly);
    for (int i = 0; i < size; i++)
        packer << bytes[i];

    emit readReady(bytesRead);
}

void Reply::read(const QList<QModBusDataUnit> &requests, int slaveId, modbus_t *context)
{
    request = new RequestThread();
    request->table = requests.first().tableType();
    this->table = requests.first().tableType();
    request->startAddress = requests.first().address();
    this->startAddress = requests.first().address();
    request->size = requests.size();
    request->slaveId = slaveId;
    request->context = context;
    request->moveToThread(&thread);
    connect(this, &Reply::startRead, request.data(), &RequestThread::read);
    connect(&thread, &QThread::finished, request.data(), &QObject::deleteLater);
    connect(request.data(), &RequestThread::readReady, this, &Reply::setResults);
    thread.start();
    emit startRead();
}

void Reply::write(const QList<QModBusDataUnit> &requests, int slaveId, modbus_t *context)
{
    request = new RequestThread();
    for (int i = 0; i < requests.size(); i++)
        values.append(requests.at(i).value());
    request->values = values;
    table = requests.first().tableType();
    request->table = requests.first().tableType();
    startAddress = requests.first().address();
    request->startAddress = requests.first().address();
    request->slaveId = slaveId;
    request->context = context;
    request->moveToThread(&thread);
    connect(this, &Reply::startWrite, request.data(), &RequestThread::write);
    connect(&thread, &QThread::finished, request.data(), &QObject::deleteLater);
    connect(request.data(), &RequestThread::writeReady, this, &Reply::setFinished);
    thread.start();
    emit startWrite();
}

void Reply::setFinished()
{
    for (int i = 0; i < values.size(); i++) {
        QModBusDataUnit unit(table, startAddress + i, values.at(i));
        payload.append(unit);
    }
    finish = true;
    emit finished();
}

void Reply::setError(QModBusReply::RequestException exceptionCode, const QString &errorString)
{
    Q_UNUSED(exceptionCode);
    Q_UNUSED(errorString);
}

void Reply::setResults(QByteArray load)
{
    if (table == QModBusDevice::InputRegisters || table == QModBusDevice::HoldingRegisters) {
        for (int i = 0; i < load.size(); i = i + 2) {
            quint16 result = (load.at(i) << 8) + load.at(i + 1);
            values.append(result);
        }
    } else { //Coils and Discrete Inputs
        for (int i = 0; i < load.size(); i++)
            values.append(load.at(i));
    }
    setFinished();
}

QT_END_NAMESPACE
