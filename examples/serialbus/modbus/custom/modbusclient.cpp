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

#include "modbusclient.h"

// The read response must contain the start address (quint16), the byte count
// (quint8) and at least one register value (quint16).
static constexpr int ReadHeaderSize {3};
static constexpr int MinimumReadResponseSize {ReadHeaderSize + 2}; // 2 payload bytes

// The response must contain the start address (quint16) and the number of
// registers written (quint16).
static constexpr int WriteResponseSize {4};

ModbusClient::ModbusClient(QObject *parent)
    : QModbusTcpClient(parent)
{
    QModbusResponse::registerDataSizeCalculator(CustomRead, [](const QModbusResponse &response) {
        if (!response.isValid())
            return -1;

        if (response.dataSize() < MinimumReadResponseSize)
            return -1;
        return ReadHeaderSize + quint8(response.data().at(ReadHeaderSize - 1));
    });

    QModbusResponse::registerDataSizeCalculator(CustomWrite, [](const QModbusResponse &response) {
        if (!response.isValid())
            return -1;
        return (response.dataSize() != WriteResponseSize ? -1 : WriteResponseSize);
    });
}

static bool collateBytes(const QModbusPdu &response, QModbusDataUnit *data)
{
    if (response.dataSize() < MinimumReadResponseSize)
        return false;

    quint16 address; quint8 byteCount;
    response.decodeData(&address, &byteCount);

    if (byteCount % 2 != 0)
        return false;

    if (data) {
        QDataStream stream(response.data().remove(0, 3));

        QList<quint16> values;
        const quint8 itemCount = byteCount / 2;
        for (int i = 0; i < itemCount; i++) {
            quint16 tmp;
            stream >> tmp;
            values.append(tmp);
        }
        *data = {QModbusDataUnit::HoldingRegisters, address, values};
    }
    return true;
}

static bool collateMultipleValues(const QModbusPdu &response, QModbusDataUnit *data)
{
    if (response.dataSize() != WriteResponseSize)
        return false;

    quint16 address, count;
    response.decodeData(&address, &count);

    if (count < 1 || count > 10)
        return false;

    if (data)
        *data = {QModbusDataUnit::HoldingRegisters, address, count};
    return true;
}

bool ModbusClient::processPrivateResponse(const QModbusResponse &response, QModbusDataUnit *data)
{
    if (!response.isValid())
        return QModbusClient::processPrivateResponse(response, data);

    if (CustomRead == response.functionCode())
        return collateBytes(response, data);

    if (CustomWrite == response.functionCode())
        return collateMultipleValues(response, data);
    return QModbusClient::processPrivateResponse(response, data);
}
