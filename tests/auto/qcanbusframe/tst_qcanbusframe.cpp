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

    void streaming_data();
    void streaming();
};

tst_QCanBusFrame::tst_QCanBusFrame()
{
}

void tst_QCanBusFrame::constructors()
{
    QCanBusFrame frame1;
    QCanBusFrame frame2(123, "tst");
    QCanBusFrame::TimeStamp timeStamp1;
    QCanBusFrame::TimeStamp timeStamp2(5, 5);

    QVERIFY(frame1.payload().isEmpty());
    QVERIFY(!frame1.frameId());

    QVERIFY(!frame2.payload().isEmpty());
    QVERIFY(frame2.frameId());

    QVERIFY(!timeStamp1.seconds());
    QVERIFY(!timeStamp1.microSeconds());

    QVERIFY(timeStamp2.seconds());
    QVERIFY(timeStamp2.microSeconds());

    QVERIFY(frame1.hasExtendedFrameFormat() == true);
    QVERIFY(frame2.hasExtendedFrameFormat() == true);

    QCOMPARE(frame1.frameType(), QCanBusFrame::DataFrame);
    QCOMPARE(frame2.frameType(), QCanBusFrame::DataFrame);
}

void tst_QCanBusFrame::id()
{
    QCanBusFrame frame;
    QVERIFY(!frame.frameId());
    frame.setFrameId(512u);
    QCOMPARE(frame.frameId(), 512u);
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

Q_DECLARE_METATYPE(QCanBusFrame::FrameType)

QTEST_MAIN(tst_QCanBusFrame)

#include "tst_qcanbusframe.moc"
