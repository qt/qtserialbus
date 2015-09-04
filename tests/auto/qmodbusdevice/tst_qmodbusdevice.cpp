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
#include <QtCore/qpointer.h>

class dummyDevice : public QModBusDevice
{
friend class tst_QModBusDevice;

public:
    bool setDevice(QIODevice *transport, ApplicationDataUnit ADU)
    {
        Q_UNUSED(transport);
        Q_UNUSED(ADU);
        return true;
    }

protected:
    bool open() Q_DECL_OVERRIDE { return openState; }
    void close() Q_DECL_OVERRIDE {}

    bool openState;
};

class tst_QModBusDevice : public QObject
{
    Q_OBJECT
public:
    explicit tst_QModBusDevice();

private slots:
    void connectDevice();
    void state();
    void error();


private:
    QPointer<dummyDevice> device;
};

tst_QModBusDevice::tst_QModBusDevice()
{
    device = new dummyDevice();
    qRegisterMetaType<QModBusDevice::ModBusDeviceState>("QModBusDevice::ModBusDeviceState");
}

void tst_QModBusDevice::connectDevice()
{
    device->openState = false;
    QVERIFY(!device->connectDevice());
    QCOMPARE(device->state(), QModBusDevice::UnconnectedState);

    device->openState = true;
    QVERIFY(device->connectDevice());
    QCOMPARE(device->state(), QModBusDevice::ConnectingState);

    device->disconnectDevice();
    QCOMPARE(device->state(), QModBusDevice::ClosingState);

    device->setState(QModBusDevice::ClosingState);
    QVERIFY(!device->connectDevice());
}

void tst_QModBusDevice::state()
{
    device->setState(QModBusDevice::ConnectedState);
    QCOMPARE(device->state(), QModBusDevice::ConnectedState);
    QSignalSpy spy(device, SIGNAL(stateChanged(QModBusDevice::ModBusDeviceState)));
    device->setState(QModBusDevice::UnconnectedState);
    QCOMPARE(device->state(), QModBusDevice::UnconnectedState);
    device->setState(QModBusDevice::UnconnectedState);
    QCOMPARE(device->state(), QModBusDevice::UnconnectedState);

    QCOMPARE(spy.count(), 1);
}

void tst_QModBusDevice::error()
{
    QCOMPARE(device->error(), QModBusDevice::NoError);
    QVERIFY(device->errorString().isEmpty());

    QString errorString("error string");
    device->setError(errorString, QModBusDevice::ConnectionError);

    QCOMPARE(device->error(), QModBusDevice::ConnectionError);
    QCOMPARE(device->errorString(), errorString);
}

QTEST_MAIN(tst_QModBusDevice)

#include "tst_qmodbusdevice.moc"
