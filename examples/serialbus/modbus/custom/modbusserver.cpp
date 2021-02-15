/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the QtSerialBus module.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "modbusserver.h"
#include "modbusclient.h"

// The request must contain the start address (quint16) and the number of
// registers to read (quint16).
static constexpr int ReadRequestSize {4};

// The request must contain the start address (quint16), the number of
// registers to write (quint16), the byte count (quint8) and at least one
// register value (quint16).
static constexpr int WriteHeaderSize {5};
static constexpr int MinimumWriteRequestSize {WriteHeaderSize + 2}; // 2 payload bytes

ModbusServer::ModbusServer(QObject *parent)
    : QModbusTcpServer(parent)
{
    setMap({{QModbusDataUnit::HoldingRegisters, {QModbusDataUnit::HoldingRegisters, 0, 10}}});

    QModbusRequest::registerDataSizeCalculator(ModbusClient::CustomRead, [](const QModbusRequest &request) {
        if (!request.isValid())
            return -1;
        return (request.dataSize() != ReadRequestSize ? -1 : ReadRequestSize);
    });

    QModbusRequest::registerDataSizeCalculator(ModbusClient::CustomWrite, [](const QModbusRequest &request) {
        if (!request.isValid())
            return -1;

        if (request.dataSize() < MinimumWriteRequestSize)
            return -1;
        return WriteHeaderSize + quint8(request.data().at(WriteHeaderSize - 1));
    });
}

QModbusResponse ModbusServer::processPrivateRequest(const QModbusPdu &request)
{
    if (!request.isValid())
        return QModbusServer::processPrivateRequest(request);

    if (ModbusClient::CustomRead == request.functionCode()) {
        quint16 startAddress, count;
        request.decodeData(&startAddress, &count);

        QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, startAddress, count);
        if (!data(&unit)) {
            return QModbusExceptionResponse(request.functionCode(),
                QModbusExceptionResponse::IllegalDataAddress);
        }
        return QModbusResponse(request.functionCode(), startAddress, quint8(count * 2), unit.values());
    }

    if (ModbusClient::CustomWrite == request.functionCode()) {
        quint8 byteCount;
        quint16 startAddress, numberOfRegisters;
        request.decodeData(&startAddress, &numberOfRegisters, &byteCount);

        if (byteCount % 2 != 0) {
            return QModbusExceptionResponse(request.functionCode(),
                QModbusExceptionResponse::IllegalDataValue);
        }

        const QByteArray pduData = request.data().remove(0, WriteHeaderSize);
        QDataStream stream(pduData);

        QList<quint16> values;
        for (int i = 0; i < numberOfRegisters; i++) {
            quint16 tmp;
            stream >> tmp;
            values.append(tmp);
        }

        if (!writeData({QModbusDataUnit::HoldingRegisters, startAddress, values})) {
            return QModbusExceptionResponse(request.functionCode(),
                QModbusExceptionResponse::ServerDeviceFailure);
        }

        return QModbusResponse(request.functionCode(), startAddress, numberOfRegisters);
    }

    return QModbusServer::processPrivateRequest(request);
}
