// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtSerialBus/qmodbusclient.h>
#include <QtSerialBus/qmodbustcpclient.h>
#include <private/qmodbusclient_p.h>
#include <private/qmodbus_symbols_p.h>

#include <QtCore/qendian.h>
#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>
#include <QtTest/QtTest>

class TestClient : public QModbusClient
{
    Q_OBJECT
    class TestClientPrivate : public QModbusClientPrivate
    {
        Q_DECLARE_PUBLIC(TestClient)

    public:
        bool isOpen() const override { return m_open; }

    private:
        bool m_open = false;
    };

public:
    TestClient()
        : QModbusClient(*new TestClientPrivate)
    {}
    bool open() override {
        d_func()->m_open = true;
        setState(QModbusDevice::ConnectedState);
        return true;
    }
    void close() override {
        d_func()->m_open = true;
        setState(QModbusDevice::UnconnectedState);
    }
    bool processResponse(const QModbusResponse &response, QModbusDataUnit *data) override
    {
        return QModbusClient::processResponse(response, data);
    }
    Q_DECLARE_PRIVATE(TestClient)
};

class tst_QModbusClient : public QObject
{
    Q_OBJECT

private slots:
    void testTimeout()
    {
        TestClient client;
        QCOMPARE(client.timeout(), 1000); //default value test

        QSignalSpy spy(&client, &TestClient::timeoutChanged);
        client.setTimeout(50);
        QCOMPARE(client.timeout(), 50);
        QCOMPARE(spy.isEmpty(), false); // we expect the signal

        spy.clear();
        client.setTimeout(9); // everything below 10 fails
        QVERIFY(client.timeout() >= 10);
        QCOMPARE(spy.isEmpty(), true); // and the signal should not fire
    }

    void testNumberOfRetries()
    {
        TestClient client;
        QCOMPARE(client.numberOfRetries(), 3);

        client.setNumberOfRetries(-1);  // ignore everything below 0
        QCOMPARE(client.numberOfRetries(), 3);

        client.setNumberOfRetries(1);
        QCOMPARE(client.numberOfRetries(), 1);
    }

    void testProcessReadCoilsResponse()
    {
        TestClient client;
        const QList<quint16> values({ 1,0,1,1,0,0,1,1, 1,1,0,1,0,1,1,0, 1,0,1,0,0,0,0,0 });

        QModbusDataUnit unit(QModbusDataUnit::Coils, 100, 24);
        QModbusResponse response = QModbusResponse(QModbusResponse::ReadCoils,
            QByteArray::fromHex("03cd6b05"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), true);
        QCOMPARE(unit.valueCount(), 24);
        QCOMPARE(unit.startAddress(), 100);
        QCOMPARE(unit.values(), values);
        QCOMPARE(unit.registerType(), QModbusDataUnit::Coils);

        unit = {};
        response = QModbusResponse(QModbusResponse::ReadCoils,
            QByteArray::fromHex("03cd6b05"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), false);
        QCOMPARE(unit.valueCount(), 24);
        QCOMPARE(unit.startAddress(), -1);
        QCOMPARE(unit.values(), values);
        QCOMPARE(unit.registerType(), QModbusDataUnit::Coils);

        unit = {};
        unit.setStartAddress(1);
        response = QModbusResponse(QModbusResponse::ReadCoils,
            QByteArray::fromHex("03cd6b05"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), true);
        QCOMPARE(unit.valueCount(), 24);
        QCOMPARE(unit.startAddress(), 1);
        QCOMPARE(unit.values(), values);
        QCOMPARE(unit.registerType(), QModbusDataUnit::Coils);
    }

    void testProcessWriteSingleMultipleCoilsResponse()
    {
        TestClient client;

        QModbusDataUnit unit;
        QModbusResponse response = QModbusResponse(QModbusResponse::WriteSingleCoil,
            QByteArray::fromHex("00acff00"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), true);
        QCOMPARE(unit.valueCount(), 1);
        QCOMPARE(unit.startAddress(), 172);
        QCOMPARE(unit.values(), QList<quint16>() << Coil::On);
        QCOMPARE(unit.registerType(), QModbusDataUnit::Coils);

        unit = QModbusDataUnit();
        response = QModbusResponse(QModbusResponse::WriteMultipleCoils,
            QByteArray::fromHex("0013000a"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), true);
        QCOMPARE(unit.valueCount(), 10);
        QCOMPARE(unit.startAddress(), 19);
        QCOMPARE(unit.values(), QList<quint16>());
        QCOMPARE(unit.registerType(), QModbusDataUnit::Coils);
    }

    void testProcessReadDiscreteInputsResponse()
    {
        TestClient client;
        const QList<quint16> values({ 0,0,1,1,0,1,0,1, 1,1,0,1,1,0,1,1, 1,0,1,0,1,1,0,0 });

        QModbusDataUnit unit(QModbusDataUnit::DiscreteInputs, 100, 24);
        QModbusResponse response = QModbusResponse(QModbusResponse::ReadDiscreteInputs,
            QByteArray::fromHex("03acdb35"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), true);
        QCOMPARE(unit.valueCount(), 24);
        QCOMPARE(unit.startAddress(), 100);
        QCOMPARE(unit.values(), values);
        QCOMPARE(unit.registerType(), QModbusDataUnit::DiscreteInputs);

        response.setFunctionCode(QModbusPdu::FunctionCode(0x82));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setFunctionCode(QModbusResponse::ReadDiscreteInputs);
        response.setData(QByteArray::fromHex("35"));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setData(QByteArray::fromHex("03acdb"));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setData(QByteArray::fromHex("03acdb3517"));
        QCOMPARE(client.processResponse(response, &unit), false);

        unit = {};
        response = QModbusResponse(QModbusResponse::ReadDiscreteInputs,
            QByteArray::fromHex("03acdb35"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), false);
        QCOMPARE(unit.valueCount(), 24);
        QCOMPARE(unit.startAddress(), -1);
        QCOMPARE(unit.values(), values);
        QCOMPARE(unit.registerType(), QModbusDataUnit::DiscreteInputs);

        unit = {};
        unit.setStartAddress(1);
        response = QModbusResponse(QModbusResponse::ReadDiscreteInputs,
            QByteArray::fromHex("03acdb35"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), true);
        QCOMPARE(unit.valueCount(), 24);
        QCOMPARE(unit.startAddress(), 1);
        QCOMPARE(unit.values(), values);
        QCOMPARE(unit.registerType(), QModbusDataUnit::DiscreteInputs);
    }

    void testProcessReadHoldingRegistersResponse()
    {
        TestClient client;
        const QList<quint16> values({ 52587, 1407 });

        QModbusDataUnit unit;
        unit.setStartAddress(100);
        QModbusResponse response = QModbusResponse(QModbusResponse::ReadHoldingRegisters,
                                                   QByteArray::fromHex("04cd6b057f"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), true);
        QCOMPARE(unit.valueCount(), 2);
        QCOMPARE(unit.startAddress(), 100);
        QCOMPARE(unit.values(), values);
        QCOMPARE(unit.registerType(), QModbusDataUnit::HoldingRegisters);

        response.setFunctionCode(QModbusPdu::FunctionCode(0x83));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setFunctionCode(QModbusResponse::ReadHoldingRegisters);
        response.setData(QByteArray::fromHex("05"));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setData(QByteArray::fromHex("04cd6b"));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setData(QByteArray::fromHex("04cd6b051755"));
        QCOMPARE(client.processResponse(response, &unit), false);

        unit = {};
        response = QModbusResponse(QModbusResponse::ReadHoldingRegisters,
            QByteArray::fromHex("04cd6b057f"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), false);
        QCOMPARE(unit.valueCount(), 2);
        QCOMPARE(unit.startAddress(), -1);
        QCOMPARE(unit.values(), values);
        QCOMPARE(unit.registerType(), QModbusDataUnit::HoldingRegisters);

        unit = {};
        unit.setStartAddress(1);
        response = QModbusResponse(QModbusResponse::ReadHoldingRegisters,
            QByteArray::fromHex("04cd6b057f"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), true);
        QCOMPARE(unit.valueCount(), 2);
        QCOMPARE(unit.startAddress(), 1);
        QCOMPARE(unit.values(), values);
        QCOMPARE(unit.registerType(), QModbusDataUnit::HoldingRegisters);
    }

    void testProcessReadInputRegistersResponse()
    {
        TestClient client;
        const QList<quint16> values({ 52587, 1407 });

        QModbusDataUnit unit;
        unit.setStartAddress(100);
        QModbusResponse response = QModbusResponse(QModbusResponse::ReadInputRegisters,
                                                   QByteArray::fromHex("04cd6b057f"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), true);
        QCOMPARE(unit.valueCount(), 2);
        QCOMPARE(unit.startAddress(), 100);
        QCOMPARE(unit.values(), values);
        QCOMPARE(unit.registerType(), QModbusDataUnit::InputRegisters);

        response.setFunctionCode(QModbusPdu::FunctionCode(0x84));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setFunctionCode(QModbusResponse::ReadInputRegisters);
        response.setData(QByteArray::fromHex("05"));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setData(QByteArray::fromHex("04cd6b"));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setData(QByteArray::fromHex("04cd6b051755"));
        QCOMPARE(client.processResponse(response, &unit), false);

        unit = {};
        response = QModbusResponse(QModbusResponse::ReadInputRegisters,
            QByteArray::fromHex("04cd6b057f"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), false);
        QCOMPARE(unit.valueCount(), 2);
        QCOMPARE(unit.startAddress(), -1);
        QCOMPARE(unit.values(), values);
        QCOMPARE(unit.registerType(), QModbusDataUnit::InputRegisters);

        unit = {};
        unit.setStartAddress(1);
        response = QModbusResponse(QModbusResponse::ReadInputRegisters,
            QByteArray::fromHex("04cd6b057f"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), true);
        QCOMPARE(unit.valueCount(), 2);
        QCOMPARE(unit.startAddress(), 1);
        QCOMPARE(unit.values(), values);
        QCOMPARE(unit.registerType(), QModbusDataUnit::InputRegisters);
    }

    void testProcessWriteSingleRegisterResponse()
    {
        TestClient client;

        QModbusDataUnit unit;
        QModbusResponse response = QModbusResponse(QModbusResponse::WriteSingleRegister,
            QByteArray::fromHex("04cd6b05"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), true);
        QCOMPARE(unit.valueCount(), 1);
        QCOMPARE(unit.startAddress(), 1229);
        QCOMPARE(unit.values(), QList<quint16>(1, 27397u));
        QCOMPARE(unit.registerType(), QModbusDataUnit::HoldingRegisters);

        response.setFunctionCode(QModbusPdu::FunctionCode(0x86));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setFunctionCode(QModbusResponse::WriteSingleRegister);
        response.setData(QByteArray::fromHex("05"));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setData(QByteArray::fromHex("04cd6b051755"));
        QCOMPARE(client.processResponse(response, &unit), false);
    }

    void testProcessWriteMultipleRegistersResponse()
    {
        TestClient client;

        QModbusDataUnit unit;
        QModbusResponse response = QModbusResponse(QModbusResponse::WriteMultipleRegisters,
            QByteArray::fromHex("03cd007b"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), true);
        QCOMPARE(unit.valueCount(), 123);
        QCOMPARE(unit.startAddress(), 973);
        QCOMPARE(unit.registerType(), QModbusDataUnit::HoldingRegisters);

        response.setFunctionCode(QModbusPdu::FunctionCode(0x90));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setFunctionCode(QModbusResponse::WriteMultipleRegisters);
        response.setData(QByteArray::fromHex("05"));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setData(QByteArray::fromHex("03cd6b"));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setData(QByteArray::fromHex("03cd6b0517"));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setData(QByteArray::fromHex("03cd0000"));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setData(QByteArray::fromHex("03cd007c"));
        QCOMPARE(client.processResponse(response, &unit), false);
    }

    void testProcessReadWriteMultipleRegistersResponse()
    {
        TestClient client;
        const QList<quint16> values({ 52587, 1407 });

        QModbusDataUnit unit;
        unit.setStartAddress(100);
        QModbusResponse response = QModbusResponse(QModbusResponse::ReadWriteMultipleRegisters,
            QByteArray::fromHex("04cd6b057f"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), true);
        QCOMPARE(unit.valueCount(), 2);
        QCOMPARE(unit.values(), values);
        QCOMPARE(unit.registerType(), QModbusDataUnit::HoldingRegisters);

        response.setFunctionCode(QModbusPdu::FunctionCode(0x97));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setFunctionCode(QModbusResponse::ReadWriteMultipleRegisters);
        response.setData(QByteArray::fromHex("05"));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setData(QByteArray::fromHex("04cd6b"));
        QCOMPARE(client.processResponse(response, &unit), false);

        response.setData(QByteArray::fromHex("04cd6b051755"));
        QCOMPARE(client.processResponse(response, &unit), false);

        unit = {};
        response = QModbusResponse(QModbusResponse::ReadWriteMultipleRegisters,
            QByteArray::fromHex("04cd6b057f"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), false);
        QCOMPARE(unit.valueCount(), 2);
        QCOMPARE(unit.startAddress(), -1);
        QCOMPARE(unit.values(), values);
        QCOMPARE(unit.registerType(), QModbusDataUnit::HoldingRegisters);

        unit = {};
        unit.setStartAddress(1);
        response = QModbusResponse(QModbusResponse::ReadWriteMultipleRegisters,
            QByteArray::fromHex("04cd6b057f"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), true);
        QCOMPARE(unit.valueCount(), 2);
        QCOMPARE(unit.startAddress(), 1);
        QCOMPARE(unit.values(), values);
        QCOMPARE(unit.registerType(), QModbusDataUnit::HoldingRegisters);
    }

    void testPrivateCreateReadRequest_data()
    {
        QTest::addColumn<QModbusDataUnit::RegisterType>("rc");
        QTest::addColumn<int>("address");
        QTest::addColumn<int>("count");
        QTest::addColumn<QModbusPdu::FunctionCode>("fc");
        QTest::addColumn<QByteArray>("data");
        QTest::addColumn<bool>("isValid");

        QTest::newRow("QModbusDataUnit::Invalid") << QModbusDataUnit::Invalid << 19 << 19
            << QModbusRequest::Invalid << QByteArray() << false;
        QTest::newRow("QModbusDataUnit::Coils") << QModbusDataUnit::Coils << 19 << 19
            << QModbusRequest::ReadCoils << QByteArray::fromHex("00130013") << true;
        QTest::newRow("QModbusDataUnit::DiscreteInputs") << QModbusDataUnit::DiscreteInputs << 19
            << 19 << QModbusRequest::ReadDiscreteInputs << QByteArray::fromHex("00130013") << true;
        QTest::newRow("QModbusDataUnit::InputRegisters") << QModbusDataUnit::InputRegisters << 19
            << 19 << QModbusRequest::ReadInputRegisters << QByteArray::fromHex("00130013") << true;
        QTest::newRow("QModbusDataUnit::HoldingRegisters") << QModbusDataUnit::HoldingRegisters
            << 19 << 19 << QModbusRequest::ReadHoldingRegisters << QByteArray::fromHex("00130013")
            << true;
    }

    void testPrivateCreateReadRequest()
    {
        QFETCH(QModbusDataUnit::RegisterType, rc);
        QFETCH(int, address);
        QFETCH(int, count);

        TestClient client;
        QModbusDataUnit read(rc, address, count);
        QModbusRequest request = client.d_func()->createReadRequest(read);
        QTEST(request.functionCode(), "fc");
        QTEST(request.data(), "data");
        QTEST(request.isValid(), "isValid");
    }

    void testPrivateCreateWriteRequest_data()
    {
        QTest::addColumn<QModbusDataUnit::RegisterType>("rc");
        QTest::addColumn<int>("address");
        QTest::addColumn<QList<quint16>>("values");
        QTest::addColumn<QModbusPdu::FunctionCode>("fc");
        QTest::addColumn<QByteArray>("data");
        QTest::addColumn<bool>("isValid");

        QTest::newRow("QModbusDataUnit::Invalid") << QModbusDataUnit::Invalid << 19 << (QList<quint16> { 1 })
                                                  << QModbusRequest::Invalid << QByteArray() << false;
        QTest::newRow("QModbusDataUnit::DiscreteInputs")
                << QModbusDataUnit::DiscreteInputs << 19 << (QList<quint16> { 1 }) << QModbusRequest::Invalid
                << QByteArray() << false;
        QTest::newRow("QModbusDataUnit::InputRegisters")
                << QModbusDataUnit::InputRegisters << 19 << (QList<quint16> { 1 }) << QModbusRequest::Invalid
                << QByteArray() << false;

        QTest::newRow("QModbusDataUnit::Coils{Single}")
                << QModbusDataUnit::Coils << 172 << (QList<quint16> { 1 }) << QModbusRequest::WriteSingleCoil
                << QByteArray::fromHex("00acff00") << true;
        QTest::newRow("QModbusDataUnit::Coils{Multiple}")
                << QModbusDataUnit::Coils << 19
                << (QList<quint16> { 1,0,1,1,0,0,1,1, 1,0 /* 6 times padding */ })
                << QModbusRequest::WriteMultipleCoils << QByteArray::fromHex("0013000a02cd01") << true;
        QTest::newRow("QModbusDataUnit::HoldingRegisters{Single}")
                << QModbusDataUnit::HoldingRegisters << 1229 << (QList<quint16> { 27397u })
                << QModbusRequest::WriteSingleRegister << QByteArray::fromHex("04cd6b05") << true;
        QTest::newRow("QModbusDataUnit::HoldingRegisters{Multiple}")
                << QModbusDataUnit::HoldingRegisters << 1 << (QList<quint16> { 27397u, 27397u })
                << QModbusRequest::WriteMultipleRegisters << QByteArray::fromHex("00010002046b056b05") << true;
    }

    void testPrivateCreateWriteRequest()
    {
        QFETCH(QModbusDataUnit::RegisterType, rc);
        QFETCH(int, address);
        QFETCH(QList<quint16>, values);

        TestClient client;
        QModbusDataUnit write(rc, address, values);
        QModbusRequest request = client.d_func()->createWriteRequest(write);
        QTEST(request.functionCode(), "fc");
        QTEST(request.data(), "data");
        QTEST(request.isValid(), "isValid");
    }

    void testPrivateCreateRWRequest_data()
    {
        QTest::addColumn<QModbusDataUnit::RegisterType>("rc");
        QTest::addColumn<int>("address");
        QTest::addColumn<QList<quint16>>("values");
        QTest::addColumn<QModbusPdu::FunctionCode>("fc");
        QTest::addColumn<QByteArray>("data");
        QTest::addColumn<bool>("isValid");

        QTest::newRow("QModbusDataUnit::Invalid") << QModbusDataUnit::Invalid << 19 << (QList<quint16> { 1 })
                                                  << QModbusRequest::Invalid << QByteArray() << false;
        QTest::newRow("QModbusDataUnit::Coils") << QModbusDataUnit::Invalid << 172 << (QList<quint16> { 1 })
                                                << QModbusRequest::Invalid << QByteArray() << false;
        QTest::newRow("QModbusDataUnit::DiscreteInputs")
                << QModbusDataUnit::DiscreteInputs << 19 << (QList<quint16> { 1 }) << QModbusRequest::Invalid
                << QByteArray() << false;
        QTest::newRow("QModbusDataUnit::InputRegisters")
                << QModbusDataUnit::InputRegisters << 19 << (QList<quint16> { 1 }) << QModbusRequest::Invalid
                << QByteArray() << false;
        QTest::newRow("QModbusDataUnit::HoldingRegisters{Read|Write}")
                << QModbusDataUnit::HoldingRegisters << 1 << (QList<quint16> { 27397u, 27397u })
                << QModbusRequest::ReadWriteMultipleRegisters << QByteArray::fromHex("0001000200010002046b056b05")
                << true;
    }

    void testPrivateCreateRWRequest()
    {
        QFETCH(QModbusDataUnit::RegisterType, rc);
        QFETCH(int, address);
        QFETCH(QList<quint16>, values);

        TestClient client;
        QModbusDataUnit read(rc, address, values.size());
        QModbusDataUnit write(rc, address, values);
        QModbusRequest request = client.d_func()->createRWRequest(read, write);
        QTEST(request.functionCode(), "fc");
        QTEST(request.data(), "data");
        QTEST(request.isValid(), "isValid");
    }

    void testPrivateSendRequest()
    {
        TestClient client;
        QModbusDataUnit unit;
        QModbusReply *reply = nullptr;

        QTest::ignoreMessage(QtWarningMsg, "(Client) Device is not connected");
        QCOMPARE(client.d_func()->sendRequest(QModbusRequest(), 1, &unit), reply);
        QCOMPARE(client.error(), QModbusDevice::ConnectionError);
        QCOMPARE(client.errorString(), QString("Device not connected."));

        QTest::ignoreMessage(QtWarningMsg, "(Client) Device is not connected");
        QCOMPARE(client.d_func()->sendRequest(QModbusRequest(), 1, nullptr), reply);
        QCOMPARE(client.error(), QModbusDevice::ConnectionError);
        QCOMPARE(client.errorString(), QString("Device not connected."));

        QCOMPARE(client.connectDevice(), true);

        QTest::ignoreMessage(QtWarningMsg, "(Client) Refuse to send invalid request.");
        QCOMPARE(client.d_func()->sendRequest(QModbusRequest(), 1, &unit), reply);
        QCOMPARE(client.error(), QModbusDevice::ProtocolError);
        QCOMPARE(client.errorString(), QString("Invalid Modbus request."));

        QTest::ignoreMessage(QtWarningMsg, "(Client) Refuse to send invalid request.");
        QCOMPARE(client.d_func()->sendRequest(QModbusRequest(), 1, nullptr), reply);
        QCOMPARE(client.error(), QModbusDevice::ProtocolError);
        QCOMPARE(client.errorString(), QString("Invalid Modbus request."));

        QModbusRequest request(QModbusRequest::Diagnostics);
        // The default implementation is empty and returns a null pointer.
        QCOMPARE(client.d_func()->sendRequest(request, 1, &unit), reply);
        QCOMPARE(client.d_func()->sendRequest(request, 1, nullptr), reply);
    }

    void testTcpMbapLengthField_data()
    {
        QTest::addColumn<QByteArray>("frame");
        QTest::addColumn<bool>("accepted");

        // MBAP header: transaction id (2), protocol id (2), length (2), unit id (1).
        // The length field counts the unit id, so 2 is its smallest legal value.
        QTest::newRow("length zero") << QByteArray::fromHex("00000000000001") << false;
        QTest::newRow("length one") << QByteArray::fromHex("00000000000101") << false;
        QTest::newRow("one above the maximum") << QByteArray::fromHex("0000000000ff01") << false;
        QTest::newRow("length huge") << QByteArray::fromHex("00000000ffff01") << false;
        // 254 describes the largest legal frame, a 260 byte ADU. It is accepted by
        // the framing, here rejected later because the padding is not a valid PDU.
        QTest::newRow("maximum length")
            << (QByteArray::fromHex("0000000000fe01") + QByteArray(253, 0x03)) << true;
    }

    void testTcpMbapLengthField()
    {
        QFETCH(QByteArray, frame);
        QFETCH(bool, accepted);

        QTcpServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost, 0));

        QModbusTcpClient client;
        client.setConnectionParameter(QModbusDevice::NetworkAddressParameter,
            QStringLiteral("127.0.0.1"));
        client.setConnectionParameter(QModbusDevice::NetworkPortParameter, server.serverPort());
        QVERIFY(client.connectDevice());
        QTRY_COMPARE(client.state(), QModbusDevice::ConnectedState);
        QTRY_VERIFY(server.hasPendingConnections());
        QTcpSocket *remote = server.nextPendingConnection();

        QModbusReply *reply = client.sendReadRequest(
            QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 1), 1);
        QVERIFY(reply);

        if (!accepted) {
            const QByteArray warning = "(TCP client) Invalid MBAP length field: "
                + QByteArray::number(qFromBigEndian<quint16>(frame.constData() + 4))
                + " closing connection.";
            QTest::ignoreMessage(QtWarningMsg, warning.constData());
        }

        remote->write(frame);
        QTRY_VERIFY(reply->isFinished());

        if (!accepted) {
            QCOMPARE(reply->error(), QModbusDevice::ReplyAbortedError);
            QTRY_COMPARE(client.state(), QModbusDevice::UnconnectedState);
            QCOMPARE(client.error(), QModbusDevice::ProtocolError);
            return;
        }

        // Accepted by the framing, so the connection survives and only the
        // bogus PDU is rejected. No value reaches the application.
        QCOMPARE(reply->error(), QModbusDevice::InvalidResponseError);
        QCOMPARE(reply->result().valueCount(), 0);
        QCOMPARE(client.state(), QModbusDevice::ConnectedState);
    }
};

QTEST_MAIN(tst_QModbusClient)

#include "tst_qmodbusclient.moc"
