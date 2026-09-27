// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusframe.h>

#include <QtCore/qpointer.h>
#include <QtCore/qscopeguard.h>
#include <QtCore/qtimer.h>
#include <QtCore/QElapsedTimer>
#include <QtCore/QtPlugin>
#include <QtTest/qsignalspy.h>
#include <QtTest/qtest.h>

#include <memory>

using namespace Qt::StringLiterals;

Q_DECLARE_METATYPE(QCanBusDevice::Filter)

class tst_Backend : public QCanBusDevice
{
    Q_OBJECT
public:
    tst_Backend()
    {
        referenceFrame.setFrameId(5);
        referenceFrame.setPayload(QByteArray("FOOBAR"));
        referenceFrame.setTimeStamp({ 22, 23 });
        referenceFrame.setExtendedFrameFormat(1);
    }

    bool triggerNewFrame()
    {
        if (state() != QCanBusDevice::ConnectedState)
            return false;

        // the line below triggers the framesReceived() signal
        enqueueReceivedFrames({referenceFrame});

        return true;
    }

    bool triggerNewFrames(qsizetype count)
    {
        if (state() != QCanBusDevice::ConnectedState)
            return false;

        enqueueReceivedFrames(QList<QCanBusFrame>(count, referenceFrame));
        return true;
    }

    void triggerFrames(const QList<QCanBusFrame> &frames)
    {
        enqueueReceivedFrames(frames);
    }

    bool open() override
    {
        if (firstOpen) {
            firstOpen = false;
            return false;
        }
        setState(QCanBusDevice::ConnectedState);
        return true;
    }

    void close() override
    {
        setState(QCanBusDevice::UnconnectedState);
    }

    bool writeFrame(const QCanBusFrame &data) override
    {
        if (state() != QCanBusDevice::ConnectedState) {
            setError(QStringLiteral("Cannot write frame as device is not connected"),
                     QCanBusDevice::OperationError);
            return false;
        }

        if (writeBufferUsed) {
            enqueueOutgoingFrame(data);
            QTimer::singleShot(2000, this, [this](){ triggerDelayedWrites(); });
        } else {
            emit framesWritten(1);
        }
        return true;
    }

    void emulateError(const QString &text, QCanBusDevice::CanBusError e)
    {
        setError(text, e);
    }

    QString interpretErrorFrame(const QCanBusFrame &/*errorFrame*/) override
    {
        return QString();
    }

    bool isWriteBuffered() const { return writeBufferUsed; }
    void setWriteBuffered(bool isBuffered)
    {
        // allows switching between buffered and unbuffered write mode
        writeBufferUsed = isBuffered;
    }

    QCanBusDeviceInfo deviceInfo() const override
    {
        return createDeviceInfo(
            u"plugin"_s,
            u"name"_s,
            u"serial number"_s,
            u"description"_s,
            u"alias"_s,
            100,    //channel
            true,   // virtual
            true    // flexible data rate
        );
    }

public slots:
    void triggerDelayedWrites()
    {
        if (framesToWrite() == 0)
            return;

        dequeueOutgoingFrame();
        emit framesWritten(1);

        if (framesToWrite() > 0)
            QTimer::singleShot(2000, this, [this](){ triggerDelayedWrites(); });
    }

private:
    QCanBusFrame referenceFrame;
    bool firstOpen = true;
    bool writeBufferUsed = true;
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
    void readAll();
    void receiveQueueLimit();
    void receiveQueueOverflowThrottling();
    void receiveQueueOverflowRecovery_data();
    void receiveQueueOverflowRecovery();
    void waitForFramesReceivedWithQueueOverflow_data();
    void waitForFramesReceivedWithQueueOverflow();
    void reentrantQueueOverflow();
    void deletionDuringQueueOverflow_data();
    void deletionDuringQueueOverflow();
    void clearInputBuffer();
    void clearOutputBuffer();
    void error();
    void cleanupTestCase();
    void tst_filtering();
    void filterEqual_data();
    void filterEqual();
    void tst_bufferingAttribute();

    void tst_waitForFramesReceived();
    void tst_waitForFramesWritten();

    void tst_deviceInfo();
private:
    std::unique_ptr<tst_Backend> device;
};

tst_QCanBusDevice::tst_QCanBusDevice()
{
    qRegisterMetaType<QCanBusDevice::CanBusDeviceState>();
    qRegisterMetaType<QCanBusDevice::CanBusError>();
    qRegisterMetaType<QCanBusDevice::Filter>();
}

void tst_QCanBusDevice::initTestCase()
{
    device.reset(new tst_Backend());
    QVERIFY(device);

    QSignalSpy stateSpy(device.get(), &QCanBusDevice::stateChanged);

    QVERIFY(!device->connectDevice()); // first connect triggered to fail
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QVERIFY(device->connectDevice());
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState, 5000);
    QCOMPARE(stateSpy.size(), 4);
    QCOMPARE(stateSpy.at(0).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectingState);
    QCOMPARE(stateSpy.at(1).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::UnconnectedState);
    QCOMPARE(stateSpy.at(2).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectingState);
    QCOMPARE(stateSpy.at(3).at(0).value<QCanBusDevice::CanBusDeviceState>(),
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

    QList<QCanBusDevice::ConfigurationKey> keys = device->configurationKeys();
    QCOMPARE(keys.size(), 1);
    QVERIFY(keys.at(0) == QCanBusDevice::ErrorFilterKey);

    QCOMPARE(value.value<QCanBusFrame::FrameErrors>(),
             QCanBusFrame::LostArbitrationError | QCanBusFrame::BusError);

    device->setConfigurationParameter(QCanBusDevice::ErrorFilterKey, QVariant());
    QVERIFY(device->configurationKeys().isEmpty());
}

void tst_QCanBusDevice::write()
{
    // we assume unbuffered writing in this function
    device->setWriteBuffered(false);
    QVERIFY(!device->isWriteBuffered());

    QSignalSpy spy(device.get(), &QCanBusDevice::framesWritten);
    QSignalSpy stateSpy(device.get(), &QCanBusDevice::stateChanged);

    QCanBusFrame frame;
    frame.setPayload(QByteArray("testData"));

    device->disconnectDevice();
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::UnconnectedState, 5000);
    QCOMPARE(stateSpy.size(), 2);
    QCOMPARE(stateSpy.at(0).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ClosingState);
    QCOMPARE(stateSpy.at(1).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::UnconnectedState);
    stateSpy.clear();
    QVERIFY(stateSpy.isEmpty());

    QVERIFY(!device->writeFrame(frame));
    QCOMPARE(device->error(), QCanBusDevice::OperationError);
    QCOMPARE(spy.size(), 0);

    device->connectDevice();
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState, 5000);
    QCOMPARE(stateSpy.size(), 2);
    QCOMPARE(stateSpy.at(0).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectingState);
    QCOMPARE(stateSpy.at(1).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectedState);

    QVERIFY(device->writeFrame(frame));
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QCOMPARE(spy.size(), 1);
}

void tst_QCanBusDevice::read()
{
    QSignalSpy stateSpy(device.get(), &QCanBusDevice::stateChanged);

    device->disconnectDevice();
    QCOMPARE(device->state(), QCanBusDevice::UnconnectedState);
    stateSpy.clear();

    const QCanBusFrame frame1 = device->readFrame();
    QCOMPARE(device->error(), QCanBusDevice::OperationError);

    QVERIFY(device->connectDevice());
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState, 5000);
    QCOMPARE(stateSpy.size(), 2);
    QCOMPARE(stateSpy.at(0).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectingState);
    QCOMPARE(stateSpy.at(1).at(0).value<QCanBusDevice::CanBusDeviceState>(),
             QCanBusDevice::ConnectedState);

    device->triggerNewFrame();
    const QCanBusFrame frame2 = device->readFrame();
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QVERIFY(!frame1.frameId());
    QVERIFY(!frame1.isValid());
    QVERIFY(frame2.frameId());
    QVERIFY(frame2.isValid());
}

void tst_QCanBusDevice::readAll()
{
    enum { FrameNumber = 10 };
    device->disconnectDevice();
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::UnconnectedState, 5000);

    const QList<QCanBusFrame> empty = device->readAllFrames();
    QCOMPARE(device->error(), QCanBusDevice::OperationError);
    QVERIFY(empty.isEmpty());

    QVERIFY(device->connectDevice());
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState, 5000);

    for (int i = 0; i < FrameNumber; ++i)
        device->triggerNewFrame();

    const QList<QCanBusFrame> frames = device->readAllFrames();
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QCOMPARE(FrameNumber, frames.size());
    QVERIFY(!device->framesAvailable());
}

void tst_QCanBusDevice::receiveQueueLimit()
{
    const auto cleanup = qScopeGuard([this] {
        device->setReceiveQueueLimit(0);
        device->clear(QCanBusDevice::Input);
    });
    QCOMPARE(device->receiveQueueLimit(), 0);

    QSignalSpy errorSpy(device.get(), &QCanBusDevice::errorOccurred);
    QSignalSpy receivedSpy(device.get(), &QCanBusDevice::framesReceived);
    device->setReceiveQueueLimit(2);
    QCOMPARE(device->receiveQueueLimit(), 2);

    device->setReceiveQueueLimit(-1);
    QCOMPARE(device->receiveQueueLimit(), 2);

    device->clear(QCanBusDevice::Input);
    const QCanBusFrame first(11, QByteArray(1, 'a'));
    const QCanBusFrame second(22, QByteArray(1, 'b'));
    const QCanBusFrame third(33, QByteArray(1, 'c'));
    device->triggerFrames({first, second, third});
    QCOMPARE(device->framesAvailable(), 2);
    QCOMPARE(errorSpy.size(), 1);
    QCOMPARE(errorSpy.constFirst().constFirst().value<QCanBusDevice::CanBusError>(),
             QCanBusDevice::ReceiveQueueOverflowError);
    QCOMPARE(receivedSpy.size(), 1);
    const auto firstBatch = device->readAllFrames();
    QCOMPARE(firstBatch.size(), 2);
    QCOMPARE(firstBatch.at(0).frameId(), QCanBusFrame::FrameId(11));
    QCOMPARE(firstBatch.at(1).frameId(), QCanBusFrame::FrameId(22));
    QVERIFY(device->readAllFrames().isEmpty());

    // A burst larger than the limit still delivers the oldest frames.
    QVERIFY(device->triggerNewFrames(3));
    QCOMPARE(device->framesAvailable(), 2);
    QCOMPARE(receivedSpy.size(), 2);

    // More arrivals while full neither enqueue nor restart the episode.
    const qsizetype fullQueueReports = errorSpy.size();
    device->setReceiveQueueLimit(2);
    QVERIFY(device->triggerNewFrame());
    QCOMPARE(errorSpy.size(), fullQueueReports);
    QCOMPARE(receivedSpy.size(), 2);

    QVERIFY(device->readFrame().isValid());
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QVERIFY(device->triggerNewFrame());
    QCOMPARE(device->framesAvailable(), 2);
    QVERIFY(device->triggerNewFrame());

    // Raising the limit creates room for another frame.
    device->setReceiveQueueLimit(3);
    QVERIFY(device->triggerNewFrame());
    QCOMPARE(device->framesAvailable(), 3);
    QVERIFY(device->triggerNewFrame());

    // A full queue stays bounded and does not report the same episode again,
    // even after the rate limit expires.
    const qsizetype ongoingEpisodeReports = errorSpy.size();
    QElapsedTimer throttleTimer;
    throttleTimer.start();
    QTRY_VERIFY_WITH_TIMEOUT(throttleTimer.elapsed() >= 1100, 1500);
    const qsizetype receivedSignals = receivedSpy.size();
    device->triggerFrames({first, second, third});
    device->triggerFrames({});
    QCOMPARE(device->framesAvailable(), 3);
    QCOMPARE(errorSpy.size(), ongoingEpisodeReports);
    QCOMPARE(receivedSpy.size(), receivedSignals);

    // Disabling the limit preserves the queue and accepts more frames.
    device->setReceiveQueueLimit(0);
    QCOMPARE(device->framesAvailable(), 3);
    device->triggerFrames({first});
    QCOMPARE(device->framesAvailable(), 4);
    QCOMPARE(errorSpy.size(), ongoingEpisodeReports);

    // Lowering the limit while full keeps the oldest queued frames.
    device->clear(QCanBusDevice::Input);
    errorSpy.clear();
    device->triggerFrames({first, second, third});
    QCOMPARE(device->framesAvailable(), 3);
    device->setReceiveQueueLimit(2);
    QCOMPARE(device->framesAvailable(), 2);
    QCOMPARE(errorSpy.size(), 1);
    QCOMPARE(errorSpy.constFirst().constFirst().value<QCanBusDevice::CanBusError>(),
             QCanBusDevice::ReceiveQueueOverflowError);
    const auto retained = device->readAllFrames();
    QCOMPARE(retained.size(), 2);
    QCOMPARE(retained.at(0).frameId(), QCanBusFrame::FrameId(11));
    QCOMPARE(retained.at(1).frameId(), QCanBusFrame::FrameId(22));

    // A partially fitting batch preserves the oldest frames.
    device->triggerFrames({first});
    device->triggerFrames({second, third});
    QCOMPARE(device->framesAvailable(), 2);
    const auto partialBatch = device->readAllFrames();
    QCOMPARE(partialBatch.at(0).frameId(), QCanBusFrame::FrameId(11));
    QCOMPARE(partialBatch.at(1).frameId(), QCanBusFrame::FrameId(22));

    device->setReceiveQueueLimit(1);
    device->triggerFrames({first, second, third});
    QCOMPARE(device->framesAvailable(), 1);
    QCOMPARE(device->readFrame().frameId(), QCanBusFrame::FrameId(11));
    device->setReceiveQueueLimit(0);
    QCOMPARE(device->receiveQueueLimit(), 0);
    QCOMPARE(device->framesAvailable(), 0);

    const qsizetype previousReports = errorSpy.size();
    throttleTimer.restart();
    QTRY_VERIFY_WITH_TIMEOUT(throttleTimer.elapsed() >= 1100, 1500);
    device->setReceiveQueueLimit(1);
    device->triggerFrames({first, second});
    QCOMPARE(errorSpy.size(), previousReports + 1);
}

void tst_QCanBusDevice::receiveQueueOverflowThrottling()
{
    // Only assert suppression when both episodes finished within the throttle
    // interval. Retry if scheduling delayed the test past that interval.
    for (int attempt = 0; attempt < 3; ++attempt) {
        tst_Backend local;
        QVERIFY(!local.connectDevice()); // first connection intentionally fails
        QVERIFY(local.connectDevice());
        local.setReceiveQueueLimit(1);
        QSignalSpy errorSpy(&local, &QCanBusDevice::errorOccurred);
        const QList<QCanBusFrame> frames = {QCanBusFrame(1, {}), QCanBusFrame(2, {})};

        QElapsedTimer timer;
        timer.start();
        local.triggerFrames(frames);
        QCOMPARE(errorSpy.size(), 1);
        QCOMPARE(local.readAllFrames().size(), 1);
        local.triggerFrames(frames);
        QCOMPARE(local.framesAvailable(), 1);
        if (timer.elapsed() >= 1000)
            continue;
        QCOMPARE(errorSpy.size(), 1);
        return;
    }
    QSKIP("Scheduling delayed every attempt past the overflow throttle interval.");
}

enum class RecoveryType { ReadFrame, ReadAll, ClearInput, RaiseLimit };

void tst_QCanBusDevice::receiveQueueOverflowRecovery_data()
{
    QTest::addColumn<RecoveryType>("recovery");
    QTest::newRow("read-frame") << RecoveryType::ReadFrame;
    QTest::newRow("read-all") << RecoveryType::ReadAll;
    QTest::newRow("clear-input") << RecoveryType::ClearInput;
    QTest::newRow("raise-limit") << RecoveryType::RaiseLimit;
}

void tst_QCanBusDevice::receiveQueueOverflowRecovery()
{
    QFETCH(RecoveryType, recovery);

    tst_Backend local;
    QVERIFY(!local.connectDevice()); // first connection intentionally fails
    QVERIFY(local.connectDevice());
    local.setReceiveQueueLimit(2);
    QSignalSpy errorSpy(&local, &QCanBusDevice::errorOccurred);

    QVERIFY(local.triggerNewFrames(3));
    QCOMPARE(local.framesAvailable(), 2);
    QCOMPARE(errorSpy.size(), 1);

    QElapsedTimer throttleTimer;
    throttleTimer.start();
    QTRY_VERIFY_WITH_TIMEOUT(throttleTimer.elapsed() >= 1100, 1500);

    switch (recovery) {
    case RecoveryType::ReadFrame:
        QVERIFY(local.readFrame().isValid());
        break;
    case RecoveryType::ReadAll:
        QCOMPARE(local.readAllFrames().size(), 2);
        break;
    case RecoveryType::ClearInput:
        local.clear(QCanBusDevice::Input);
        break;
    case RecoveryType::RaiseLimit:
        local.setReceiveQueueLimit(3);
        break;
    }

    QVERIFY(local.triggerNewFrames(3));
    QCOMPARE(local.framesAvailable(), local.receiveQueueLimit());
    QCOMPARE(errorSpy.size(), 2);
    QCOMPARE(local.error(), QCanBusDevice::ReceiveQueueOverflowError);
}

void tst_QCanBusDevice::waitForFramesReceivedWithQueueOverflow_data()
{
    QTest::addColumn<bool>("queueFull");
    QTest::newRow("partial-batch") << false;
    QTest::newRow("full-queue") << true;
}

void tst_QCanBusDevice::waitForFramesReceivedWithQueueOverflow()
{
    QFETCH(bool, queueFull);

    tst_Backend local;
    QVERIFY(!local.connectDevice()); // first connection intentionally fails
    QVERIFY(local.connectDevice());
    local.setReceiveQueueLimit(1);
    if (queueFull)
        QVERIFY(local.triggerNewFrame());
    QSignalSpy errorSpy(&local, &QCanBusDevice::errorOccurred);
    QSignalSpy receivedSpy(&local, &QCanBusDevice::framesReceived);

    QTimer::singleShot(0, &local, [&local] { local.triggerNewFrames(2); });
    QVERIFY(!local.waitForFramesReceived(1000));
    QCOMPARE(local.error(), QCanBusDevice::ReceiveQueueOverflowError);
    QCOMPARE(errorSpy.size(), 1);
    QCOMPARE(receivedSpy.size(), queueFull ? 0 : 1);
    QCOMPARE(local.readAllFrames().size(), 1);

    // A subsequent batch that fits still wakes a blocking reader successfully.
    QTimer::singleShot(0, &local, [&local] { local.triggerNewFrame(); });
    QVERIFY(local.waitForFramesReceived(1000));
    QCOMPARE(local.error(), QCanBusDevice::NoError);
    QCOMPARE(local.framesAvailable(), 1);
    QCOMPARE(errorSpy.size(), 1);
}

void tst_QCanBusDevice::reentrantQueueOverflow()
{
    tst_Backend local;
    QVERIFY(!local.connectDevice());
    QVERIFY(local.connectDevice());
    local.setReceiveQueueLimit(1);
    QSignalSpy errorSpy(&local, &QCanBusDevice::errorOccurred);

    bool nested = false;
    QElapsedTimer nestedReportTimer;
    connect(&local, &QCanBusDevice::framesReceived, this, [&] {
        if (nested)
            return;
        nested = true;
        QCOMPARE(local.readAllFrames().size(), 1);
        QElapsedTimer timer;
        timer.start();
        QTRY_VERIFY_WITH_TIMEOUT(timer.elapsed() >= 1100, 1500);
        nestedReportTimer.start();
        QVERIFY(local.triggerNewFrames(2));
    });

    QVERIFY(local.triggerNewFrames(2));
    QCOMPARE(local.framesAvailable(), 1);
    QVERIFY(nestedReportTimer.isValid());
    if (nestedReportTimer.elapsed() >= 1000)
        QSKIP("Scheduling delayed the outer report past the overflow throttle interval.");
    QCOMPARE(errorSpy.size(), 1);
    QCOMPARE(local.error(), QCanBusDevice::ReceiveQueueOverflowError);
}

void tst_QCanBusDevice::deletionDuringQueueOverflow_data()
{
    QTest::addColumn<bool>("lowerLimit");
    QTest::addColumn<bool>("deleteOnError");
    QTest::newRow("enqueue-error") << false << true;
    QTest::newRow("enqueue-received") << false << false;
    QTest::newRow("lower-limit-error") << true << true;
}

void tst_QCanBusDevice::deletionDuringQueueOverflow()
{
    QFETCH(bool, lowerLimit);
    QFETCH(bool, deleteOnError);

    auto local = std::make_unique<tst_Backend>();
    QPointer<tst_Backend> guard(local.get());
    const QList<QCanBusFrame> frames = {QCanBusFrame(1, {}), QCanBusFrame(2, {})};
    if (lowerLimit)
        local->triggerFrames(frames);
    else
        local->setReceiveQueueLimit(1);
    int receivedSignals = 0;
    int errorSignals = 0;
    connect(local.get(), &QCanBusDevice::framesReceived, this, [&] { ++receivedSignals; });
    connect(local.get(), &QCanBusDevice::errorOccurred, this, [&] { ++errorSignals; });
    const auto deleteDevice = [&local] {
        QCOMPARE(local->receiveQueueLimit(), 1);
        QCOMPARE(local->framesAvailable(), 1);
        local->setReceiveQueueLimit(0);
        delete local.release();
    };
    if (deleteOnError)
        connect(local.get(), &QCanBusDevice::errorOccurred, this, deleteDevice);
    else
        connect(local.get(), &QCanBusDevice::framesReceived, this, deleteDevice);

    if (lowerLimit)
        local->setReceiveQueueLimit(1);
    else
        local->triggerFrames(frames);
    QVERIFY(!guard);
    QCOMPARE(receivedSignals, lowerLimit ? 0 : 1);
    QCOMPARE(errorSignals, deleteOnError ? 1 : 0);
}

void tst_QCanBusDevice::clearInputBuffer()
{
    device->disconnectDevice();
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::UnconnectedState, 5000);

    device->clear(QCanBusDevice::Input);
    QCOMPARE(device->error(), QCanBusDevice::OperationError);

    QVERIFY(device->connectDevice());
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState, 5000);

    device->clear(QCanBusDevice::Input);
    QCOMPARE(device->error(), QCanBusDevice::NoError);

    for (int i = 0; i < 10; ++i)
        device->triggerNewFrame();

    device->clear(QCanBusDevice::Input);
    QCOMPARE(device->error(), QCanBusDevice::NoError);

    QVERIFY(!device->framesAvailable());
}

void tst_QCanBusDevice::clearOutputBuffer()
{
    // this test requires buffered writing
    device->setWriteBuffered(true);
    device->disconnectDevice();
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::UnconnectedState, 5000);

    device->clear(QCanBusDevice::Output);
    QCOMPARE(device->error(), QCanBusDevice::OperationError);

    QVERIFY(device->connectDevice());
    QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState, 5000);

    device->clear(QCanBusDevice::Output);
    QCOMPARE(device->error(), QCanBusDevice::NoError);

    // first test buffered writing, frames will be written after some delay
    QSignalSpy spy(device.get(), &QCanBusDevice::framesWritten);
    for (int i = 0; i < 10; ++i)
        device->writeFrame(QCanBusFrame(0x123, "output"));
    QTRY_VERIFY_WITH_TIMEOUT(spy.size() == 10, 5000);

    // now test clearing the buffer before the frames are actually written
    spy.clear();
    for (int i = 0; i < 10; ++i)
        device->writeFrame(QCanBusFrame(0x123, "output"));

    device->clear(QCanBusDevice::Output);
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QTRY_VERIFY_WITH_TIMEOUT(spy.size() == 0, 5000);
}

void tst_QCanBusDevice::error()
{
    QSignalSpy spy(device.get(), &QCanBusDevice::errorOccurred);
    QString testString(QStringLiteral("testString"));

    auto backend = qobject_cast<tst_Backend *>(device.get());
    QVERIFY(backend);

    // NoError
    QVERIFY(device->errorString().isEmpty());

    // ReadError
    backend->emulateError(testString + QStringLiteral("a"), QCanBusDevice::ReadError);
    QCOMPARE(testString + QStringLiteral("a"), device->errorString());
    QCOMPARE(device->error(), 1);
    QCOMPARE(spy.size(), 1);

    // WriteError
    backend->emulateError(testString + QStringLiteral("b"), QCanBusDevice::WriteError);
    QCOMPARE(testString + QStringLiteral("b"), device->errorString());
    QCOMPARE(device->error(), 2);
    QCOMPARE(spy.size(), 2);

    // ConnectionError
    backend->emulateError(testString + QStringLiteral("c"), QCanBusDevice::ConnectionError);
    QCOMPARE(testString + QStringLiteral("c"), device->errorString());
    QCOMPARE(device->error(), 3);
    QCOMPARE(spy.size(), 3);

    // ConfigurationError
    backend->emulateError(testString + QStringLiteral("d"), QCanBusDevice::ConfigurationError);
    QCOMPARE(testString + QStringLiteral("d"), device->errorString());
    QCOMPARE(device->error(), 4);
    QCOMPARE(spy.size(), 4);

    // UnknownError
    backend->emulateError(testString + QStringLiteral("e"), QCanBusDevice::UnknownError);
    QCOMPARE(testString + QStringLiteral("e"), device->errorString());
    QCOMPARE(device->error(), 5);
    QCOMPARE(spy.size(), 5);
}

void tst_QCanBusDevice::cleanupTestCase()
{
    device->disconnectDevice();
    QCOMPARE(device->state(), QCanBusDevice::UnconnectedState);
    QCanBusFrame frame = device->readFrame();
    QVERIFY(!frame.frameId());
}

void tst_QCanBusDevice::tst_filtering()
{
    QCanBusDevice::Filter defaultFilter;
    QCOMPARE(defaultFilter.frameId, 0x0u);
    QCOMPARE(defaultFilter.frameIdMask, 0x0u);
    QCOMPARE(defaultFilter.type, QCanBusFrame::InvalidFrame);
    QCOMPARE(defaultFilter.format, QCanBusDevice::Filter::MatchBaseAndExtendedFormat);

    QList<QCanBusDevice::Filter> filters;
    QCanBusDevice::Filter f;
    f.frameId = 0x1;
    f.type = QCanBusFrame::DataFrame;
    f.frameIdMask = 0xFF;
    f.format = QCanBusDevice::Filter::MatchBaseAndExtendedFormat;
    filters.append(f);

    f.frameId = 0x2;
    f.type = QCanBusFrame::RemoteRequestFrame;
    f.frameIdMask = 0x0;
    f.format = QCanBusDevice::Filter::MatchBaseFormat;
    filters.append(f);

    const QVariant wrapper = QVariant::fromValue(filters);
    const auto newFilter = wrapper.value<QList<QCanBusDevice::Filter> >();
    QCOMPARE(newFilter.size(), 2);

    QCOMPARE(newFilter.at(0).type, QCanBusFrame::DataFrame);
    QCOMPARE(newFilter.at(0).frameId, 0x1u);
    QCOMPARE(newFilter.at(0).frameIdMask, 0xFFu);
    QVERIFY(newFilter.at(0).format & QCanBusDevice::Filter::MatchBaseAndExtendedFormat);
    QVERIFY(newFilter.at(0).format & QCanBusDevice::Filter::MatchBaseFormat);
    QVERIFY(newFilter.at(0).format & QCanBusDevice::Filter::MatchExtendedFormat);

    QCOMPARE(newFilter.at(1).type, QCanBusFrame::RemoteRequestFrame);
    QCOMPARE(newFilter.at(1).frameId, 0x2u);
    QCOMPARE(newFilter.at(1).frameIdMask, 0x0u);
    QVERIFY((newFilter.at(1).format & QCanBusDevice::Filter::MatchBaseAndExtendedFormat)
              != QCanBusDevice::Filter::MatchBaseAndExtendedFormat);
    QVERIFY(newFilter.at(1).format & QCanBusDevice::Filter::MatchBaseFormat);
    QVERIFY(!(newFilter.at(1).format & QCanBusDevice::Filter::MatchExtendedFormat));
}

void tst_QCanBusDevice::filterEqual_data()
{
    using Filter = QCanBusDevice::Filter;
    using Frame = QCanBusFrame;

    QTest::addColumn<QCanBusDevice::Filter>("first");
    QTest::addColumn<QCanBusDevice::Filter>("second");
    QTest::addColumn<bool>("isEqual");

    auto filter = [](quint32 frameId, quint32 frameIdMask,
                     Frame::FrameType type,
                     Filter::FormatFilter format) {
        Filter result;
        result.frameId = frameId;
        result.frameIdMask = frameIdMask;
        result.type = type;
        result.format = format;
        return result;
    };

    QTest::newRow("empty-equal")
            << Filter()
            << Filter()
            << true;
    QTest::newRow("empty-default-equal")
            << Filter()
            << filter(0, 0, Frame::InvalidFrame, Filter::MatchBaseAndExtendedFormat)
            << true;
    QTest::newRow("empty-non-default-different")
            << Filter()
            << filter(1, 2, Frame::ErrorFrame, Filter::MatchBaseFormat)
            << false;

    QTest::newRow("frame-id-equal")
            << filter(0x345, 0x800, Frame::RemoteRequestFrame, Filter::MatchBaseFormat)
            << filter(0x345, 0x800, Frame::RemoteRequestFrame, Filter::MatchBaseFormat)
            << true;
    QTest::newRow("frame-id-different")
            << filter(0x345, 0x000, Frame::RemoteRequestFrame, Filter::MatchBaseFormat)
            << filter(0x346, 0x000, Frame::RemoteRequestFrame, Filter::MatchBaseFormat)
            << false;

    QTest::newRow("frame-mask-equal")
            << filter(0x123, 0x7FF, Frame::InvalidFrame, Filter::MatchBaseAndExtendedFormat)
            << filter(0x123, 0x7FF, Frame::InvalidFrame, Filter::MatchBaseAndExtendedFormat)
            << true;
    QTest::newRow("frame-mask-different")
            << filter(0x123, 0x7FF, Frame::InvalidFrame, Filter::MatchBaseAndExtendedFormat)
            << filter(0x123, 0x7FE, Frame::InvalidFrame, Filter::MatchBaseAndExtendedFormat)
            << false;

    QTest::newRow("frame-type-equal")
            << filter(0xFFF, 0xBFF, Frame::DataFrame,    Filter::MatchBaseAndExtendedFormat)
            << filter(0xFFF, 0xBFF, Frame::DataFrame,    Filter::MatchBaseAndExtendedFormat)
            << true;
    QTest::newRow("frame-type-different")
            << filter(0xFFF, 0xBFF, Frame::DataFrame,    Filter::MatchBaseAndExtendedFormat)
            << filter(0xFFF, 0xBFF, Frame::InvalidFrame, Filter::MatchBaseAndExtendedFormat)
            << false;

    QTest::newRow("filter-equal")
            << filter(0xFFF, 0xBFF, Frame::ErrorFrame,    Filter::MatchExtendedFormat)
            << filter(0xFFF, 0xBFF, Frame::ErrorFrame,    Filter::MatchExtendedFormat)
            << true;
    QTest::newRow("filter-different")
            << filter(0xFFF, 0xBFF, Frame::ErrorFrame,    Filter::MatchExtendedFormat)
            << filter(0xFFF, 0xBFF, Frame::ErrorFrame,    Filter::MatchBaseAndExtendedFormat)
            << false;
}

void tst_QCanBusDevice::filterEqual()
{
    QFETCH(QCanBusDevice::Filter, first);
    QFETCH(QCanBusDevice::Filter, second);
    QFETCH(bool, isEqual);

    if (isEqual) {
        QCOMPARE(first, second);
    } else {
        QVERIFY(first != second);
    }
}

void tst_QCanBusDevice::tst_bufferingAttribute()
{
    std::unique_ptr<tst_Backend> canDevice(new tst_Backend);
    QVERIFY(canDevice != nullptr);
    // by default buffered set to true
    QVERIFY(canDevice->isWriteBuffered());

    canDevice->setWriteBuffered(false);
    QVERIFY(!canDevice->isWriteBuffered());
    canDevice->setWriteBuffered(true);
    QVERIFY(canDevice->isWriteBuffered());
}

void tst_QCanBusDevice::tst_waitForFramesReceived()
{
    device->disconnectDevice();
    QVERIFY(!device->waitForFramesReceived(100));
    QCOMPARE(device->error(), QCanBusDevice::OperationError);

    if (device->state() != QCanBusDevice::ConnectedState) {
        QVERIFY(device->connectDevice());
        QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState, 5000);
    }

    QVERIFY(!device->framesAvailable());
    QVERIFY(device->triggerNewFrame());
    QVERIFY(device->framesAvailable());

    // frame is already available, but no new frame comes in
    // while function blocks, therefore times out
    QVERIFY(!device->waitForFramesReceived(2000));
    QCOMPARE(device->error(), QCanBusDevice::TimeoutError);

    QCanBusFrame frame = device->readFrame();
    QVERIFY(frame.isValid());
    QCOMPARE(frame.payload(), QByteArray("FOOBAR"));
    QVERIFY(!device->framesAvailable());

    QElapsedTimer elapsed;
    elapsed.start();
    // no pending frame (should trigger active wait & timeout)
    QVERIFY(!device->waitForFramesReceived(5000));
    QVERIFY(elapsed.hasExpired(4000)); // should have caused time elapse
    QCOMPARE(device->error(), QCanBusDevice::TimeoutError);

    QTimer::singleShot(2000, [&]() { device->triggerNewFrame(); });
    elapsed.restart();
    // frame will be inserted after 2s
    QVERIFY(device->waitForFramesReceived(8000));
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QVERIFY(!elapsed.hasExpired(8000));

    frame = device->readFrame();
    QVERIFY(frame.isValid());
    QCOMPARE(frame.payload(), QByteArray("FOOBAR"));
    QVERIFY(!device->framesAvailable());

    QTimer::singleShot(2000, [&]() {
        device->emulateError(QStringLiteral("TriggerError"), QCanBusDevice::ReadError);
    });
    elapsed.restart();
    // error will be inserted after 2s
    QVERIFY(!device->waitForFramesReceived(8000));
    QVERIFY(!elapsed.hasExpired(8000));
    QCOMPARE(device->errorString(), QStringLiteral("TriggerError"));
    QCOMPARE(device->error(), QCanBusDevice::ReadError);

    // test recursive calling of waitForFramesReceived() behavior
    int handleCounter = 0;
    QTimer::singleShot(1000, [&]() {
        device->triggerNewFrame();
        device->triggerNewFrame();
    });
    QTimer::singleShot(2000, [&]() { device->triggerNewFrame(); });
    QObject::connect(device.get(), &QCanBusDevice::framesReceived, this, [this, &handleCounter]() {
        handleCounter++;
        // this should trigger a recursion which we want to catch
        QVERIFY(!device->waitForFramesReceived(5000));
        // Only the first two frames create a recursion, as the outer
        // waitForFramesReceived() will immediately exit once at least
        // one frame was received. Therefore the third frame here leads
        // to TimeoutError, as no further frame is received.
        if (handleCounter < 3) {
            QCOMPARE(device->error(), QCanBusDevice::OperationError);
        } else {
            QCOMPARE(device->error(), QCanBusDevice::TimeoutError);
        }
    });
    QVERIFY(device->waitForFramesReceived(8000));
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QTRY_COMPARE_WITH_TIMEOUT(handleCounter, 3, 5000);
}

void tst_QCanBusDevice::tst_waitForFramesWritten()
{
    device->disconnectDevice();
    QVERIFY(!device->waitForFramesWritten(100));
    QCOMPARE(device->error(), QCanBusDevice::OperationError);

    if (device->state() != QCanBusDevice::ConnectedState) {
        QVERIFY(!device->waitForFramesWritten(100));
        QCOMPARE(device->error(), QCanBusDevice::OperationError);

        QVERIFY(device->connectDevice());
        QTRY_VERIFY_WITH_TIMEOUT(device->state() == QCanBusDevice::ConnectedState, 5000);
    }

    device->setWriteBuffered(false);
    QVERIFY(!device->waitForFramesWritten(1000)); // no buffer, waiting not possible
    QCOMPARE(device->error(), QCanBusDevice::NoError);

    device->setWriteBuffered(true);

    QVERIFY(device->framesToWrite() == 0);
    QVERIFY(!device->waitForFramesWritten(1000)); // nothing in buffer, nothing to wait for
    QCOMPARE(device->error(), QCanBusDevice::NoError);

    QCanBusFrame frame;
    frame.setPayload(QByteArray("testData"));

    // test error case
    QTimer::singleShot(500, [&]() {
        device->emulateError(QStringLiteral("TriggerWriteError"), QCanBusDevice::WriteError);
    });
    device->writeFrame(frame);
    QElapsedTimer elapsed;
    elapsed.start();

    // error will be triggered
    QVERIFY(!device->waitForFramesWritten(8000));
    QVERIFY(!elapsed.hasExpired(8000));
    QCOMPARE(device->errorString(), QStringLiteral("TriggerWriteError"));
    QCOMPARE(device->error(), QCanBusDevice::WriteError);

    // flush remaining frames out to reset the test
    QTRY_VERIFY_WITH_TIMEOUT(device->framesToWrite() == 0, 10000);

    // test timeout
    device->writeFrame(frame);
    QVERIFY(!device->waitForFramesWritten(500));
    QCOMPARE(device->error(), QCanBusDevice::TimeoutError);
    QVERIFY(elapsed.hasExpired(500));

    // flush remaining frames out to reset the test
    QTRY_VERIFY_WITH_TIMEOUT(device->framesToWrite() == 0, 10000);

    device->writeFrame(frame);
    device->writeFrame(frame);
    elapsed.restart();
    QVERIFY(device->waitForFramesWritten(8000));
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QVERIFY(!elapsed.hasExpired(8000));

    // flush remaining frames out to reset the test
    QTRY_VERIFY_WITH_TIMEOUT(device->framesToWrite() == 0, 10000);

    // test recursive calling of waitForFramesWritten() behavior
    int handleCounter = 0;
    device->writeFrame(frame);
    QTimer::singleShot(1000, [&]() { device->writeFrame(frame); });
    QTimer::singleShot(2000, [&]() { device->writeFrame(frame); });
    QObject::connect(device.get(), &QCanBusDevice::framesWritten, this, [this, &handleCounter]() {
        handleCounter++;
        // this should trigger a recursion which we want to catch
        QVERIFY(!device->waitForFramesWritten(5000));
        QCOMPARE(device->error(), QCanBusDevice::OperationError);
    });
    QVERIFY(device->waitForFramesWritten(8000));
    QCOMPARE(device->error(), QCanBusDevice::NoError);
    QTRY_COMPARE_WITH_TIMEOUT(handleCounter, 3, 5000);

    device->setWriteBuffered(false);
}

void tst_QCanBusDevice::tst_deviceInfo()
{
    std::unique_ptr<tst_Backend> canDevice(new tst_Backend);
    QVERIFY(canDevice != nullptr);

    auto info = canDevice->deviceInfo();
    QCOMPARE(info.plugin(), u"plugin"_s);
    QCOMPARE(info.name(), u"name"_s);
    QCOMPARE(info.serialNumber(), u"serial number"_s);
    QCOMPARE(info.description(), u"description"_s);
    QCOMPARE(info.alias(), u"alias"_s);
    QCOMPARE(info.channel(), 100);
    QCOMPARE(info.hasFlexibleDataRate(), true);
    QCOMPARE(info.isVirtual(), true);
}

QTEST_MAIN(tst_QCanBusDevice)
Q_IMPORT_PLUGIN(TestCanBusPlugin)

#include "tst_qcanbusdevice.moc"
