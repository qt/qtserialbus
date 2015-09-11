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

#if defined(Q_OS_UNIX)
# include <errno.h>
#endif

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
        //internal error
        break;
    }
}

void RequestThread::writeBits()
{
    QByteArray bits(values.size(), Qt::Uninitialized);
    for (int i = 0; i < values.size(); i++) {
        if (values.at(i))
            bits[i] = true;
        else
            bits[i] = false;
    }
    if (modbus_write_bits(context, startAddress, values.size(), (uint8_t*) bits.data()) == -1) {
        emit error(errno);
        return;
    }

    emit writeReady();
}

void RequestThread::writeBytes()
{
    QVector<quint16> bytes(values.size());
    for (int i = 0; i < values.size(); i++)
        bytes[i] = values.at(i);
    if (modbus_write_registers(context, startAddress, values.size(), bytes.data()) == -1) {
        emit error(errno);
        return;
    }

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
    QByteArray bits(size, 0);
    if (table == QModBusDevice::DiscreteInputs) {
        if (modbus_read_input_bits(context, startAddress, size, (uint8_t*) bits.data()) == -1) {
            emit error(errno);
            return;
        }
    } else {
        if (modbus_read_bits(context, startAddress, size, (uint8_t*) bits.data()) == -1) {
            emit error(errno);
            return;
        }
    }
    QByteArray bitsRead;
    for (int i = 0; i < size; i++)
        bitsRead[i] = bits[i];
    emit readReady(bitsRead);
}

void RequestThread::readBytes()
{
    QVector<quint16> bytes(size, 0);
    if (table == QModBusDevice::InputRegisters) {
        if (modbus_read_input_registers(context, startAddress, size, bytes.data()) == -1) {
            emit error(errno);
            return;
        }
    } else { //Holding Register
        if (modbus_read_registers(context, startAddress, size, bytes.data()) == -1) {
            emit error(errno);
            return;
        }
    }

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
    connect(request.data(), &RequestThread::error, this, &Reply::handleError);
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
    connect(request.data(), &RequestThread::error, this, &Reply::handleError);
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

void Reply::setError(QModBusReply::RequestError errorCode, const QString &errorString)
{
    payload.clear();
    errorType = errorCode;
    errorText = errorString;
    emit errorOccurred(errorCode);
    finish = true;
    emit finished();
}

void Reply::setResults(QByteArray load)
{
    if (table == QModBusDevice::InputRegisters || table == QModBusDevice::HoldingRegisters) {
        for (int i = 0; i < load.size(); i = i + 2) {
            quint16 result = (load.at(i) << 8) | load.at(i + 1);
            values.append(result);
        }
    } else { //Coils and Discrete Inputs
        for (int i = 0; i < load.size(); i++)
            values.append(load.at(i));
    }
    setFinished();
}

void Reply::handleError(int errorNumber)
{
    QString errorText;
    QModBusReply::RequestError error;
    // defined in libmodbus
    switch (errorNumber) {
    case EMBXILFUN:
        error = QModBusReply::IllegalFunction;
        errorText = modbus_strerror(errorNumber);
        break;
    case EMBXILADD:
        error = QModBusReply::IllegalDataAddress;
        errorText = modbus_strerror(errorNumber);
        break;
    case EMBXILVAL:
        error = QModBusReply::IllegalDataValue;
        errorText = modbus_strerror(errorNumber);
        break;
    case EMBXSFAIL:
        error = QModBusReply::SlaveFailure;
        errorText = modbus_strerror(errorNumber);
        break;
    case EMBXACK:
        error = QModBusReply::Acknowledge;
        errorText = modbus_strerror(errorNumber);
        break;
    case EMBXSBUSY:
        error = QModBusReply::SlaveBusy;
        errorText = modbus_strerror(errorNumber);
        break;
    case EMBXMEMPAR:
        error = QModBusReply::MemoryParity;
        errorText = modbus_strerror(errorNumber);
        break;
    case EMBXGPATH:
        error = QModBusReply::GatewayUnavailable;
        errorText = modbus_strerror(errorNumber);
        break;
    case EMBXGTAR:
        error = QModBusReply::NoResponse;
        errorText = modbus_strerror(errorNumber);
        break;
    case EMBBADCRC:
        error = QModBusReply::InvalidCRC;
        errorText = modbus_strerror(errorNumber);
        break;
    case EMBXNACK:
    case EMBBADDATA:
    case EMBBADEXC:
    case EMBUNKEXC:
    case EMBMDATA:
        error = QModBusReply::InvalidError;
        errorText = modbus_strerror(errorNumber);
        break;
    default:
        error = QModBusReply::InvalidError;
        errorText = qt_error_string(errorNumber);
        break;
    }
    setError(error, errorText);
}

QT_END_NAMESPACE
