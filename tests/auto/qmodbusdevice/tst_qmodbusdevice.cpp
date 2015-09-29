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

#include <QtTest/QtTest>
#include <QtSerialBus/qmodbusdevice.h>
#include <QtSerialBus/private/qmodbusdevice_p.h>
#include <QtCore/qpointer.h>

class dummyDevice : public QModbusDevice
{
friend class tst_QModbusDevice;

protected:
    bool open() Q_DECL_OVERRIDE { return openState; }
    void close() Q_DECL_OVERRIDE {}

    bool openState;
};

class tst_QModbusDevice : public QObject
{
    Q_OBJECT
public:
    explicit tst_QModbusDevice();

private slots:
    void initTestCase()
    {
        dp = new QModbusDevicePrivate;
    }
    void cleanupTestCase()
    {
        // TODO: it doesn't like the delete
        // delete dp;
    }

    void connectDevice();
    void state();
    void error();

    void testChecksumLRC_data();
    void testChecksumLRC();

private:
    QModbusDevicePrivate *dp;
    QPointer<dummyDevice> device;
};

tst_QModbusDevice::tst_QModbusDevice()
{
    device = new dummyDevice();
    qRegisterMetaType<QModbusDevice::ModBusDeviceState>("QModbusDevice::ModBusDeviceState");
}

void tst_QModbusDevice::connectDevice()
{
    device->openState = false;
    QVERIFY(!device->connectDevice());
    QCOMPARE(device->state(), QModbusDevice::UnconnectedState);

    device->openState = true;
    QVERIFY(device->connectDevice());
    QCOMPARE(device->state(), QModbusDevice::ConnectingState);

    device->disconnectDevice();
    QCOMPARE(device->state(), QModbusDevice::ClosingState);

    device->setState(QModbusDevice::ClosingState);
    QVERIFY(!device->connectDevice());
}

void tst_QModbusDevice::state()
{
    device->setState(QModbusDevice::ConnectedState);
    QCOMPARE(device->state(), QModbusDevice::ConnectedState);
    QSignalSpy spy(device, SIGNAL(stateChanged(QModbusDevice::ModBusDeviceState)));
    device->setState(QModbusDevice::UnconnectedState);
    QCOMPARE(device->state(), QModbusDevice::UnconnectedState);
    device->setState(QModbusDevice::UnconnectedState);
    QCOMPARE(device->state(), QModbusDevice::UnconnectedState);

    QCOMPARE(spy.count(), 1);
}

void tst_QModbusDevice::error()
{
    QCOMPARE(device->error(), QModbusDevice::NoError);
    QVERIFY(device->errorString().isEmpty());

    QString errorString("error string");
    device->setError(errorString, QModbusDevice::ConnectionError);

    QCOMPARE(device->error(), QModbusDevice::ConnectionError);
    QCOMPARE(device->errorString(), errorString);
}

void tst_QModbusDevice::testChecksumLRC_data()
{
    // Modbus ASCII Messages generated with pymodbus message-generator.py

    QTest::addColumn<QByteArray>("pdu");
    QTest::addColumn<quint8>("lrc");

    QTest::newRow(":0107F8")
        << QByteArray::fromHex("0107")
        << quint8(0xF8);
    QTest::newRow(":010BF4")
        << QByteArray::fromHex("010B")
        << quint8(0xF4);
    QTest::newRow(":010CF3")
        << QByteArray::fromHex("010C")
        << quint8(0xF3);
    QTest::newRow(":0111EE")
        << QByteArray::fromHex("0111")
        << quint8(0xEE);
    QTest::newRow(":011400EB")
        << QByteArray::fromHex("011400")
        << quint8(0xEB);
    QTest::newRow(":011500EA")
        << QByteArray::fromHex("011500")
        << quint8(0xEA);
    QTest::newRow(":1103006B00037E")
        << QByteArray::fromHex("1103006B0003")
        << quint8(0x7E);
    QTest::newRow(":01160012FFFF0000D9")
        << QByteArray::fromHex("01160012FFFF0000")
        << quint8(0xD9);
    QTest::newRow(":0110001200081000010001000100010001000100010001BD")
        << QByteArray::fromHex("0110001200081000010001000100010001000100010001")
        << quint8(0xBD);
    QTest::newRow(":011700120008000000081000010001000100010001000100010001AE")
        << QByteArray::fromHex("011700120008000000081000010001000100010001000100010001")
        << quint8(0xAE);

}
void tst_QModbusDevice::testChecksumLRC()
{
    QFETCH(QByteArray, pdu);
    QFETCH(quint8, lrc);

    QCOMPARE(dp->calculateLRC(pdu.constData(), pdu.size()), lrc);
    QCOMPARE(dp->checkLRC(pdu.constData(), pdu.size(), lrc), true);
}
}

QTEST_MAIN(tst_QModbusDevice)

#include "tst_qmodbusdevice.moc"
