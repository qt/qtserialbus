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
#include <QtSerialBus/QCanFrame>

class tst_QCanFrame : public QObject
{
    Q_OBJECT
public:
    explicit tst_QCanFrame();

private slots:
    void constructors();
    void id();
    void payload();
    void timeStamp();

    void streaming_data();
    void streaming();
};

tst_QCanFrame::tst_QCanFrame()
{
}

void tst_QCanFrame::constructors()
{
    QCanFrame frame1;
    QCanFrame frame2(123, "tst");
    QCanFrame::TimeStamp timeStamp1;
    QCanFrame::TimeStamp timeStamp2(5, 5);

    QVERIFY(frame1.payload().isEmpty());
    QVERIFY(!frame1.frameId());

    QVERIFY(!frame2.payload().isEmpty());
    QVERIFY(frame2.frameId());

    QVERIFY(!timeStamp1.seconds());
    QVERIFY(!timeStamp1.microSeconds());

    QVERIFY(timeStamp2.seconds());
    QVERIFY(timeStamp2.microSeconds());
}

void tst_QCanFrame::id()
{
    QCanFrame frame;
    QVERIFY(!frame.frameId());
    frame.setFrameId(512);
    QCOMPARE(frame.frameId(), 512);
}

void tst_QCanFrame::payload()
{
    QCanFrame frame;
    QVERIFY(frame.payload().isEmpty());
    frame.setPayload("test");
    QCOMPARE(frame.payload().data(), "test");
}

void tst_QCanFrame::timeStamp()
{
    QCanFrame frame;
    QCanFrame::TimeStamp timeStamp = frame.timeStamp();
    QVERIFY(!timeStamp.seconds());
    QVERIFY(!timeStamp.microSeconds());
    timeStamp.setMicroSeconds(5);
    timeStamp.setSeconds(4);
    QCOMPARE(timeStamp.seconds(), 4);
    QCOMPARE(timeStamp.microSeconds(), 5);
}

void tst_QCanFrame::streaming_data()
{
    QTest::addColumn<qint32>("frameId");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<qint64>("seconds");
    QTest::addColumn<qint64>("microSeconds");

    QTest::newRow("emptyFrame") << qint32(0) << QByteArray() << qint64(0) << qint64(0);
    QTest::newRow("fullFrame") << qint32(123) << QByteArray("abcdef") << qint64(456) << qint64(789);
}

void tst_QCanFrame::streaming()
{
    QFETCH(qint32, frameId);
    QFETCH(QByteArray, payload);
    QFETCH(qint64, seconds);
    QFETCH(qint64, microSeconds);

    QCanFrame originalFrame(frameId, payload);
    const QCanFrame::TimeStamp originalStamp(seconds, microSeconds);
    originalFrame.setTimeStamp(originalStamp);

    QByteArray buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);
    out << originalFrame;

    QDataStream in(buffer);
    QCanFrame restoredFrame;
    in >> restoredFrame;
    const QCanFrame::TimeStamp restoredStamp(restoredFrame.timeStamp());

    QCOMPARE(restored.frameId(), original.frameId());
    QCOMPARE(restored.payload(), original.payload());

    QCOMPARE(restoredStamp.seconds(), originalStamp.seconds());
    QCOMPARE(restoredStamp.microSeconds(), originalStamp.microSeconds());
}

QTEST_MAIN(tst_QCanFrame)

#include "tst_qcanframe.moc"
