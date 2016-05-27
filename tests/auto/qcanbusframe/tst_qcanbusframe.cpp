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
#include <QtSerialBus/QCanBusFrame>

class tst_QCanBusFrame : public QObject
{
    Q_OBJECT
public:
    explicit tst_QCanBusFrame();

private slots:
    void constructors();
    void id();
    void payload();
    void timeStamp();

    void tst_isValid_data();
    void tst_isValid();

    void streaming_data();
    void streaming();

    void tst_error();
};

tst_QCanBusFrame::tst_QCanBusFrame()
{
}

void tst_QCanBusFrame::constructors()
{
    QCanBusFrame frame1;
    QCanBusFrame frame2(123, "tst");
    QCanBusFrame frame3(123456, "tst");
    QCanBusFrame::TimeStamp timeStamp1;
    QCanBusFrame::TimeStamp timeStamp2(5, 5);

    QVERIFY(frame1.payload().isEmpty());
    QVERIFY(!frame1.frameId());

    QVERIFY(!frame2.payload().isEmpty());
    QVERIFY(frame2.frameId());

    QVERIFY(!frame3.payload().isEmpty());
    QVERIFY(frame3.frameId());

    QVERIFY(!timeStamp1.seconds());
    QVERIFY(!timeStamp1.microSeconds());

    QVERIFY(timeStamp2.seconds());
    QVERIFY(timeStamp2.microSeconds());

    QVERIFY(frame1.hasExtendedFrameFormat() == false);
    QVERIFY(frame2.hasExtendedFrameFormat() == false);
    QVERIFY(frame3.hasExtendedFrameFormat() == true);

    QCOMPARE(frame1.frameType(), QCanBusFrame::DataFrame);
    QCOMPARE(frame2.frameType(), QCanBusFrame::DataFrame);
    QCOMPARE(frame3.frameType(), QCanBusFrame::DataFrame);
}

void tst_QCanBusFrame::id()
{
    QCanBusFrame frame;
    QVERIFY(!frame.frameId());
    frame.setExtendedFrameFormat(false);
    frame.setFrameId(2047u);
    QCOMPARE(frame.frameId(), 2047u);
    QVERIFY(frame.hasExtendedFrameFormat() == false);
    // id > 2^11 -> extended format
    frame.setExtendedFrameFormat(false);
    frame.setFrameId(2048u);
    QCOMPARE(frame.frameId(), 2048u);
    QVERIFY(frame.hasExtendedFrameFormat() == true);
    // id < 2^11 -> no extended format
    frame.setExtendedFrameFormat(false);
    frame.setFrameId(512u);
    QCOMPARE(frame.frameId(), 512u);
    QVERIFY(frame.hasExtendedFrameFormat() == false);
    // id < 2^11 -> keep extended format
    frame.setExtendedFrameFormat(true);
    frame.setFrameId(512u);
    QCOMPARE(frame.frameId(), 512u);
    QVERIFY(frame.hasExtendedFrameFormat() == true);
}

void tst_QCanBusFrame::payload()
{
    QCanBusFrame frame;
    QVERIFY(frame.payload().isEmpty());
    frame.setPayload("test");
    QCOMPARE(frame.payload().data(), "test");
}

void tst_QCanBusFrame::timeStamp()
{
    QCanBusFrame frame;
    QCanBusFrame::TimeStamp timeStamp = frame.timeStamp();
    QVERIFY(!timeStamp.seconds());
    QVERIFY(!timeStamp.microSeconds());
    timeStamp.setMicroSeconds(5);
    timeStamp.setSeconds(4);
    QCOMPARE(timeStamp.seconds(), 4);
    QCOMPARE(timeStamp.microSeconds(), 5);
}

void tst_QCanBusFrame::tst_isValid_data()
{
    QTest::addColumn<QCanBusFrame::FrameType>("frameType");
    QTest::addColumn<bool>("isValid");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<uint>("id");
    QTest::addColumn<bool>("extended");

    QTest::newRow("invalid frame")
                 << QCanBusFrame::InvalidFrame << false
                 << QByteArray() << 0u << false;
    QTest::newRow("data frame")
                 << QCanBusFrame::DataFrame << true
                 << QByteArray() << 0u << false;
    QTest::newRow("error frame")
                 << QCanBusFrame::ErrorFrame << true
                 << QByteArray() << 0u << false;
    QTest::newRow("remote request frame")
                 << QCanBusFrame::RemoteRequestFrame << true
                 << QByteArray() << 0u << false;
    QTest::newRow("unknown frame")
                 << QCanBusFrame::UnknownFrame << true
                 << QByteArray() << 0u << false;
    QTest::newRow("data frame CAN max payload")
                 << QCanBusFrame::DataFrame << true
                 << QByteArray(8, 0) << 0u << false;
    QTest::newRow("data frame CAN FD max payload")
                 << QCanBusFrame::DataFrame << true
                 << QByteArray(64, 0) << 0u << false;
    QTest::newRow("data frame CAN too much payload")
                 << QCanBusFrame::DataFrame << false
                 << QByteArray(65, 0) << 0u << false;
    QTest::newRow("data frame short id")
                 << QCanBusFrame::DataFrame << true
                 << QByteArray(8, 0) << (1u << 11) - 1 << false;
    QTest::newRow("data frame long id")
                 << QCanBusFrame::DataFrame << true
                 << QByteArray(8, 0) << (1u << 11) << true;
    QTest::newRow("data frame bad long id")
                 << QCanBusFrame::DataFrame << false
                 << QByteArray(8, 0) << (1u << 11) << false;
 }

void tst_QCanBusFrame::tst_isValid()
{
    QFETCH(QCanBusFrame::FrameType, frameType);
    QFETCH(bool, isValid);
    QFETCH(QByteArray, payload);
    QFETCH(uint, id);
    QFETCH(bool, extended);

    QCanBusFrame frame(frameType);
    frame.setPayload(payload);
    frame.setFrameId(id);
    frame.setExtendedFrameFormat(extended);
    QCOMPARE(frame.isValid(), isValid);
    QCOMPARE(frame.frameType(), frameType);
    QCOMPARE(frame.payload(), payload);
    QCOMPARE(frame.frameId(), id);
    QCOMPARE(frame.hasExtendedFrameFormat(), extended);

    frame.setFrameType(QCanBusFrame::InvalidFrame);
    QCOMPARE(frame.isValid(), false);
    QCOMPARE(QCanBusFrame::InvalidFrame, frame.frameType());
}

void tst_QCanBusFrame::streaming_data()
{
    QTest::addColumn<quint32>("frameId");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<qint64>("seconds");
    QTest::addColumn<qint64>("microSeconds");
    QTest::addColumn<bool>("isExtended");
    QTest::addColumn<QCanBusFrame::FrameType>("frameType");


    QTest::newRow("emptyFrame") << quint32(0) << QByteArray()
                                << qint64(0) << qint64(0)
                                << false << QCanBusFrame::DataFrame;
    QTest::newRow("fullFrame1") << quint32(123) << QByteArray("abcde1")
                               << qint64(456) << qint64(784)
                               << true << QCanBusFrame::DataFrame;
    QTest::newRow("fullFrame2") << quint32(123) << QByteArray("abcde2")
                               << qint64(457) << qint64(785)
                               << false << QCanBusFrame::DataFrame;
    QTest::newRow("fullFrame3") << quint32(123) << QByteArray("abcde3")
                               << qint64(458) << qint64(786)
                               << true << QCanBusFrame::RemoteRequestFrame;
    QTest::newRow("fullFrame4") << quint32(123) << QByteArray("abcde4")
                               << qint64(459) << qint64(787)
                               << false << QCanBusFrame::RemoteRequestFrame;
    QTest::newRow("fullFrame5") << quint32(123) << QByteArray("abcde5")
                               << qint64(460) << qint64(789)
                               << true << QCanBusFrame::ErrorFrame;
    QTest::newRow("fullFrame6") << quint32(123) << QByteArray("abcde6")
                               << qint64(453) << qint64(788)
                               << false << QCanBusFrame::ErrorFrame;
}

void tst_QCanBusFrame::streaming()
{
    QFETCH(quint32, frameId);
    QFETCH(QByteArray, payload);
    QFETCH(qint64, seconds);
    QFETCH(qint64, microSeconds);
    QFETCH(bool, isExtended);
    QFETCH(QCanBusFrame::FrameType, frameType);

    QCanBusFrame originalFrame(frameId, payload);
    const QCanBusFrame::TimeStamp originalStamp(seconds, microSeconds);
    originalFrame.setTimeStamp(originalStamp);

    originalFrame.setExtendedFrameFormat(isExtended);
    originalFrame.setFrameType(frameType);

    QByteArray buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);
    out << originalFrame;

    QDataStream in(buffer);
    QCanBusFrame restoredFrame;
    in >> restoredFrame;
    const QCanBusFrame::TimeStamp restoredStamp(restoredFrame.timeStamp());

    QCOMPARE(restoredFrame.frameId(), originalFrame.frameId());
    QCOMPARE(restoredFrame.payload(), originalFrame.payload());

    QCOMPARE(restoredStamp.seconds(), originalStamp.seconds());
    QCOMPARE(restoredStamp.microSeconds(), originalStamp.microSeconds());

    QCOMPARE(restoredFrame.frameType(), originalFrame.frameType());
    QCOMPARE(restoredFrame.hasExtendedFrameFormat(),
             originalFrame.hasExtendedFrameFormat());
}

void tst_QCanBusFrame::tst_error()
{
    QCanBusFrame frame(1);
    QCOMPARE(frame.frameType(), QCanBusFrame::DataFrame);
    QCOMPARE(frame.frameId(), 1u);
    QCOMPARE(frame.error(), QCanBusFrame::NoError);

    //set error -> should be ignored since still DataFrame
    frame.setError(QCanBusFrame::AnyError);
    QCOMPARE(frame.frameType(), QCanBusFrame::DataFrame);
    QCOMPARE(frame.frameId(), 1u);
    QCOMPARE(frame.error(), QCanBusFrame::NoError);

    frame.setFrameType(QCanBusFrame::ErrorFrame);
    QCOMPARE(frame.frameType(), QCanBusFrame::ErrorFrame);
    QCOMPARE(frame.frameId(), 0u); //id of Error frame always 0
    QCOMPARE(frame.error(), QCanBusFrame::TransmissionTimeoutError);

    frame.setError(QCanBusFrame::FrameErrors(QCanBusFrame::ControllerError|QCanBusFrame::ProtocolViolationError));
    QCOMPARE(frame.frameType(), QCanBusFrame::ErrorFrame);
    QCOMPARE(frame.frameId(), 0u); //id of Error frame always 0
    QCOMPARE(frame.error(),
             QCanBusFrame::ControllerError|QCanBusFrame::ProtocolViolationError);

    frame.setFrameType(QCanBusFrame::RemoteRequestFrame);
    QCOMPARE(frame.frameType(), QCanBusFrame::RemoteRequestFrame);
    QCOMPARE(frame.frameId(), (uint)(QCanBusFrame::ControllerError|QCanBusFrame::ProtocolViolationError));
    QCOMPARE(frame.error(), QCanBusFrame::NoError);
}

QTEST_MAIN(tst_QCanBusFrame)

#include "tst_qcanbusframe.moc"
