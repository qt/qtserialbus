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

#include "qmodbusclient.h"
#include "qmodbusclient_p.h"

#include <bitset>

QT_BEGIN_NAMESPACE

/*!
    \class QModbusClient
    \inmodule QtSerialBus
    \since 5.6

    \brief The QModbusClient class is the interface to send Modbus requests.

    The QModbusClient API is constructed around one QModbusClient object, which holds the common
    configuration and settings for the requests it sends. One QModbusClient should be enough for
    the whole Qt application.

    Once a QModbusClient object has been created, the application can use it to send requests.
    The returned object is used to obtain any data returned in response to the corresponding request.

    QModbusClient has an asynchronous API. When the finished slot is called, the parameter
    it takes is the QModbusReplyEx object containing the PDU as well as meta-data (Addressing, etc.).

    Note: After the request has finished, it is the responsibility of the user to delete the
    QModbusReplyEx object at an appropriate time. Do not directly delete it inside the slot connected
    to requestFinished(). You can use the deleteLater() function.

    Note: QModbusClient queues the requests it receives. The number of requests executed in
    parallel is dependent on the protocol. For example, the HTTP protocol on desktop platforms
    issues 6 requests in parallel for one host/port combination.
*/

/*!
    Constructs a Modbus client device with the specified \a parent.
 */
QModbusClient::QModbusClient(QObject *parent)
    : QModbusDevice(*new QModbusClientPrivate, parent)
{
}

/*!
    \internal
*/
QModbusClient::~QModbusClient()
{
}

/*!
    \internal
*/
QModbusClient::QModbusClient(QModbusClientPrivate &dd, QObject *parent) :
    QModbusDevice(dd, parent)
{

}

/*!
    \fn QModbusReply *QModbusClient::write(const QModbusDataUnit &request, int slaveId)

    Sends a request to modify the contents of the data pointed by \a request. Returns a
    new QModbusReply object, which emits the finished() signal whenever a positive response
    for the write request is received. Modbus network may have multiple servers, each server has
    a unique \a slaveId.
 */

/*!
    \fn QModbusReply *QModbusClient::read(const QModbusDataUnit &request, int slaveId)

    Sends a request to read the contents of the data pointed by \a request. Returns a new
    QModbusReply object, which emits the finished() signal whenever data arrives. Modbus network
    may have multiple servers, each server has unique \a slaveId.
 */

/*
    Processes a Modbus server \a response and stores the decoded information in \a data. Returns
    true on success; otherwise false.
*/
bool QModbusClient::processResponse(const QModbusResponse &response, QModbusDataUnit *data)
{
    return d_func()->processResponse(response, data);
}

/*
    To be implemented by custom Modbus client implementation. The default implementation ignores
    \a response and \a data. It always returns false to indicate error.
*/
bool QModbusClient::processPrivateModbusResponse(const QModbusResponse &response, QModbusDataUnit *data)
{
    Q_UNUSED(response)
    Q_UNUSED(data)
    return false;
}

/*
    TODO: implement
*/
bool QModbusClientPrivate::processResponse(const QModbusResponse &response, QModbusDataUnit *data)
{
    switch (response.functionCode()) {
    case QModbusRequest::ReadCoils:
        return processReadCoilsResponse(response, data);
    case QModbusRequest::ReadDiscreteInputs:
    case QModbusRequest::ReadHoldingRegisters:
    case QModbusRequest::ReadInputRegisters:
    case QModbusRequest::WriteSingleCoil:
        return processWriteSingleCoilResponse(response, data);
    case QModbusRequest::WriteSingleRegister:
    case QModbusRequest::ReadExceptionStatus:
    case QModbusRequest::Diagnostics:
    case QModbusRequest::GetCommEventCounter:
    case QModbusRequest::GetCommEventLog:
    case QModbusRequest::WriteMultipleCoils:
        return processWriteMultipleCoilsResponse(response, data);
    case QModbusRequest::WriteMultipleRegisters:
    case QModbusRequest::ReportServerId:
    case QModbusRequest::ReadFileRecord:
    case QModbusRequest::WriteFileRecord:
    case QModbusRequest::MaskWriteRegister:
    case QModbusRequest::ReadWriteMultipleRegisters:
    case QModbusRequest::ReadFifoQueue:
    case QModbusRequest::EncapsulatedInterfaceTransport:
    default:
        break;
    }
    return q_func()->processPrivateModbusResponse(response, data);
}

static bool isValid(const QModbusResponse &response, QModbusResponse::FunctionCode fc)
{
    if (!response.isValid())
        return false;
    if (response.isException())
        return false;
    if (response.functionCode() != fc)
        return false;
    return true;
}

bool QModbusClientPrivate::processReadCoilsResponse(const QModbusResponse &response,
    QModbusDataUnit *data)
{
    if (!isValid(response, QModbusResponse::ReadCoils))
        return false;

    // we expect at least the byte count
    const QByteArray payload = response.data();
    if (payload.size() < 1)
        return false;

    // byte count needs to match available bytes
    const quint8 byteCount = payload[0];
    if ((payload.size() - 1) != byteCount)
        return false;

    qint32 coil = 0;
    QVector<quint16> values(byteCount * 8);
    for (qint32 i = 1; i < payload.size(); ++i) {
        const std::bitset<8> byte = payload[i];
        for (qint32 currentBit = 0; currentBit < 8; ++currentBit)
            values[coil++] = byte[currentBit];
    }

    if (data) {
        data->setValues(values);
        data->setStartAddress(0);
        data->setValueCount(byteCount * 8);
        data->setRegisterType(QModbusRegister::Coils);
    }
    return true;
}

bool QModbusClientPrivate::processWriteSingleCoilResponse(const QModbusResponse &response,
    QModbusDataUnit *data)
{
    if (!isValid(response, QModbusResponse::WriteSingleCoil))
        return false;

    const QByteArray payload = response.data();
    if (payload.size() < 4)
        return false;

    quint16 address, value;
    response.decodeData(&address, &value);
    if (data) {
        data->setValueCount(1);
        data->setStartAddress(address);
        data->setValues(QVector<quint16>{ value });
        data->setRegisterType(QModbusRegister::Coils);
    }
    return true;
}

bool QModbusClientPrivate::processWriteMultipleCoilsResponse(const QModbusResponse &response,
    QModbusDataUnit *data)
{
    if (!isValid(response, QModbusResponse::WriteMultipleCoils))
        return false;

    const QByteArray payload = response.data();
    if (payload.size() < 4)
        return false;

    quint16 address, count;
    response.decodeData(&address, &count);
    if (data) {
        data->setValueCount(count);
        data->setStartAddress(address);
        data->setRegisterType(QModbusRegister::Coils);
    }
    return true;
}

QT_END_NAMESPACE
