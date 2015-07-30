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

#include <QtSerialBus/QCanBusDevice>
#include <QtSerialBus/QCanBusFrame>

#include <QtTest/QtTest>
#include <QSignalSpy>

class tst_Backend : public QCanBusDevice
{
    Q_OBJECT
public:
    tst_Backend()
    {
        referenceFrame.setFrameId(5);
        referenceFrame.setPayload(QByteArray("FOOBAR"));
        QCanBusFrame::TimeStamp stamp;
        stamp.setSeconds(22);
        stamp.setMicroSeconds(23);
        referenceFrame.setTimeStamp(stamp);
        referenceFrame.setExtendedFrameFormat(1);

        // this test assumes that each readFrame() call returns referenceFrame
        // fill the buffer with frames
        for (int i = 0; i < 5; i++)
            enqueueReceivedFrame(referenceFrame);
    }

    bool open()
    {
        setState(QCanBusDevice::ConnectedState);
        return true;
    }

    void close()
    {
        setState(QCanBusDevice::UnconnectedState);
    }

    bool writeFrame(const QCanBusFrame &/*data*/)
    {
        if (state() != QCanBusDevice::ConnectedState)
            return false;

        emit written();
        return true;
    }

    void emulateError(const QString &text, QCanBusDevice::CanBusError e)
    {
        setError(text, e);
    }

    QString interpretErrorFrame(const QCanBusFrame &/*errorFrame*/)
    {
        return QString();
    }

signals:
    void written();

private:
    QCanBusFrame referenceFrame;
};

class tst_QCanBusDevice : public QObject
{
    Q_OBJECT
public:
    explicit tst_QCanBusDevice();

private slots:
    void initTestCase();
    void conf();
    void write();
    void read();
    void error();
    void cleanupTestCase();

private:
    QPointer<QCanBusDevice> device;
};

tst_QCanBusDevice::tst_QCanBusDevice() :
    device(0)
{
    qRegisterMetaType<QCanBusDevice::CanBusDeviceState>();
    qRegisterMetaType<QCanBusDevice::CanBusError>();
}

void tst_QCanBusDevice::initTestCase()
{
    device = new tst_Backend();
    QVERIFY(device);

    QSignalSpy stateSpy(device.data(),
                        SIGNAL(stateChanged(QCanBusDevice::CanBusDeviceState)));


    QVERIFY(device->connectDevice());
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState,
                             5000);
    QCOMPARE(stateSpy.count(), 2);
    QCOMPARE(stateSpy[0].at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectingState);
    QCOMPARE(stateSpy[1].at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectedState);
}

void tst_QCanBusDevice::conf()
{
    QVERIFY(device->configurationKeys().isEmpty());

    // invalid QVariant ignored
    device->setConfigurationParameter(QCanBusDevice::RawFilterKey, QVariant());
    QVERIFY(device->configurationKeys().isEmpty());

    QCanBusFrame::FrameErrors error =
            (QCanBusFrame::LostArbitrationError | QCanBusFrame::BusError);

    device->setConfigurationParameter(
                QCanBusDevice::ErrorFilterKey, QVariant::fromValue(error));
    QVariant value = device->configurationParameter(QCanBusDevice::ErrorFilterKey);
    QVERIFY(value.isValid());

    QVector<int> keys = device->configurationKeys();
    QCOMPARE(keys.size(), 1);
    QVERIFY(keys[0] == QCanBusDevice::ErrorFilterKey);

    QCOMPARE(value.value<QCanBusFrame::FrameErrors>(),
             QCanBusFrame::LostArbitrationError | QCanBusFrame::BusError);

    device->setConfigurationParameter(QCanBusDevice::ErrorFilterKey, QVariant());
    QVERIFY(device->configurationKeys().isEmpty());
}

void tst_QCanBusDevice::write()
{
    QSignalSpy spy(device, SIGNAL(written()));
    QSignalSpy stateSpy(device,
                        SIGNAL(stateChanged(QCanBusDevice::CanBusDeviceState)));

    QCanBusFrame frame;
    frame.setPayload(QByteArray("testData"));

    device->disconnectDevice();
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::UnconnectedState,
                             5000);
    QCOMPARE(stateSpy.count(), 2);
    QCOMPARE(stateSpy[0].at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ClosingState);
    QCOMPARE(stateSpy[1].at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::UnconnectedState);
    stateSpy.clear();
    QVERIFY(stateSpy.isEmpty());

    device->writeFrame(frame);
    QCOMPARE(spy.count(), 0);


    device->connectDevice();
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState,
                             5000);
    QCOMPARE(stateSpy.count(), 2);
    QCOMPARE(stateSpy[0].at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectingState);
    QCOMPARE(stateSpy[1].at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectedState);

    device->writeFrame(frame);
    QCOMPARE(spy.count(), 1);
}

void tst_QCanBusDevice::read()
{
    QSignalSpy stateSpy(device,
                        SIGNAL(stateChanged(QCanBusDevice::CanBusDeviceState)));

    device->disconnectDevice();
    QCOMPARE(device->state(), QCanBusDevice::UnconnectedState);
    stateSpy.clear();

    const QCanBusFrame frame1 = device->readFrame();

    QVERIFY(device->connectDevice());
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState,
                             5000);
    QCOMPARE(stateSpy.count(), 2);
    QCOMPARE(stateSpy[0].at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectingState);
    QCOMPARE(stateSpy[1].at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectedState);

    const QCanBusFrame frame2 = device->readFrame();
    QVERIFY(!frame1.frameId());
    QVERIFY(!frame1.isValid());
    QVERIFY(frame2.frameId());
    QVERIFY(frame2.isValid());
}

void tst_QCanBusDevice::error()
{
    QSignalSpy spy(device, SIGNAL(errorOccurred(QCanBusDevice::CanBusError)));
    QString testString(QStringLiteral("testString"));

    tst_Backend *backend = qobject_cast<tst_Backend *>(device);
    QVERIFY(backend);

    //ReadError
    backend->emulateError(testString+QString::fromLatin1("a"),
                         QCanBusDevice::ReadError);
    QCOMPARE(testString+QString::fromLatin1("a"), device->errorString());
    QVERIFY(device->error() == 1);
    QCOMPARE(spy.count(), 1);

    //WriteError
    backend->emulateError(testString+QString::fromLatin1("b"),
                         QCanBusDevice::WriteError);
    QCOMPARE(testString+QString::fromLatin1("b"), device->errorString());
    QVERIFY(device->error() == 2);
    QCOMPARE(spy.count(), 2);

    //ConnectionError
    backend->emulateError(testString+QString::fromLatin1("c"),
                         QCanBusDevice::ConnectionError);
    QCOMPARE(testString+QString::fromLatin1("c"), device->errorString());
    QVERIFY(device->error() == 3);
    QCOMPARE(spy.count(), 3);

    //ConfigurationError
    backend->emulateError(testString+QString::fromLatin1("d"),
                         QCanBusDevice::ConfigurationError);
    QCOMPARE(testString+QString::fromLatin1("d"), device->errorString());
    QVERIFY(device->error() == 4);
    QCOMPARE(spy.count(), 4);

    //UnknownError
    backend->emulateError(testString+QString::fromLatin1("e"),
                          QCanBusDevice::UnknownError);
    QCOMPARE(testString+QString::fromLatin1("e"), device->errorString());
    QVERIFY(device->error() == 5);
    QCOMPARE(spy.count(), 5);
}

void tst_QCanBusDevice::cleanupTestCase()
{
    device->disconnectDevice();
    QCOMPARE(device->state(), QCanBusDevice::UnconnectedState);
    QCanBusFrame frame = device->readFrame();
    QVERIFY(!frame.frameId());
}

QTEST_MAIN(tst_QCanBusDevice)

#include "tst_qcanbusdevice.moc"
