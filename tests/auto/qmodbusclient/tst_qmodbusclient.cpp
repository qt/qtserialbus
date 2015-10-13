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

#include <QtSerialBus/qmodbusclient.h>

#include <QtTest/QtTest>

class TestClient : public QModbusClient
{
public:
    TestClient() Q_DECL_EQ_DEFAULT;

    QModbusReplyEx * sendRequest(const QModbusDataUnit &, int) Q_DECL_OVERRIDE { return Q_NULLPTR; }
    QModbusReply *read(const QModbusDataUnit &, int)  Q_DECL_OVERRIDE { return Q_NULLPTR; }
    QModbusReply *write(const QModbusDataUnit &, int) Q_DECL_OVERRIDE { return Q_NULLPTR; }
    virtual bool open() Q_DECL_OVERRIDE { return true; }
    virtual void close() Q_DECL_OVERRIDE {}

    bool processResponse(const QModbusResponse &response, QModbusDataUnit *data)
    {
        return QModbusClient::processResponse(response, data);
    }
};

class tst_QModbusClient : public QObject
{
    Q_OBJECT

private slots:
    void testProcessResponseReadWriteSingleMultipleCoils()
    {
        TestClient client;

        QModbusDataUnit unit;
        QModbusResponse response = QModbusResponse(QModbusResponse::ReadCoils,
            QByteArray::fromHex("03cd6b05"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), true);
        QCOMPARE(unit.valueCount(), 24);
        QCOMPARE(unit.startAddress(), 0);
        QCOMPARE(unit.values(),
            QVector<quint16>({ 1,0,1,1,0,0,1,1, 1,1,0,1,0,1,1,0, 1,0,1,0,0,0,0,0 }));
        QCOMPARE(unit.registerType(), QModbusDataUnit::Coils);

        unit = QModbusDataUnit();
        response = QModbusResponse(QModbusResponse::WriteSingleCoil,
            QByteArray::fromHex("00acff00"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), true);
        QCOMPARE(unit.valueCount(), 1);
        QCOMPARE(unit.startAddress(), 172);
        QCOMPARE(unit.values(), QVector<quint16>() << 0xff00);
        QCOMPARE(unit.registerType(), QModbusDataUnit::Coils);

        unit = QModbusDataUnit();
        response = QModbusResponse(QModbusResponse::WriteMultipleCoils,
            QByteArray::fromHex("0013000a"));
        QCOMPARE(client.processResponse(response, &unit), true);

        QCOMPARE(unit.isValid(), true);
        QCOMPARE(unit.valueCount(), 10);
        QCOMPARE(unit.startAddress(), 19);
        QCOMPARE(unit.values(), QVector<quint16>());
        QCOMPARE(unit.registerType(), QModbusDataUnit::Coils);
    }
};

QTEST_MAIN(tst_QModbusClient)

#include "tst_qmodbusclient.moc"
