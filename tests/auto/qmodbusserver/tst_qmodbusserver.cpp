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

#include <QtSerialBus/qmodbusserver.h>

#include <QtTest/QtTest>

class TestServer : public QModbusServer
{
public:
    TestServer() Q_DECL_EQ_DEFAULT;

    virtual bool open() Q_DECL_OVERRIDE { return true; }
    virtual void close() Q_DECL_OVERRIDE {}

    QModbusResponse processRequest(const QModbusPdu &request)
    {
        return QModbusServer::processRequest(request);
    }
};

#define MAP_RANGE 500

class tst_QModbusServer : public QObject
{
    Q_OBJECT

private:
    TestServer server;

private slots:
    void init()
    {
        QModbusDataUnitMap map;
        map.insert(QModbusDataUnit::DiscreteInputs, { QModbusDataUnit::DiscreteInputs, 0, MAP_RANGE });
        map.insert(QModbusDataUnit::Coils, { QModbusDataUnit::Coils, 0, MAP_RANGE });
        map.insert(QModbusDataUnit::InputRegisters, { QModbusDataUnit::InputRegisters, 0, MAP_RANGE });
        map.insert(QModbusDataUnit::HoldingRegisters, { QModbusDataUnit::HoldingRegisters, 0, MAP_RANGE });
        server.setMap(map);
    }

    void testProcessRequestReadWriteSingleMultipleCoils()
    {
        // request write Coil 173, address: 0x00ac -> 172, value: 0xff00 -> ON
        QModbusRequest request = QModbusRequest(QModbusRequest::WriteSingleCoil,
            QByteArray::fromHex("00acff00"));
        QModbusResponse response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("00acff00"));

        // request read Coil 173, address: 0x00ac -> 172, count: 0x0001 -> 1
        request = QModbusRequest(QModbusRequest::ReadCoils, QByteArray::fromHex("00ac0001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, byte count: 0x01 -> 1, status: 0x01 -> 0000 0001
        QCOMPARE(response.data(), QByteArray::fromHex("0101"));

        // request write 10 coils starting at coil 20, address: 0x0013 -> 19, count: 0x000a -> 10,
        // payload bytes: 0x02 -> 2, values: 0xcd -> 1100 1101, 0x01 -> 0000 0001
        request = QModbusRequest(QModbusRequest::WriteMultipleCoils,
            QByteArray::fromHex("0013000a02cd01"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request's first 4 bytes
        QCOMPARE(response.data(), QByteArray::fromHex("0013000a"));

        // request read 10 coils starting at coil 20, address: 0x0013 -> 19, count: 0x000a -> 10
        request = QModbusRequest(QModbusRequest::ReadCoils, QByteArray::fromHex("0013000a"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, byte count: 0x02 -> 1, status: 0xcd -> 1100 1101, 0x01 -> 0000 0001
        QCOMPARE(response.data(), QByteArray::fromHex("02cd01"));

        // request write 19 coils starting at coil 20, address: 0x0013 -> 19, count: 0x0013 -> 19,
        // payload bytes: 0x03 -> 3, values: 0xcd -> 1100 1101, 0x6b -> 0110 1011, 0x05 -> 0000 0101
        request = QModbusRequest(QModbusRequest::WriteMultipleCoils,
            QByteArray::fromHex("0013001303cd6b05"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request's first 4 bytes
        QCOMPARE(response.data(), QByteArray::fromHex("00130013"));

        // request read 19 coils starting at coil 20, address: 0x0013 -> 19, count: 0x0013 -> 19
        request = QModbusRequest(QModbusRequest::ReadCoils, QByteArray::fromHex("00130013"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, byte count: 0x03 -> 3
        // status: 0xcd -> 1100 1101, 0x6b -> 0110 1011, 0x05 -> 0000 0101
        QCOMPARE(response.data(), QByteArray::fromHex("03cd6b05"));

        // request write 10 coils, starting at coil 0, address: 0x0000 -> 0, count: 0x000a -> 10
        // payload bytes: 0x02 -> 2, values: 0xcd -> 1100 1101, 0x02 -> 0000 0010
        request = QModbusRequest(QModbusRequest::WriteMultipleCoils,
            QByteArray::fromHex("0000000a02cd02"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request's first 4 bytes
        QCOMPARE(response.data(), QByteArray::fromHex("0000000a"));

        // request read 10 coils starting at coil 0, address: 0x0000 -> 0, count: 0x000a -> 10
        request = QModbusRequest(QModbusRequest::ReadCoils, QByteArray::fromHex("0000000a"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, byte count: 0x02 -> 2, status: 0xcd -> 1100 1101, 0x02 -> 0000 0020
        QCOMPARE(response.data(), QByteArray::fromHex("02cd02"));
    }

    void testProcessReadDiscreteInputsRequest()
    {
        server.setData(QModbusDataUnit::DiscreteInputs, 172, true);
        // request read DiscreteInput 173, address: 0x00ac -> 172, count: 0x0001 -> 1
        QModbusRequest request = QModbusRequest(QModbusRequest::ReadDiscreteInputs,
                                                QByteArray::fromHex("00ac0001"));
        QModbusResponse response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, byte count: 0x01 -> 1, status: 0x01 -> 0000 0001
        QCOMPARE(response.data(), QByteArray::fromHex("0101"));

        server.setData(QModbusDataUnit::DiscreteInputs, 19, quint16(0x0010));
        // request read 10 inputs starting at input 20, address: 0x0013 -> 19, count: 0x000a -> 10
        request = QModbusRequest(QModbusRequest::ReadDiscreteInputs, QByteArray::fromHex("0013000a"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, byte count: 0x02 -> 1, status: 0x00 -> 0000 0000, 0x10 -> 0001 0000
        QCOMPARE(response.data(), QByteArray::fromHex("020100"));

        // request read 10 inputs starting at input 501
        request = QModbusRequest(QModbusRequest::ReadDiscreteInputs, QByteArray::fromHex("01f5000a"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        QCOMPARE(response.data(), QByteArray::fromHex("02"));

        // request read 2001 inputs starting at input 0
        request = QModbusRequest(QModbusRequest::ReadDiscreteInputs, QByteArray::fromHex("000007d1"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        // request read 0 inputs starting at input 0
        request = QModbusRequest(QModbusRequest::ReadDiscreteInputs, QByteArray::fromHex("00000000"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        QCOMPARE(response.data(), QByteArray::fromHex("03"));
    }

    void testProcessReadHoldingRegistersRequest()
    {
        server.setData(QModbusDataUnit::HoldingRegisters, 172, 1234u);
        server.setData(QModbusDataUnit::HoldingRegisters, 173, 4321u);

        // request read holding registers 173, address: 0x00ac -> 172, count: 0x0001 -> 1
        QModbusRequest request = QModbusRequest(QModbusRequest::ReadHoldingRegisters,
                                                QByteArray::fromHex("00ac0001"));
        QModbusResponse response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, byte count: 0x02 -> 2, value: 1234u -> 04d2
        QCOMPARE(response.data(), QByteArray::fromHex("0204d2"));

        // request read 2 registers starting at 172
        request = QModbusRequest(QModbusRequest::ReadHoldingRegisters,
                                 QByteArray::fromHex("00ac0002"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, byte count: 0x04 -> 4, status: 1234u = 04d2, 4321u =10e1
        QCOMPARE(response.data(), QByteArray::fromHex("0404d210e1"));

        // request read 10 registers starting at offset 501
        request = QModbusRequest(QModbusRequest::ReadHoldingRegisters,
                                 QByteArray::fromHex("01f5000a"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        QCOMPARE(response.data(), QByteArray::fromHex("02"));

        // request read 126 registers starting at offset 0
        request = QModbusRequest(QModbusRequest::ReadHoldingRegisters, QByteArray::fromHex("0000007e"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        // request read 0 registers starting at offset 0
        request = QModbusRequest(QModbusRequest::ReadHoldingRegisters, QByteArray::fromHex("00000000"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        QCOMPARE(response.data(), QByteArray::fromHex("03"));
    }

    void testProcessReadInputRegistersRequest()
    {
        server.setData(QModbusDataUnit::InputRegisters, 172, 1234u);
        server.setData(QModbusDataUnit::InputRegisters, 173, 4321u);

        // request read input registers 173, address: 0x00ac -> 172, count: 0x0001 -> 1
        QModbusRequest request = QModbusRequest(QModbusRequest::ReadInputRegisters,
                                                QByteArray::fromHex("00ac0001"));
        QModbusResponse response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, byte count: 0x02 -> 2, value: 1234u -> 04d2
        QCOMPARE(response.data(), QByteArray::fromHex("0204d2"));

        // request read 2 registers starting at 172
        request = QModbusRequest(QModbusRequest::ReadInputRegisters,
                                 QByteArray::fromHex("00ac0002"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, byte count: 0x04 -> 4, status: 1234u = 04d2, 4321u =10e1
        QCOMPARE(response.data(), QByteArray::fromHex("0404d210e1"));

        // request read 10 registers starting at offset 501
        request = QModbusRequest(QModbusRequest::ReadInputRegisters,
                                 QByteArray::fromHex("01f5000a"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        QCOMPARE(response.data(), QByteArray::fromHex("02"));

        // request read 1 register at offset 0 with corrupt message (+1 byte)
        request = QModbusRequest(QModbusRequest::ReadInputRegisters,
                                 QByteArray::fromHex("0000000100"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        // request read 126 registers starting at offset 0
        request = QModbusRequest(QModbusRequest::ReadInputRegisters, QByteArray::fromHex("0000007e"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        // request read 0 registers starting at offset 0
        request = QModbusRequest(QModbusRequest::ReadInputRegisters, QByteArray::fromHex("00000000"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        QCOMPARE(response.data(), QByteArray::fromHex("03"));
    }

    void testProcessWriteSingleRegisterRequest()
    {
        // request write register 173, address: 0x00ac -> 172, value: 0x00ff
        QModbusRequest request = QModbusRequest(QModbusRequest::WriteSingleRegister,
            QByteArray::fromHex("00ac00ff"));
        QModbusResponse response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("00ac00ff"));

        // request write register at offset 501
        request = QModbusRequest(QModbusRequest::WriteSingleRegister,
                                 QByteArray::fromHex("01f500ff"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        QCOMPARE(response.data(), QByteArray::fromHex("02"));

        // request write register at offset 0 value only 1 byte
        request = QModbusRequest(QModbusRequest::WriteSingleRegister,
                                 QByteArray::fromHex("00007d"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        QCOMPARE(response.data(), QByteArray::fromHex("03"));
    }

    void testProcessDiagnosticsRequest()
    {
        // subfunction 00
        QModbusRequest request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("000000ffabcd"));
        QModbusResponse response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("000000ffabcd"));


        //subfunction 01
        //TODO: impossible due to connectDevice() asking open() which is pure virtual
        //validate this in qmodbustcpserver and qmodbusrtuslave
//        request = QModbusRequest(QModbusRequest::Diagnostics,
//            QByteArray::fromHex("00010000"));
//        response = server.processRequest(request);
//        QCOMPARE(response.isException(), false);
//        // response, equals request
//        QCOMPARE(response.data(), QByteArray::fromHex("00010000"));

//        request = QModbusRequest(QModbusRequest::Diagnostics,
//            QByteArray::fromHex("0001ff00"));
//        response = server.processRequest(request);
//        QCOMPARE(response.isException(), false);
//        // response, equals request
//        QCOMPARE(response.data(), QByteArray::fromHex("0001ff00"));

        // invalidate
        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("0001ff01"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        // subfunction 02
        // validate
        request = QModbusRequest(QModbusRequest::Diagnostics,
                                 QByteArray::fromHex("00020000"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("00020000"));

        // invalidate
        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("00020001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("0002000001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        // subfunction 03
        // validate
        request = QModbusRequest(QModbusRequest::Diagnostics,
                                 QByteArray::fromHex("00030a00"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("00030a00"));

        // invalidate
        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("00030a01"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("00030a0101"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        // subfunction 04
        request = QModbusRequest(QModbusRequest::Diagnostics,
                                 QByteArray::fromHex("00040000"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("00040000"));

        // invalidate
        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("00040001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("0004000001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        // subfunction 10: counter value is 0
        request = QModbusRequest(QModbusRequest::Diagnostics,
                                 QByteArray::fromHex("000a0000"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("000a0000"));

        // invalidate
        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("000a0001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("000a000001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        // subfunction 11: counter value is 0
        request = QModbusRequest(QModbusRequest::Diagnostics,
                                 QByteArray::fromHex("000b0000"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("000b0000"));

        // invalidate
        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("000b0001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("000b000001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        // subfunction 12: counter value is 0
        request = QModbusRequest(QModbusRequest::Diagnostics,
                                 QByteArray::fromHex("000c0000"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("000c0000"));

        // invalidate
        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("000c0001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("000c000001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        // subfunction 13: counter value is 0
        request = QModbusRequest(QModbusRequest::Diagnostics,
                                 QByteArray::fromHex("000d0000"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("000d0000"));

        // invalidate
        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("000d0001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("000d000001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        // subfunction 14: counter value is 0
        request = QModbusRequest(QModbusRequest::Diagnostics,
                                 QByteArray::fromHex("000e0000"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("000e0000"));

        // invalidate
        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("000e0001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("000e000001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        // subfunction 15: counter value is 0
        request = QModbusRequest(QModbusRequest::Diagnostics,
                                 QByteArray::fromHex("000f0000"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("000f0000"));

        // invalidate
        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("000f0001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("000f000001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        // subfunction 16: counter value is 0
        request = QModbusRequest(QModbusRequest::Diagnostics,
                                 QByteArray::fromHex("00100000"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("00100000"));

        // invalidate
        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("00100001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("0010000001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        // subfunction 17: counter value is 0
        request = QModbusRequest(QModbusRequest::Diagnostics,
                                 QByteArray::fromHex("00110000"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("00110000"));

        // invalidate
        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("00110001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("0011000001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        // subfunction 18: counter value is 0
        request = QModbusRequest(QModbusRequest::Diagnostics,
                                 QByteArray::fromHex("00120000"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("00120000"));

        // invalidate
        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("00120001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("0012000001"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("03"));

        // subfunction > 4 < 10
        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("0005ff01"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("01"));

        // subfunction > 18
        request = QModbusRequest(QModbusRequest::Diagnostics,
            QByteArray::fromHex("00130000"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("01"));
    }

    void testProcessWriteMultipleRegistersRequest()
    {
        // request write at register 173, address: 0x00ac -> 172, value: 0x00ff 0x1234
        QModbusRequest request = QModbusRequest(QModbusRequest::WriteMultipleRegisters,
            QByteArray::fromHex("00ac00020400ff1234"));
        QModbusResponse response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("00ac0002"));

        // request write register at offset 501
        request = QModbusRequest(QModbusRequest::WriteMultipleRegisters,
                                 QByteArray::fromHex("01f500010200ff"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        QCOMPARE(response.data(), QByteArray::fromHex("02"));

        // request write 1 register at offset 0 value only 1 byte
        request = QModbusRequest(QModbusRequest::WriteMultipleRegisters,
                                 QByteArray::fromHex("000000010200"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        QCOMPARE(response.data(), QByteArray::fromHex("03"));
    }

    void testProcessReadWriteMultipleRegistersRequest()
    {
        server.setData(QModbusDataUnit::HoldingRegisters, 172, 1234u);
        server.setData(QModbusDataUnit::HoldingRegisters, 173, 1235u);
        server.setData(QModbusDataUnit::HoldingRegisters, 174, 1236u);

        // request read 3 registers at register 173, address: 0x00ac -> 172,
        //         write 3 registers at register 173: 1237u, 1238u, 1239u
        QModbusRequest request = QModbusRequest(QModbusRequest::ReadWriteMultipleRegisters,
            QByteArray::fromHex("00ac000300ac00030604d504d604d7"));
        QModbusResponse response = server.processRequest(request);
        QCOMPARE(response.isException(), false);
        // response, equals request
        QCOMPARE(response.data(), QByteArray::fromHex("0604d504d604d7"));

        // request write register at offset 501
        request = QModbusRequest(QModbusRequest::ReadWriteMultipleRegisters,
                                 QByteArray::fromHex("00ac000301f500010200ff"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        QCOMPARE(response.data(), QByteArray::fromHex("02"));

        // request write 3 registers at offset 173 value only 5 bytes
        request = QModbusRequest(QModbusRequest::ReadWriteMultipleRegisters,
                                 QByteArray::fromHex("00ac000300ac00030604d504d604"));
        response = server.processRequest(request);
        QCOMPARE(response.isException(), true);
        QCOMPARE(response.data(), QByteArray::fromHex("03"));
    }

    void tst_dataCalls_data()
    {
        QTest::addColumn<QModbusDataUnit::RegisterType>("registerType");

        QTest::newRow("InitCoils") << QModbusDataUnit::Coils;
        QTest::newRow("InitDiscreteInputs") << QModbusDataUnit::DiscreteInputs;
        QTest::newRow("InitInputRegisters") << QModbusDataUnit::InputRegisters;
        QTest::newRow("InitHoldingRegisters") << QModbusDataUnit::HoldingRegisters;
        QTest::newRow("InitInvalid") << QModbusDataUnit::Invalid;
    }

    void tst_dataCalls()
    {
        QFETCH(QModbusDataUnit::RegisterType, registerType);
        // basic assumption, all registers have start address 0 and size 500

        const bool validDataUnit = (registerType != QModbusDataUnit::Invalid);
        //test initialization of 0
        if (validDataUnit) {
            for (int i = 0;  i < MAP_RANGE; i++) {
                quint16 data = 123;
                QVERIFY(server.data(registerType, i, &data));
                QCOMPARE(data, quint16(0));
            }
        } else {
            quint16 data = 0;
            QVERIFY(!server.data(registerType, 0 , &data));
        }

        quint16 data = 0;
        QSignalSpy writtenSpy(
                    &server, SIGNAL(dataWritten(QModbusDataUnit::RegisterType,int,int)));
        QVERIFY(writtenSpy.isEmpty());

        QVERIFY(!server.data(registerType, MAP_RANGE+1, &data)); // out of range
        QCOMPARE(data, quint16(0));
        QVERIFY(!server.data(registerType, 1, 0)); // invalid data pointer
        QCOMPARE(data, quint16(0));

        QCOMPARE(server.setData(registerType, 1, 444), validDataUnit);
        QCOMPARE(server.data(registerType, 1, &data), validDataUnit);
        if (validDataUnit) {
            QCOMPARE(data, quint16(444));
            QTRY_COMPARE(writtenSpy.count(), 1);
            QList<QVariant> signalData = writtenSpy.at(0);
            QCOMPARE(signalData.count(), 3);
            QCOMPARE(signalData.at(0).value<QModbusDataUnit::RegisterType>(), registerType);
            QCOMPARE(signalData.at(1).toInt(), 1);
            QCOMPARE(signalData.at(2).toInt(), 1);
        } else {
            QCOMPARE(data, quint16(0));
            QTRY_VERIFY(writtenSpy.isEmpty());
        }

        //write 444 again but this time no dataWritten since no value change
        data = 0;
        writtenSpy.clear();
        QCOMPARE(server.setData(registerType, 1, 444), validDataUnit);
        QCOMPARE(server.data(registerType, 1, &data), validDataUnit);
        if (validDataUnit)
            QCOMPARE(data, quint16(444));
        else
            QCOMPARE(data, quint16(0));
        QTRY_VERIFY(writtenSpy.isEmpty()); //

        QVERIFY(!server.data(registerType, 1, 0)); // out of range although value set
        QVERIFY(!server.setData(registerType, -1, 1));
        QVERIFY(!server.setData(registerType, MAP_RANGE+1, 1));
        QTRY_VERIFY(writtenSpy.isEmpty());

        //testing server.setData(ModbusDataUnit&)

        const QVector<quint16> valueVector = { 1, 1, 1, 1, 1};
        const QVector<quint16> zeroVector = { 0, 0, 0, 0, 0};
        QModbusDataUnit rangeUnit(registerType, 7, valueVector);
        QCOMPARE(rangeUnit.valueCount(), 5u);
        QCOMPARE(rangeUnit.values().count(), 5);
        QCOMPARE(rangeUnit.startAddress(), 7);
        QVERIFY(rangeUnit.values() == valueVector);
        QVERIFY(rangeUnit.registerType() == registerType);

        writtenSpy.clear();
        QVERIFY(server.setData(rangeUnit) == validDataUnit);
        if (validDataUnit) {
            for (int i = rangeUnit.startAddress();
                 i < rangeUnit.startAddress() + int(rangeUnit.valueCount()); i++) {
                quint16 readData = 0;
                QVERIFY(server.data(registerType, i, &readData));
                QCOMPARE(readData, valueVector.at(i-rangeUnit.startAddress()));
            }

            QTRY_COMPARE(writtenSpy.count(), 1);
            QList<QVariant> signalData = writtenSpy.at(0);
            QCOMPARE(signalData.count(), 3);
            QCOMPARE(signalData.at(0).value<QModbusDataUnit::RegisterType>(), registerType);
            QCOMPARE(signalData.at(1).toInt(), rangeUnit.startAddress());
            QCOMPARE(signalData.at(2).toUInt(), rangeUnit.valueCount());
        }

        // no writtenData() signal when writing same rangeUnit again
        writtenSpy.clear();
        QVERIFY(server.setData(rangeUnit) == validDataUnit);
        QTRY_VERIFY(writtenSpy.isEmpty());

        //never fits anywhere
        QModbusDataUnit oversizeUnit(registerType, 0, MAP_RANGE*2);
        QCOMPARE(oversizeUnit.valueCount(), uint(MAP_RANGE*2));
        QCOMPARE(oversizeUnit.values().count(), MAP_RANGE*2);
        QCOMPARE(oversizeUnit.startAddress(), 0);
        QCOMPARE(oversizeUnit.registerType(), registerType);

        //completely outside of valid range
        writtenSpy.clear();
        rangeUnit.setStartAddress(MAP_RANGE + 1 );
        QVERIFY(!server.setData(rangeUnit));

        //slightly outside of valid range
        rangeUnit.setStartAddress(MAP_RANGE - 2);
        QVERIFY(!server.setData(rangeUnit));

        //slightly outside of valid range in the bottom
        rangeUnit.setStartAddress(-1);
        QVERIFY(!server.setData(rangeUnit));

        //input data unit doesn't fit
        QVERIFY(!server.setData(oversizeUnit));
        oversizeUnit.setStartAddress(-1);
        QVERIFY(!server.setData(oversizeUnit));
        oversizeUnit.setStartAddress(MAP_RANGE+1);
        QVERIFY(!server.setData(oversizeUnit));
        oversizeUnit.setStartAddress(MAP_RANGE-2);
        QVERIFY(!server.setData(oversizeUnit));
        QTRY_VERIFY(writtenSpy.isEmpty());

        //testing server.data(QModbusDataUnit *)
        QModbusDataUnit requestUnit(registerType, 7, 5);
        QCOMPARE(requestUnit.valueCount(), 5u);
        QCOMPARE(requestUnit.values().count(), 5);
        QCOMPARE(requestUnit.startAddress(), 7);
        QVERIFY(requestUnit.registerType() == registerType);
        QVERIFY(requestUnit.values() != valueVector);

        QVERIFY(server.data(&requestUnit) == validDataUnit);
        if (validDataUnit) {
            QVERIFY(requestUnit.values() == valueVector);
            QCOMPARE(requestUnit.valueCount(), 5u);
            QCOMPARE(requestUnit.values().count(), 5);
            QCOMPARE(requestUnit.startAddress(), 7);
        }

        requestUnit.setValues(zeroVector);
        QVERIFY(requestUnit.values() != valueVector);
        QVERIFY(requestUnit.values() == zeroVector);

        requestUnit.setStartAddress(MAP_RANGE + 1);
        QVERIFY(!server.data(&requestUnit));
        requestUnit.setStartAddress(MAP_RANGE - 2);
        QVERIFY(!server.data(&requestUnit));

        // ask for entire map
        requestUnit.setStartAddress(-1);
        QVERIFY(server.data(&requestUnit) == validDataUnit);
        if (validDataUnit) {
            QCOMPARE(requestUnit.valueCount(), uint(MAP_RANGE));
            QCOMPARE(requestUnit.values().count(), MAP_RANGE);
        }

        oversizeUnit.setStartAddress(0);
        QVERIFY(!server.data(&oversizeUnit));
        oversizeUnit.setStartAddress(MAP_RANGE+1);
        QVERIFY(!server.data(&oversizeUnit));
        oversizeUnit.setStartAddress(MAP_RANGE-2);
        QVERIFY(!server.data(&oversizeUnit));

        oversizeUnit.setStartAddress(-1);
        QVERIFY(server.data(&oversizeUnit) == validDataUnit);
        if (validDataUnit) {
            QCOMPARE(oversizeUnit.valueCount(), uint(MAP_RANGE));
            QCOMPARE(oversizeUnit.values().count(), MAP_RANGE);
        }
    }

    void tst_slaveAddress()
    {
        server.setSlaveAddress(56);
        QCOMPARE(server.slaveAddress(), 56);
        server.setSlaveAddress(1);
        QCOMPARE(server.slaveAddress(), 1);
    }

    void tst_diagnosticRegister()
    {
        server.setDiagnosticRegister(56u);
        QCOMPARE(server.diagnosticRegister(), quint16(56));
        server.setDiagnosticRegister(1u);
        QCOMPARE(server.diagnosticRegister(), quint16(1));
    }

    void tst_continueOnError()
    {
        server.setContinueOnError(true);
        QCOMPARE(server.continueOnError(), true);
        server.setContinueOnError(false);
        QCOMPARE(server.continueOnError(), false);
    }

};

QTEST_MAIN(tst_QModbusServer)

#include "tst_qmodbusserver.moc"
