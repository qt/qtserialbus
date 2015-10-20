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

#include <QtCore/qdebug.h>
#include <QtSerialBus/qmodbuspdu.h>

#include <QtTest/QtTest>

static QString s_msg;
static void myMessageHandler(QtMsgType, const QMessageLogContext &, const QString &msg)
{
    s_msg = msg;
}

class DebugHandler
{
public:
    DebugHandler(QtMessageHandler newMessageHandler)
        : oldMessageHandler(qInstallMessageHandler(newMessageHandler)) {}
    ~DebugHandler() {
        qInstallMessageHandler(oldMessageHandler);
    }
private:
    QtMessageHandler oldMessageHandler;
};

static quint8 minimumDataSize(QModbusPdu::FunctionCode code, bool request)
{
    switch (code) {
    case QModbusPdu::ReadCoils:
    case QModbusPdu::ReadDiscreteInputs:
        return request ? 4u : 2u;
    case QModbusPdu::WriteSingleCoil:
    case QModbusPdu::WriteSingleRegister:
        return 4u;
    case QModbusPdu::ReadHoldingRegisters:
    case QModbusPdu::ReadInputRegisters:
        return request ? 4u : 3u;
    case QModbusPdu::ReadExceptionStatus:
        return request ? 0u : 1u;
    case QModbusPdu::Diagnostics:
        return 4u;
    case QModbusPdu::GetCommEventCounter:
        return request ? 0u : 4u;
    case QModbusPdu::GetCommEventLog:
        return request ? 0u : 8u;
    case QModbusPdu::WriteMultipleCoils:
        return request ? 6u : 4u;
    case QModbusPdu::WriteMultipleRegisters:
        return request ? 7u : 4u;
    case QModbusPdu::ReportServerId:
        return request ? 0u : 4u;   // TODO: The spec is not really clear here.
    case QModbusPdu::ReadFileRecord:
        return request ? 8u : 5u;
    case QModbusPdu::WriteFileRecord:
        return 10u;
    case QModbusPdu::MaskWriteRegister:
        return 6u;
    case QModbusPdu::ReadWriteMultipleRegisters:
        return request ? 11u : 3u;
    case QModbusPdu::ReadFifoQueue:
        return request ? 2u : 6u;
    case QModbusPdu::EncapsulatedInterfaceTransport:
        break; // TODO: The spec is not really clear here.
    }
    return 0u;
}

class tst_QModbusPdu : public QObject
{
    Q_OBJECT

private slots:
    void testDefaultConstructor()
    {
        QModbusRequest request;
        QCOMPARE(request.isValid(), false);
        QCOMPARE(request.isException(), false);
        QCOMPARE(request.functionCode(), QModbusRequest::FunctionCode(0x00));
        QCOMPARE(request.data().toHex(), QByteArray());

        QModbusResponse response;
        QCOMPARE(response.isValid(), false);
        QCOMPARE(response.isException(), false);
        QCOMPARE(response.functionCode(), QModbusResponse::FunctionCode(0x00));
        QCOMPARE(response.data().toHex(), QByteArray());

        QModbusExceptionResponse exception;
        QCOMPARE(exception.isValid(), false);
        QCOMPARE(exception.isException(), false);
        QCOMPARE(exception.functionCode(), QModbusExceptionResponse::FunctionCode(0x00));
        QCOMPARE(exception.data().toHex(), QByteArray());
    }

    void testVaridicConstructor()
    {
        QModbusRequest request(QModbusRequest::ReportServerId);
        QCOMPARE(request.isValid(), true);
        QCOMPARE(request.isException(), false);
        QCOMPARE(request.functionCode(), QModbusRequest::ReportServerId);
        QCOMPARE(request.data().toHex(), QByteArray());

        // Byte Count, Server ID, Run Indicator Status
        QModbusResponse response(QModbusResponse::ReportServerId, quint8(0x02), quint8(0x01),
            quint8(0xff));
        QCOMPARE(response.isValid(), true);
        QCOMPARE(response.isException(), false);
        QCOMPARE(response.functionCode(), QModbusResponse::ReportServerId);
        QCOMPARE(response.data().toHex(), QByteArray("0201ff"));
    }

    void testQByteArrayConstructor()
    {
        QModbusRequest request(QModbusRequest::ReportServerId, QByteArray());
        QCOMPARE(request.isValid(), true);
        QCOMPARE(request.isException(), false);
        QCOMPARE(request.functionCode(), QModbusRequest::ReportServerId);
        QCOMPARE(request.data().toHex(), QByteArray());

        // Byte Count, Server ID, Run Indicator Status
        QModbusResponse response(QModbusResponse::ReportServerId, QByteArray::fromHex("ff0102"));
        QCOMPARE(response.isValid(), true);
        QCOMPARE(response.isException(), false);
        QCOMPARE(response.functionCode(), QModbusResponse::ReportServerId);
        QCOMPARE(response.data().toHex(), QByteArray("ff0102"));
    }

    void testCopyConstructorAndAssignment()
    {
        QModbusPdu pdu;
        pdu.setFunctionCode(QModbusPdu::ReadCoils);

        QModbusRequest request(pdu);
        QCOMPARE(request.functionCode(), QModbusResponse::ReadCoils);

        QModbusResponse response(pdu);
        QCOMPARE(response.functionCode(), QModbusResponse::ReadCoils);

        pdu.setFunctionCode(QModbusPdu::WriteMultipleCoils);

        request = pdu;
        QCOMPARE(request.functionCode(), QModbusResponse::WriteMultipleCoils);

        response = pdu;
        QCOMPARE(response.functionCode(), QModbusResponse::WriteMultipleCoils);
    }

    void testEncodeDecode()
    {
        QModbusRequest invalid;
        quint8 data = 0xff;
        invalid.decodeData(&data);
        QCOMPARE(data, quint8(0xff));
        invalid.encodeData(data);
        QCOMPARE(invalid.data().toHex(), QByteArray("ff"));

        data = 0x00;
        QModbusRequest request(QModbusRequest::ReportServerId);
        request.decodeData(&data);
        QCOMPARE(data, quint8(0x00));
        QCOMPARE(request.data().toHex(), QByteArray());

        QModbusResponse response(QModbusResponse::ReportServerId);
        response.encodeData(quint8(0x02), quint8(0x01), quint8(0xff));
        quint8 count = 0, id = 0, run = 0;
        response.decodeData(&count, &id, &run);
        QCOMPARE(count, quint8(0x02));
        QCOMPARE(id, quint8(0x01));
        QCOMPARE(run, quint8(0xff));
        QCOMPARE(response.data().toHex(), QByteArray("0201ff"));

        count = 0, id = 0, run = 0;
        response = QModbusResponse(QModbusResponse::ReportServerId, QByteArray::fromHex("0201ff"));
        QCOMPARE(response.isValid(), true);
        QCOMPARE(response.isException(), false);
        QCOMPARE(response.functionCode(), QModbusResponse::ReportServerId);
        QCOMPARE(response.data().toHex(), QByteArray("0201ff"));
        response.decodeData(&count, &id, &run);
        QCOMPARE(count, quint8(0x02));
        QCOMPARE(id, quint8(0x01));
        QCOMPARE(run, quint8(0xff));

        QModbusRequest readCoils(QModbusRequest::ReadCoils);
        // starting address and quantity of coils
        readCoils.encodeData(quint16(12), quint16(10));
        quint16 address, quantity;
        readCoils.decodeData(&address, &quantity);
        QCOMPARE(address, quint16(0x0c));
        QCOMPARE(quantity, quint16(0x0a));

        request = QModbusRequest(QModbusRequest::WriteMultipleCoils,
            QByteArray::fromHex("0013000a02cd01"));
        QCOMPARE(request.data().toHex(), QByteArray("0013000a02cd01"));
        address = 0xffff, quantity = 0xffff;
        quint8 bytes = 0xff, firstByte = 0xff, secondByte = 0xff;
        request.decodeData(&address, &quantity, &bytes, &firstByte, &secondByte);
        QCOMPARE(address, quint16(19));
        QCOMPARE(quantity, quint16(10));
        QCOMPARE(bytes, quint8(2));
        QCOMPARE(firstByte, quint8(0xcd));
        QCOMPARE(secondByte, quint8(0x01));
    }

    void testQModbusExceptionResponsePdu()
    {
        QModbusExceptionResponse exception(QModbusExceptionResponse::ReportServerId,
            QModbusExceptionResponse::ServerDeviceFailure);
        QCOMPARE(exception.isValid(), true);
        QCOMPARE(exception.isException(), true);
        QCOMPARE(exception.functionCode(), QModbusExceptionResponse::FunctionCode(0x91));
        QCOMPARE(exception.data().toHex(), QByteArray("04"));

        QModbusExceptionResponse response;
        QCOMPARE(response.isValid(), false);
        QCOMPARE(response.isException(), false);
        QCOMPARE(response.functionCode(), QModbusExceptionResponse::Invalid);

        response.setFunctionCode(QModbusExceptionResponse::ReportServerId);
        response.setExeceptionCode(QModbusExceptionResponse::ServerDeviceFailure);
        QCOMPARE(response.isValid(), true);
        QCOMPARE(response.isException(), true);
        QCOMPARE(response.functionCode(), QModbusExceptionResponse::FunctionCode(0x91));
        QCOMPARE(response.data().toHex(), QByteArray("04"));

        QModbusPdu pdu;
        pdu.setFunctionCode(QModbusExceptionResponse::FunctionCode(QModbusPdu::ReadCoils | quint8(0x80)));
        QCOMPARE(pdu.isException(), true);
        QCOMPARE(pdu.functionCode(), QModbusExceptionResponse::FunctionCode(0x81));

        QModbusExceptionResponse exception2(pdu);
        QCOMPARE(exception2.isException(), true);
        QCOMPARE(exception2.functionCode(), QModbusExceptionResponse::FunctionCode(0x81));

        QModbusExceptionResponse exception3 = pdu;
        QCOMPARE(exception3.isException(), true);
        QCOMPARE(exception3.functionCode(), QModbusExceptionResponse::FunctionCode(0x81));
    }

    void testQDebugStreamOperator()
    {
        DebugHandler mhs(myMessageHandler);
        {
            QDebug d = qDebug();
            d << QModbusPdu();
            d << QModbusRequest();
            d << QModbusResponse();
            d << QModbusExceptionResponse();
        }
        QCOMPARE(s_msg, QString::fromLatin1("0x00 0x00 0x00 0x00"));

        {
            QDebug d = qDebug();
            d << QModbusRequest(QModbusRequest::ReadCoils, quint16(19), quint16(19));
        }
        QCOMPARE(s_msg, QString::fromLatin1("0x0100130013"));

        {
            QDebug d = qDebug();
            d << QModbusResponse(QModbusResponse::ReadCoils, QByteArray::fromHex("03cd6b05"));
        }
        QCOMPARE(s_msg, QString::fromLatin1("0x0103cd6b05"));

        {
            QDebug d = qDebug();
            d << QModbusExceptionResponse(QModbusExceptionResponse::ReadCoils,
                QModbusExceptionResponse::IllegalDataAddress);
        }
        QCOMPARE(s_msg, QString::fromLatin1("0x8102"));
    }

    void testMinimumDataSize()
    {
        bool request = true;
        QCOMPARE(minimumDataSize(QModbusPdu::ReadCoils, request),
            QModbusRequest::minimumDataSize(QModbusRequest::ReadCoils));
        QCOMPARE(minimumDataSize(QModbusPdu::ReadDiscreteInputs, request),
            QModbusRequest::minimumDataSize(QModbusRequest::ReadDiscreteInputs));
        QCOMPARE(minimumDataSize(QModbusPdu::WriteSingleCoil, request),
            QModbusRequest::minimumDataSize(QModbusRequest::WriteSingleCoil));
        QCOMPARE(minimumDataSize(QModbusPdu::ReadHoldingRegisters, request),
            QModbusRequest::minimumDataSize(QModbusRequest::ReadHoldingRegisters));
        QCOMPARE(minimumDataSize(QModbusPdu::ReadInputRegisters, request),
            QModbusRequest::minimumDataSize(QModbusRequest::ReadInputRegisters));
        QCOMPARE(minimumDataSize(QModbusPdu::WriteSingleRegister, request),
            QModbusRequest::minimumDataSize(QModbusRequest::WriteSingleRegister));
        QCOMPARE(minimumDataSize(QModbusPdu::ReadExceptionStatus, request),
            QModbusRequest::minimumDataSize(QModbusRequest::ReadExceptionStatus));
        QCOMPARE(minimumDataSize(QModbusPdu::Diagnostics, request),
            QModbusRequest::minimumDataSize(QModbusRequest::Diagnostics));
        QCOMPARE(minimumDataSize(QModbusPdu::GetCommEventCounter, request),
            QModbusRequest::minimumDataSize(QModbusRequest::GetCommEventCounter));
        QCOMPARE(minimumDataSize(QModbusPdu::GetCommEventLog, request),
            QModbusRequest::minimumDataSize(QModbusRequest::GetCommEventLog));
        QCOMPARE(minimumDataSize(QModbusPdu::WriteMultipleCoils, request),
            QModbusRequest::minimumDataSize(QModbusRequest::WriteMultipleCoils));
        QCOMPARE(minimumDataSize(QModbusPdu::WriteMultipleRegisters, request),
            QModbusRequest::minimumDataSize(QModbusRequest::WriteMultipleRegisters));
        QCOMPARE(minimumDataSize(QModbusPdu::ReportServerId, request),
            QModbusRequest::minimumDataSize(QModbusRequest::ReportServerId));
        QCOMPARE(minimumDataSize(QModbusPdu::ReadFileRecord, request),
            QModbusRequest::minimumDataSize(QModbusRequest::ReadFileRecord));
        QCOMPARE(minimumDataSize(QModbusPdu::WriteFileRecord, request),
            QModbusRequest::minimumDataSize(QModbusRequest::WriteFileRecord));
        QCOMPARE(minimumDataSize(QModbusPdu::MaskWriteRegister, request),
            QModbusRequest::minimumDataSize(QModbusRequest::MaskWriteRegister));
        QCOMPARE(minimumDataSize(QModbusPdu::ReadWriteMultipleRegisters, request),
            QModbusRequest::minimumDataSize(QModbusRequest::ReadWriteMultipleRegisters));
        QCOMPARE(minimumDataSize(QModbusPdu::ReadFifoQueue, request),
            QModbusRequest::minimumDataSize(QModbusRequest::ReadFifoQueue));
        QCOMPARE(minimumDataSize(QModbusPdu::EncapsulatedInterfaceTransport, request),
            QModbusRequest::minimumDataSize(QModbusRequest::EncapsulatedInterfaceTransport));

        request = false;
        QCOMPARE(minimumDataSize(QModbusPdu::ReadCoils, request),
            QModbusResponse::minimumDataSize(QModbusResponse::ReadCoils));
        QCOMPARE(minimumDataSize(QModbusPdu::ReadDiscreteInputs, request),
            QModbusResponse::minimumDataSize(QModbusResponse::ReadDiscreteInputs));
        QCOMPARE(minimumDataSize(QModbusPdu::WriteSingleCoil, request),
            QModbusResponse::minimumDataSize(QModbusResponse::WriteSingleCoil));
        QCOMPARE(minimumDataSize(QModbusPdu::ReadHoldingRegisters, request),
            QModbusResponse::minimumDataSize(QModbusResponse::ReadHoldingRegisters));
        QCOMPARE(minimumDataSize(QModbusPdu::ReadInputRegisters, request),
            QModbusResponse::minimumDataSize(QModbusResponse::ReadInputRegisters));
        QCOMPARE(minimumDataSize(QModbusPdu::WriteSingleRegister, request),
            QModbusResponse::minimumDataSize(QModbusResponse::WriteSingleRegister));
        QCOMPARE(minimumDataSize(QModbusPdu::ReadExceptionStatus, request),
            QModbusResponse::minimumDataSize(QModbusResponse::ReadExceptionStatus));
        QCOMPARE(minimumDataSize(QModbusPdu::Diagnostics, request),
            QModbusResponse::minimumDataSize(QModbusResponse::Diagnostics));
        QCOMPARE(minimumDataSize(QModbusPdu::GetCommEventCounter, request),
            QModbusResponse::minimumDataSize(QModbusResponse::GetCommEventCounter));
        QCOMPARE(minimumDataSize(QModbusPdu::GetCommEventLog, request),
            QModbusResponse::minimumDataSize(QModbusResponse::GetCommEventLog));
        QCOMPARE(minimumDataSize(QModbusPdu::WriteMultipleCoils, request),
            QModbusResponse::minimumDataSize(QModbusResponse::WriteMultipleCoils));
        QCOMPARE(minimumDataSize(QModbusPdu::WriteMultipleRegisters, request),
            QModbusResponse::minimumDataSize(QModbusResponse::WriteMultipleRegisters));
        QCOMPARE(minimumDataSize(QModbusPdu::ReportServerId, request),
            QModbusResponse::minimumDataSize(QModbusResponse::ReportServerId));
        QCOMPARE(minimumDataSize(QModbusPdu::ReadFileRecord, request),
            QModbusResponse::minimumDataSize(QModbusResponse::ReadFileRecord));
        QCOMPARE(minimumDataSize(QModbusPdu::WriteFileRecord, request),
            QModbusResponse::minimumDataSize(QModbusResponse::WriteFileRecord));
        QCOMPARE(minimumDataSize(QModbusPdu::MaskWriteRegister, request),
            QModbusResponse::minimumDataSize(QModbusResponse::MaskWriteRegister));
        QCOMPARE(minimumDataSize(QModbusPdu::ReadWriteMultipleRegisters, request),
            QModbusResponse::minimumDataSize(QModbusResponse::ReadWriteMultipleRegisters));
        QCOMPARE(minimumDataSize(QModbusPdu::ReadFifoQueue, request),
            QModbusResponse::minimumDataSize(QModbusResponse::ReadFifoQueue));
        QCOMPARE(minimumDataSize(QModbusPdu::EncapsulatedInterfaceTransport, request),
            QModbusResponse::minimumDataSize(QModbusResponse::EncapsulatedInterfaceTransport));
    }
};

QTEST_MAIN(tst_QModbusPdu)

#include "tst_qmodbuspdu.moc"
