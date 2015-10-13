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

    void setSlaveId(int) Q_DECL_OVERRIDE {}
    int slaveId() const Q_DECL_OVERRIDE { return 0; }
    bool data(QModbusDataUnit::RegisterType, quint16, quint16 *) Q_DECL_OVERRIDE { return true; }
    bool setData(QModbusDataUnit::RegisterType, quint16, quint16) Q_DECL_OVERRIDE { return true; }
    virtual bool open() Q_DECL_OVERRIDE { return true; }
    virtual void close() Q_DECL_OVERRIDE {}

    QModbusResponse processRequest(const QModbusPdu &request)
    {
        return QModbusServer::processRequest(request);
    }
};

class tst_QModbusServer : public QObject
{
    Q_OBJECT

private:
    TestServer server;

private slots:
    void init()
    {
        QModbusDataUnitMap map;
        map.insert(QModbusDataUnit::DiscreteInputs, { QModbusDataUnit::DiscreteInputs, 0, 500 });
        map.insert(QModbusDataUnit::Coils, { QModbusDataUnit::Coils, 0, 500 });
        map.insert(QModbusDataUnit::InputRegisters, { QModbusDataUnit::InputRegisters, 0, 500 });
        map.insert(QModbusDataUnit::HoldingRegisters, { QModbusDataUnit::HoldingRegisters, 0, 500 });
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
    }
};

QTEST_MAIN(tst_QModbusServer)

#include "tst_qmodbusserver.moc"
