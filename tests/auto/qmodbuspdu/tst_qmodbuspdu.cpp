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

#include <QtSerialBus/qmodbuspdu.h>

#include <QtTest/QtTest>

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
};

QTEST_MAIN(tst_QModbusPdu)

#include "tst_qmodbuspdu.moc"
