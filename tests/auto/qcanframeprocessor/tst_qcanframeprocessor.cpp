// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtTest/qtest.h>

#include <QtCore/QtEndian>

#include <QtSerialBus/qcanbusframe.h>
#include <QtSerialBus/qcanframeprocessor.h>
#include <QtSerialBus/qcanmessagedescription.h>
#include <QtSerialBus/qcansignaldescription.h>
#include <QtSerialBus/private/qcanframeprocessor_p.h>

QT_USE_NAMESPACE

class tst_QCanFrameProcessor : public QObject
{
    Q_OBJECT

private slots:
    /* basics */
    void construct();
    void copy();
    void move();
    void comparison();
    void detachOnOperations();

    /* parse */
    void parseSignalFromFrameId_data();
    void parseSignalFromFrameId();

    void parseSignalFromPayload_data();
    void parseSignalFromPayload();

    void parseWithValueConversion_data();
    void parseWithValueConversion();

    void parseMultiplexedSignals_data();
    void parseMultiplexedSignals();

    void parseExtendedMultiplexedSignals_data();
    void parseExtendedMultiplexedSignals();

    void parseWithErrorsAndWarnings_data();
    void parseWithErrorsAndWarnings();

    void extractUniqueId_data();
    void extractUniqueId();

    /* generate */
    void prepareFrameId_data();
    void prepareFrameId();

    void preparePayload_data();
    void preparePayload();

    void prepareWithValueConversion_data();
    void prepareWithValueConversion();

    void prepareMultiplexedPayload_data();
    void prepareMultiplexedPayload();

    void prepareWithErrorsAndWarnings_data();
    void prepareWithErrorsAndWarnings();

    void prepareUniqueId_data();
    void prepareUniqueId();
};

void tst_QCanFrameProcessor::construct()
{
    QCanFrameProcessor p;

    QVERIFY(p.messageDescriptions().isEmpty());
    QCOMPARE(p.error(), QCanFrameProcessor::Error::NoError);
    QVERIFY(p.errorDescription().isEmpty());
    QVERIFY(p.warnings().isEmpty());
    QVERIFY(!p.uniqueIdDescription().isValid());
    QVERIFY(!QCanFrameProcessorPrivate::get(p)->isShared());
}

void tst_QCanFrameProcessor::copy()
{
    QCanMessageDescription desc;
    desc.setName("test");
    desc.setUniqueId(123);
    desc.setSize(8);

    QCanFrameProcessor p;
    p.setMessageDescriptions({ desc });
    QVERIFY(!QCanFrameProcessorPrivate::get(p)->isShared());

    QCanFrameProcessor p1(p);
    QCOMPARE(p1, p);
    QVERIFY(QCanFrameProcessorPrivate::get(p)->isShared());
    QVERIFY(QCanFrameProcessorPrivate::get(p1)->isShared());

    QCanFrameProcessor p2 = p;
    QCOMPARE(p2, p);
    QVERIFY(QCanFrameProcessorPrivate::get(p)->isShared());
    QVERIFY(QCanFrameProcessorPrivate::get(p1)->isShared());
    QVERIFY(QCanFrameProcessorPrivate::get(p2)->isShared());

    p2.clearMessageDescriptions();
    QCOMPARE_NE(p2, p);
    QVERIFY(QCanFrameProcessorPrivate::get(p)->isShared());
    QVERIFY(QCanFrameProcessorPrivate::get(p1)->isShared());
    QVERIFY(!QCanFrameProcessorPrivate::get(p2)->isShared());
}

void tst_QCanFrameProcessor::move()
{
    QCanMessageDescription desc;
    desc.setName("test");
    desc.setUniqueId(123);
    desc.setSize(8);

    QCanFrameProcessor p;
    p.setMessageDescriptions({ desc });
    QVERIFY(!QCanFrameProcessorPrivate::get(p)->isShared());

    QCanFrameProcessor pCopy;
    pCopy.setMessageDescriptions({ desc });
    QCOMPARE(pCopy, p);

    QCanFrameProcessor p1(std::move(p));
    QCOMPARE(p1, pCopy);
    QVERIFY(!QCanFrameProcessorPrivate::get(p1)->isShared());

    QCanFrameProcessor p2 = std::move(p1);
    QCOMPARE(p2, pCopy);
    QVERIFY(!QCanFrameProcessorPrivate::get(p2)->isShared());

    p = std::move(p2);
    QCOMPARE(p, pCopy);
    QVERIFY(!QCanFrameProcessorPrivate::get(p)->isShared());
}

void tst_QCanFrameProcessor::comparison()
{
    QCanSignalDescription sigDesc;
    sigDesc.setBitLength(8);

    QCanMessageDescription desc;
    desc.setName("test");
    desc.setUniqueId(123);
    desc.setSize(1);
    desc.addSignalDescription(sigDesc);

    QCanUniqueIdDescription uidDesc;
    uidDesc.setSource(QtCanBus::DataSource::FrameId);
    uidDesc.setStartBit(0);
    uidDesc.setBitLength(29);

    QCanFrameProcessor p;
    p.setUniqueIdDescription(uidDesc);
    p.setMessageDescriptions({ desc });

    QCanFrameProcessor p1 = p;
    QCOMPARE(p1, p);

    // make sure that parsing errors do not affect the comparison
    p1.parseFrame(QCanBusFrame(1, QByteArray(1, 0x00)));
    QCOMPARE_NE(p1.error(), p.error());
    QCOMPARE_EQ(p1, p);

    // make sure that warnings do not affect comparison
    p1.parseFrame(QCanBusFrame(123, QByteArray(1, 0x00)));
    QCOMPARE_NE(p1.warnings(), p.warnings());
    QCOMPARE_EQ(p1, p);

    // updating uniqueId description affects comparison
    uidDesc.setSource(QtCanBus::DataSource::Payload);
    p1.setUniqueIdDescription(uidDesc);
    QCOMPARE_NE(p1, p);

    p1 = p;
    QCOMPARE(p1, p);
    // updating message descriptions affects comparison
    desc.setName("test1");
    desc.setUniqueId(456);
    p1.addMessageDescriptions({ desc });
    QCOMPARE_NE(p1, p);
}

void tst_QCanFrameProcessor::detachOnOperations()
{
    QCanMessageDescription desc;
    desc.setName("test");
    desc.setUniqueId(123);
    desc.setSize(8);

    QCanFrameProcessor p;
    p.setMessageDescriptions({ desc });

    // calling const methods does not detach
    {
        QCanFrameProcessor p1(p);
        QVERIFY(QCanFrameProcessorPrivate::get(p1)->isShared());

        const auto error = p1.error();
        Q_UNUSED(error);
        QVERIFY(QCanFrameProcessorPrivate::get(p1)->isShared());

        const auto errorDesc = p1.errorDescription();
        Q_UNUSED(errorDesc);
        QVERIFY(QCanFrameProcessorPrivate::get(p1)->isShared());

        const auto warnings = p1.warnings();
        Q_UNUSED(warnings);
        QVERIFY(QCanFrameProcessorPrivate::get(p1)->isShared());

        const auto msgDesc = p1.messageDescriptions();
        Q_UNUSED(msgDesc);
        QVERIFY(QCanFrameProcessorPrivate::get(p1)->isShared());

        const auto uidDesc = p1.uniqueIdDescription();
        Q_UNUSED(uidDesc);
        QVERIFY(QCanFrameProcessorPrivate::get(p1)->isShared());
    }

    // callings non-const methods detaches
    {
        QCanFrameProcessor p1(p);
        QVERIFY(QCanFrameProcessorPrivate::get(p1)->isShared());

        p1.prepareFrame(123, QVariantMap());
        QVERIFY(!QCanFrameProcessorPrivate::get(p1)->isShared());
    }
    {
        QCanFrameProcessor p1(p);
        QVERIFY(QCanFrameProcessorPrivate::get(p1)->isShared());

        p1.parseFrame(QCanBusFrame(123, QByteArray(8, 0x01)));
        QVERIFY(!QCanFrameProcessorPrivate::get(p1)->isShared());
    }
    QCanMessageDescription desc1;
    desc1.setName("test1");
    desc1.setUniqueId(234);
    desc1.setSize(8);
    {
        QCanFrameProcessor p1(p);
        QVERIFY(QCanFrameProcessorPrivate::get(p1)->isShared());

        p1.addMessageDescriptions({ desc1 });
        QVERIFY(!QCanFrameProcessorPrivate::get(p1)->isShared());
    }
    {
        QCanFrameProcessor p1(p);
        QVERIFY(QCanFrameProcessorPrivate::get(p1)->isShared());

        p1.setMessageDescriptions({ desc, desc1 });
        QVERIFY(!QCanFrameProcessorPrivate::get(p1)->isShared());
    }
    {
        QCanFrameProcessor p1(p);
        QVERIFY(QCanFrameProcessorPrivate::get(p1)->isShared());

        p1.clearMessageDescriptions();
        QVERIFY(!QCanFrameProcessorPrivate::get(p1)->isShared());
    }
    {
        QCanFrameProcessor p1(p);
        QVERIFY(QCanFrameProcessorPrivate::get(p1)->isShared());

        auto uidDesc = p.uniqueIdDescription();
        uidDesc.setBitLength(1);
        p1.setUniqueIdDescription(uidDesc);
        QVERIFY(!QCanFrameProcessorPrivate::get(p1)->isShared());
    }
}

void tst_QCanFrameProcessor::parseSignalFromFrameId_data()
{
    QTest::addColumn<quint32>("frameId");
    QTest::addColumn<quint16>("startBit");
    QTest::addColumn<quint16>("bitLength");
    QTest::addColumn<QtCanBus::DataFormat>("format");
    QTest::addColumn<QtCanBus::DataEndian>("endian");
    QTest::addColumn<QVariant>("expectedValue");

    quint32 frameId {0x1234FFEC};
    for (quint16 i = 0; i < 8; ++i) {
        quint32 value = (frameId << i) & 0x1FFFFFFFU;

        QTest::addRow("int16, le, from %d", i)
                << value << i << quint16(16)
                << QtCanBus::DataFormat::SignedInteger
                << QtCanBus::DataEndian::LittleEndian
                << QVariant(-20);

        QTest::addRow("uint16, le, from %d", i)
                << value << i << quint16(16)
                << QtCanBus::DataFormat::UnsignedInteger
                << QtCanBus::DataEndian::LittleEndian
                << QVariant(65516);
    }

    frameId = quint32(0x12348582);
    for (quint16 i = 0; i < 8; ++i) {
        quint32 value = (frameId << i) & 0x1FFFFFFFU;

        QTest::addRow("int16, be, from %d", i)
                << value << i << quint16(16)
                << QtCanBus::DataFormat::SignedInteger
                << QtCanBus::DataEndian::BigEndian
                << QVariant(-32123);

        QTest::addRow("uint16, be, from %d", i)
                << value << i << quint16(16)
                << QtCanBus::DataFormat::UnsignedInteger
                << QtCanBus::DataEndian::BigEndian
                << QVariant(33413);
    }
}

void tst_QCanFrameProcessor::parseSignalFromFrameId()
{
    QFETCH(quint32, frameId);
    QFETCH(quint16, startBit);
    QFETCH(quint16, bitLength);
    QFETCH(QtCanBus::DataFormat, format);
    QFETCH(QtCanBus::DataEndian, endian);
    QFETCH(QVariant, expectedValue);

    QCanSignalDescription sig;
    sig.setName("test_signal");
    sig.setDataSource(QtCanBus::DataSource::FrameId);
    sig.setDataFormat(format);
    sig.setDataEndian(endian);
    sig.setStartBit(startBit);
    sig.setBitLength(bitLength);
    QVERIFY(sig.isValid());

    // The unique id will be extracted from the 0-th payload byte
    QCanUniqueIdDescription uidDesc;
    uidDesc.setSource(QtCanBus::DataSource::Payload);
    uidDesc.setStartBit(0);
    uidDesc.setBitLength(4);

    const QtCanBus::UniqueId expectedUid = 0x0A;

    QCanMessageDescription message;
    message.setName("test_message");
    message.setUniqueId(expectedUid);
    message.setSize(1);
    message.addSignalDescription(sig);
    QVERIFY(message.isValid());

    QCanFrameProcessor parser;
    parser.setUniqueIdDescription(uidDesc);
    parser.addMessageDescriptions({ message });

    QCanBusFrame frame(frameId, QByteArray(1, 0x0A));
    QVERIFY(frame.isValid());

    const auto result = parser.parseFrame(frame);
    const auto signalValues = result.signalValues;
    QCOMPARE(result.uniqueId, expectedUid);
    QVERIFY(!signalValues.isEmpty());

    const QVariantMap expectedSignalValues({qMakePair(sig.name(), expectedValue)});

    // The values can be doubles, so we need to compare them explicitly
    QCOMPARE(signalValues.keys(), expectedSignalValues.keys());
    for (const auto &key : expectedSignalValues.keys()) {
        const auto expectedValue = expectedSignalValues.value(key);
        if (expectedValue.canConvert(QMetaType::fromType<double>()))
            QCOMPARE(signalValues.value(key).toDouble(), expectedValue.toDouble());
        else
            QCOMPARE(signalValues.value(key), expectedValue);
    }
}

void tst_QCanFrameProcessor::parseSignalFromPayload_data()
{
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<quint16>("startBit");
    QTest::addColumn<quint16>("bitLength");
    QTest::addColumn<QtCanBus::DataFormat>("format");
    QTest::addColumn<QtCanBus::DataEndian>("endian");
    QTest::addColumn<QVariant>("expectedValue");

    // (u)int, LE/BE, variable start and bitLength

    // Every next value in the list is supposed to have a length in bits cut
    // by one.
    static constexpr quint16 unsignedValues[] = { 0xCFC7, 0x67E3, 0x33F1, 0x19F8,
                                                  0x0CFC, 0x067E, 0x033F, 0x019F };
    // signedValues contain signed representations of the unsignedValues,
    // considering that the bit lengths is cut every time. For example,
    // unsingedValues[1] contain 1 in its 14'th bit (starting from 0), so it
    // must be treated as a negative value.
    static constexpr qint16 signedValues[] = { -12345, -6173, -3087, -1544,
                                               -772, -386, -193, -97 };

    for (quint16 i = 0; i < 8; ++i) {
        // shift, to change start bit
        quint32 valueForPayload = qToLittleEndian(unsignedValues[i]) << i;
        const QByteArray expectedPayload(reinterpret_cast<const char*>(&valueForPayload),
                                         sizeof(quint32));
        QTest::addRow("int, le, from %d, length %d", i, 16 - i)
                << expectedPayload << i << quint16(16 - i)
                << QtCanBus::DataFormat::SignedInteger
                << QtCanBus::DataEndian::LittleEndian
                << QVariant(signedValues[i]);

        QTest::addRow("uint, le, from %d, length %d", i, 16 - i)
                << expectedPayload << i << quint16(16 - i)
                << QtCanBus::DataFormat::UnsignedInteger
                << QtCanBus::DataEndian::LittleEndian
                << QVariant(unsignedValues[i]);

        const quint16 beValue = qToBigEndian(unsignedValues[i]);
        const uchar *beValueData = reinterpret_cast<const uchar *>(&beValue);
        quint32 beValueForPayload = (beValueData[0] << i) + (beValueData[1] << 8);
        const QByteArray expectedBePayload(reinterpret_cast<const char*>(&beValueForPayload),
                                           sizeof(quint32));
        QTest::addRow("int, be, from %d, length %d", i, 16 - i)
                << expectedBePayload << i << quint16(16 - i)
                << QtCanBus::DataFormat::SignedInteger
                << QtCanBus::DataEndian::BigEndian
                << QVariant(signedValues[i]);

        QTest::addRow("uint, be, from %d, length %d", i, 16 - i)
                << expectedBePayload << i << quint16(16 - i)
                << QtCanBus::DataFormat::UnsignedInteger
                << QtCanBus::DataEndian::BigEndian
                << QVariant(unsignedValues[i]);
    }

    // float, LE/BE, variable start
    const float floatVal = 123.456f;
    for (quint16 i = 0; i < 8; ++i) {
        quint64 valueForPayload = 0;
        const float leFloatVal = qToLittleEndian(floatVal);
        memcpy(&valueForPayload, &leFloatVal, sizeof(float));
        valueForPayload = valueForPayload << i;
        const QByteArray expectedPayload(reinterpret_cast<const char*>(&valueForPayload),
                                         sizeof(quint64));
        QTest::addRow("float, le, from %d", i)
                << expectedPayload << i << quint16(32)
                << QtCanBus::DataFormat::Float
                << QtCanBus::DataEndian::LittleEndian
                << QVariant(floatVal);

        valueForPayload = 0;
        const float beFloatVal = qToBigEndian(floatVal);
        memcpy(&valueForPayload, &beFloatVal, sizeof(float));
        valueForPayload = valueForPayload << i;
        const QByteArray expectedBePayload(reinterpret_cast<const char*>(&valueForPayload),
                                           sizeof(quint64));
        QTest::addRow("float, be, from %d", i)
                << expectedBePayload << i << quint16(32)
                << QtCanBus::DataFormat::Float
                << QtCanBus::DataEndian::BigEndian
                << QVariant(floatVal);
    }

    // double, LE/BE
    {
        const double doubleVal = 1.234e-15;
        const double leDoubleVal = qToLittleEndian(doubleVal);
        const QByteArray expectedPayload(reinterpret_cast<const char*>(&leDoubleVal),
                                         sizeof(double));
        QTest::addRow("double, le")
                << expectedPayload << quint16(0) << quint16(64)
                << QtCanBus::DataFormat::Double
                << QtCanBus::DataEndian::LittleEndian
                << QVariant(doubleVal);

        const double beDoubleVal = qToBigEndian(doubleVal);
        const QByteArray expectedBePayload(reinterpret_cast<const char*>(&beDoubleVal),
                                           sizeof(double));
        QTest::addRow("double, be")
                << expectedBePayload << quint16(0) << quint16(64)
                << QtCanBus::DataFormat::Double
                << QtCanBus::DataEndian::BigEndian
                << QVariant(doubleVal);
    }

    // ASCII
    {
        const QByteArray expectedData("abc", 3);
        const quint16 offset = 8; // start from the 2nd byte
        for (quint16 i = 0; i < 8; ++i) {
            quint64 valueForPayload = 0;
            memcpy(&valueForPayload, expectedData.constData(), expectedData.size());
            valueForPayload = valueForPayload << (offset + i);
            const QByteArray payload(reinterpret_cast<const char *>(&valueForPayload),
                                     sizeof(quint64));
            QTest::addRow("ascii, offset %d", offset + i)
                    << payload << quint16(offset + i) << quint16(expectedData.size() * 8)
                    << QtCanBus::DataFormat::Ascii
                    << QtCanBus::DataEndian::LittleEndian
                    << QVariant(expectedData);
        }
    }
}

void tst_QCanFrameProcessor::parseSignalFromPayload()
{
    QFETCH(QByteArray, payload);
    QFETCH(quint16, startBit);
    QFETCH(quint16, bitLength);
    QFETCH(QtCanBus::DataFormat, format);
    QFETCH(QtCanBus::DataEndian, endian);
    QFETCH(QVariant, expectedValue);

    QCanSignalDescription sig;
    sig.setName("test_signal");
    sig.setDataFormat(format);
    sig.setDataEndian(endian);
    sig.setStartBit(startBit);
    sig.setBitLength(bitLength);
    QVERIFY(sig.isValid());

    const QtCanBus::UniqueId uniqueId = 123;

    QCanMessageDescription message;
    message.setName("test_message");
    message.setUniqueId(uniqueId);
    message.setSize(payload.size());
    message.addSignalDescription(sig);
    QVERIFY(message.isValid());

    // Assume that our uniqueId is the whole FrameId
    QCanUniqueIdDescription uidDesc;
    uidDesc.setBitLength(29);

    QCanFrameProcessor parser;
    parser.setUniqueIdDescription(uidDesc);
    parser.addMessageDescriptions({ message });

    QCanBusFrame frame(uniqueId, payload);
    QVERIFY(frame.isValid());

    const auto result = parser.parseFrame(frame);
    const auto signalValues = result.signalValues;
    QCOMPARE(result.uniqueId, uniqueId);
    QVERIFY(!signalValues.isEmpty());

    const QVariantMap expectedSignalValues({qMakePair(sig.name(), expectedValue)});

    // The values can be doubles, so we need to compare them explicitly
    QCOMPARE(signalValues.keys(), expectedSignalValues.keys());
    for (const auto &key : expectedSignalValues.keys()) {
        const auto expectedValue = expectedSignalValues.value(key);
        if (expectedValue.canConvert(QMetaType::fromType<double>()))
            QCOMPARE(signalValues.value(key).toDouble(), expectedValue.toDouble());
        else
            QCOMPARE(signalValues.value(key), expectedValue);
    }
}

void tst_QCanFrameProcessor::parseWithValueConversion_data()
{
    QTest::addColumn<double>("factor");
    QTest::addColumn<double>("offset");
    QTest::addColumn<double>("scaling");
    QTest::addColumn<quint8>("payloadValue");
    QTest::addColumn<QVariant>("expectedValue");

    // expectedValue = scaling * (payloadValue * factor + offset);
    QTest::newRow("nan, nan, nan") << qQNaN() << qQNaN() << qQNaN() << quint8(30) << QVariant(30);
    QTest::newRow("0.75, nan, nan") << 0.75 << qQNaN() << qQNaN() << quint8(30) << QVariant(22.5);
    QTest::newRow("0.75, 15.3, nan") << 0.75 << 15.3 << qQNaN() << quint8(30) << QVariant(37.8);
    QTest::newRow("0.75, 15.3, 100") << 0.75 << 15.3 << 100. << quint8(30) << QVariant(3780.);
    QTest::newRow("0.75, nan, 100") << 0.75 << qQNaN() << 100. << quint8(30) << QVariant(2250.);
    QTest::newRow("nan, 15.3, 100") << qQNaN() << 15.3 << 100. << quint8(30) << QVariant(4530.);
    // check that setting factor and/or scaling to 0 is equivalent to NaN
    QTest::newRow("0, 15.3, 100") << 0. << 15.3 << 100. << quint8(30) << QVariant(4530.);
    QTest::newRow("0, 15.3, 0") << 0. << 15.3 << 0. << quint8(30) << QVariant(45.3);
}

void tst_QCanFrameProcessor::parseWithValueConversion()
{
    QFETCH(double, factor);
    QFETCH(double, offset);
    QFETCH(double, scaling);
    QFETCH(quint8, payloadValue);
    QFETCH(QVariant, expectedValue);

    QCanSignalDescription sig;
    sig.setName("test_signal");
    sig.setStartBit(0);
    sig.setBitLength(8);
    sig.setFactor(factor);
    sig.setOffset(offset);
    sig.setScaling(scaling);

    const QtCanBus::UniqueId uniqueId = 123;

    QCanMessageDescription message;
    message.setName("test_message");
    message.setUniqueId(uniqueId);
    message.setSize(1);
    message.addSignalDescription(sig);
    QVERIFY(message.isValid());

    // Assume that our uniqueId is the whole FrameId
    QCanUniqueIdDescription uidDesc;
    uidDesc.setBitLength(29);

    QCanFrameProcessor parser;
    parser.setUniqueIdDescription(uidDesc);
    parser.addMessageDescriptions({ message });

    QCanBusFrame frame(uniqueId, QByteArray(1, char(payloadValue)));
    QVERIFY(frame.isValid());

    const auto result = parser.parseFrame(frame);
    const auto signalValues = result.signalValues;
    QCOMPARE(result.uniqueId, uniqueId);
    QVERIFY(!signalValues.isEmpty());

    const QVariantMap expectedSignalValues({qMakePair(sig.name(), expectedValue)});

    // The values can be doubles, so we need to compare them explicitly
    QCOMPARE(signalValues.keys(), expectedSignalValues.keys());
    for (const auto &key : expectedSignalValues.keys()) {
        const auto expectedValue = expectedSignalValues.value(key);
        if (expectedValue.canConvert(QMetaType::fromType<double>()))
            QCOMPARE(signalValues.value(key).toDouble(), expectedValue.toDouble());
        else
            QCOMPARE(signalValues.value(key), expectedValue);
    }
}

void tst_QCanFrameProcessor::parseMultiplexedSignals_data()
{
    QTest::addColumn<QString>("s0Name");
    QTest::addColumn<QString>("s1Name");
    QTest::addColumn<QString>("s2Name");
    QTest::addColumn<QVariant>("s1MuxValue");
    QTest::addColumn<QVariant>("s2MuxValue");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<QVariantMap>("expectedResult");

    const QString s0("s0");
    const QString s1("s1");
    const QString s2("s2");
    const QVariant s1MuxValue(1);
    const QVariant s2MuxValue(2);

    QTest::newRow("s0 and s1") << s0 << s1 << s2 << s1MuxValue << s2MuxValue
                               << QByteArray(1, 0x29)
                               << QVariantMap({ qMakePair(s0, s1MuxValue), qMakePair(s1, 0x0A) });

    QTest::newRow("s0 and s2") << s0 << s1 << s2 << s1MuxValue << s2MuxValue
                               << QByteArray(1, 0x2E)
                               << QVariantMap({ qMakePair(s0, s2MuxValue), qMakePair(s2, 0x0B) });

    QTest::newRow("only s0") << s0 << s1 << s2 << s1MuxValue << s2MuxValue
                             << QByteArray(1, 0x73)
                             << QVariantMap({ qMakePair(s0, 0x03) });

}

void tst_QCanFrameProcessor::parseMultiplexedSignals()
{
    QFETCH(QString, s0Name);
    QFETCH(QString, s1Name);
    QFETCH(QString, s2Name);
    QFETCH(QVariant, s1MuxValue);
    QFETCH(QVariant, s2MuxValue);
    QFETCH(QByteArray, payload);
    QFETCH(QVariantMap, expectedResult);

    QCanSignalDescription s0; // multiplexor
    s0.setName(s0Name);
    s0.setStartBit(0);
    s0.setBitLength(2);
    s0.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
    s0.setMultiplexState(QtCanBus::MultiplexState::MultiplexorSwitch);

    QCanSignalDescription s1; // multiplexed signal 1, used when s0 == 1
    s1.setName(s1Name);
    s1.setStartBit(2);
    s1.setBitLength(6);
    s1.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    s1.addMultiplexSignal(s0.name(), s1MuxValue);

    QCanSignalDescription s2; // multiplexed signal 2, used when s0 == 2
    s2.setName(s2Name);
    s2.setStartBit(2);
    s2.setBitLength(6);
    s2.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    s2.addMultiplexSignal(s0.name(), s2MuxValue);

    const QtCanBus::UniqueId uniqueId = 123;

    QCanMessageDescription msg;
    msg.setName("test");
    msg.setUniqueId(uniqueId);
    msg.setSize(payload.size());
    msg.setSignalDescriptions({ s0, s1, s2 });

    // Assume that our uniqueId is the whole FrameId
    QCanUniqueIdDescription uidDesc;
    uidDesc.setBitLength(29);

    QCanFrameProcessor parser;
    parser.setUniqueIdDescription(uidDesc);
    parser.addMessageDescriptions({ msg });

    QCanBusFrame frame(msg.uniqueId(), payload);
    QVERIFY(frame.isValid());

    const auto result = parser.parseFrame(frame);
    QCOMPARE(result.uniqueId, uniqueId);
    QCOMPARE(result.signalValues, expectedResult);
}

void tst_QCanFrameProcessor::parseExtendedMultiplexedSignals_data()
{
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<QVariantMap>("expectedResult");

    const QString s0Name("s0");
    const QString s1Name("s1");
    const QString s2Name("s2");
    const QString s3Name("s3");
    const QString s4Name("s4");

    QTest::newRow("s0 and s2") << QByteArray::fromHex("2434")
                               << QVariantMap({ qMakePair(s0Name, 4),
                                                qMakePair(s2Name, 0x342) });

    QTest::newRow("s0, s1 and s3") << QByteArray::fromHex("1134")
                                   << QVariantMap({ qMakePair(s0Name, 1),
                                                    qMakePair(s1Name, 1),
                                                    qMakePair(s3Name, 0x34) });

    QTest::newRow("s0, s1 and s4") << QByteArray::fromHex("2134")
                                   << QVariantMap({ qMakePair(s0Name, 1),
                                                    qMakePair(s1Name, 2),
                                                    qMakePair(s4Name, 0x34) });

    QTest::newRow("s0 and s1") << QByteArray::fromHex("7134")
                               << QVariantMap({ qMakePair(s0Name, 1),
                                                qMakePair(s1Name, 7) });

    QTest::newRow("only s0") << QByteArray::fromHex("2534")
                             << QVariantMap({ qMakePair(s0Name, 5) });
}

void tst_QCanFrameProcessor::parseExtendedMultiplexedSignals()
{
    QFETCH(QByteArray, payload);
    QFETCH(QVariantMap, expectedResult);

    QCanSignalDescription s0; // multiplexor 1
    s0.setName("s0");
    s0.setDataEndian(QtCanBus::DataEndian::LittleEndian);
    s0.setStartBit(0);
    s0.setBitLength(4);
    s0.setMultiplexState(QtCanBus::MultiplexState::MultiplexorSwitch);

    QCanSignalDescription s1; // multiplexor 2, also depends on the value of s0
    s1.setName("s1");
    s1.setDataEndian(QtCanBus::DataEndian::LittleEndian);
    s1.setStartBit(4);
    s1.setBitLength(4);
    s1.setMultiplexState(QtCanBus::MultiplexState::SwitchAndSignal);
    s1.addMultiplexSignal(s0.name(), 1);

    QCanSignalDescription s2; // depends on mux1 only, contains 12 bits of data
    s2.setName("s2");
    s2.setDataEndian(QtCanBus::DataEndian::LittleEndian);
    s2.setStartBit(4);
    s2.setBitLength(12);
    s2.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    const QCanSignalDescription::MultiplexValues muxValuesS0 { qMakePair(2, 4) };
    s2.addMultiplexSignal(s0.name(), muxValuesS0);

    QCanSignalDescription s3; // depends on mux2 (so indirectly on mux1)
    s3.setName("s3");
    s3.setDataEndian(QtCanBus::DataEndian::LittleEndian);
    s3.setStartBit(8);
    s3.setBitLength(8);
    s3.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    s3.addMultiplexSignal(s1.name(), 1);

    QCanSignalDescription s4; // depends on mux2 (so indirectly on mux1)
    s4.setName("s4");
    s4.setDataEndian(QtCanBus::DataEndian::LittleEndian);
    s4.setStartBit(8);
    s4.setBitLength(8);
    s4.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    const QCanSignalDescription::MultiplexValues muxValuesS1 { qMakePair(2, 3), qMakePair(5, 5) };
    s4.addMultiplexSignal(s1.name(), muxValuesS1);

    const QtCanBus::UniqueId uniqueId = 123;

    QCanMessageDescription msg;
    msg.setName("test");
    msg.setUniqueId(uniqueId);
    msg.setSize(payload.size());
    msg.setSignalDescriptions({ s0, s1, s2, s3, s4 });

    // Assume that our uniqueId is the whole FrameId
    QCanUniqueIdDescription uidDesc;
    uidDesc.setBitLength(29);

    QCanFrameProcessor parser;
    parser.setUniqueIdDescription(uidDesc);
    parser.addMessageDescriptions({ msg });

    QCanBusFrame frame(msg.uniqueId(), payload);
    QVERIFY(frame.isValid());

    const auto result = parser.parseFrame(frame);
    QCOMPARE(result.uniqueId, uniqueId);
    QCOMPARE(result.signalValues, expectedResult);
}

void tst_QCanFrameProcessor::parseWithErrorsAndWarnings_data()
{
    QTest::addColumn<QCanMessageDescription>("messageDescription");
    QTest::addColumn<QCanUniqueIdDescription>("uniqueIdDescription");
    QTest::addColumn<QCanBusFrame>("frame");
    QTest::addColumn<QCanFrameProcessor::Error>("expectedError");
    QTest::addColumn<QString>("expectedErrorDescription");
    QTest::addColumn<QStringList>("expectedWarnings");
    QTest::addColumn<QtCanBus::UniqueId>("expectedUniqueId");
    QTest::addColumn<QVariantMap>("expectedResult");

    QTest::addRow("Invalid frame") << QCanMessageDescription()
                                   << QCanUniqueIdDescription()
                                   << QCanBusFrame(0x2345ABCD, QByteArray())
                                   << QCanFrameProcessor::Error::InvalidFrame
                                   << tr("Invalid frame.")
                                   << QStringList()
                                   << QtCanBus::UniqueId(0)
                                   << QVariantMap();

    QTest::addRow("Unsupported frame") << QCanMessageDescription()
                                       << QCanUniqueIdDescription()
                                       << QCanBusFrame(QCanBusFrame::RemoteRequestFrame)
                                       << QCanFrameProcessor::Error::UnsupportedFrameFormat
                                       << tr("Unsupported frame format.")
                                       << QStringList()
                                       << QtCanBus::UniqueId(0)
                                       << QVariantMap();

    QTest::addRow("No UID description")
            << QCanMessageDescription() << QCanUniqueIdDescription()
            << QCanBusFrame(123, QByteArray(2, 0x01))
            << QCanFrameProcessor::Error::DecodingError
            << tr("No valid unique identifier description is specified.")
            << QStringList() << QtCanBus::UniqueId(0) << QVariantMap();

    QCanUniqueIdDescription uidDesc;
    uidDesc.setSource(QtCanBus::DataSource::Payload);
    uidDesc.setStartBit(8);
    uidDesc.setBitLength(16); // for extended FrameId

    QTest::addRow("UID extraction failed")
            << QCanMessageDescription() << uidDesc
            << QCanBusFrame(123, QByteArray(2, 0x01))
            << QCanFrameProcessor::Error::DecodingError
            << tr("Failed to extract unique id from the frame.")
            << QStringList() << QtCanBus::UniqueId(0) << QVariantMap();

    uidDesc = QCanUniqueIdDescription(); // reset
    uidDesc.setBitLength(29);

    QTest::addRow("Unknown unique id")
            << QCanMessageDescription() << uidDesc
            << QCanBusFrame(123, QByteArray(8, 0x01))
            << QCanFrameProcessor::Error::DecodingError
            << tr("Could not find a message description for unique id 123.")
            << QStringList() << QtCanBus::UniqueId(0) << QVariantMap();

    QCanSignalDescription signalDesc;
    signalDesc.setName("s0");
    signalDesc.setStartBit(0);
    signalDesc.setBitLength(16);

    QCanMessageDescription messageDesc;
    messageDesc.setName("testMessage");
    messageDesc.setUniqueId(123);
    messageDesc.setSize(2);
    messageDesc.addSignalDescription(signalDesc);

    QTest::addRow("Invalid payload size")
            << messageDesc << uidDesc
            << QCanBusFrame(123, QByteArray(8, 0x01))
            << QCanFrameProcessor::Error::DecodingError
            << tr("Payload size does not match message description. "
                  "Actual size = 8, expected size = 2.")
            << QStringList()
            << QtCanBus::UniqueId(0)
            << QVariantMap();

    // format float, but length != 32
    QCanSignalDescription invalidSignalDesc;
    invalidSignalDesc.setName("invalidSignal");
    invalidSignalDesc.setStartBit(16);
    invalidSignalDesc.setBitLength(8);
    invalidSignalDesc.setDataFormat(QtCanBus::DataFormat::Float);

    messageDesc.setSize(8);
    messageDesc.addSignalDescription(invalidSignalDesc);

    QStringList expectedWarnings = { tr("Skipping signal invalidSignal in message with "
                                        "unique id 123 because its description is invalid.") };

    // Invalid signal skipped, and we still get results from other signals
    QTest::addRow("invalid signal desc warning")
            << messageDesc << uidDesc << QCanBusFrame(123, QByteArray(8, 0x01))
            << QCanFrameProcessor::Error::NoError << QString()
            << expectedWarnings << QtCanBus::UniqueId(123)
            << QVariantMap({ qMakePair("s0", 0x0101) });

    // other valid signal
    signalDesc.setName("s1");
    signalDesc.setStartBit(32);
    signalDesc.setBitLength(8);

    messageDesc.addSignalDescription(signalDesc);

    // valid signal, but length exceeds frame length
    signalDesc.setName("tooLong");
    signalDesc.setStartBit(40);
    signalDesc.setBitLength(32);

    messageDesc.addSignalDescription(signalDesc);

    expectedWarnings.push_back(tr("Skipping signal tooLong in message with unique id 123. "
                                  "Its start + length exceeds data length."));

    // Two invalid signals skipped, two valid results are received
    QTest::addRow("invalid signal desc warning")
            << messageDesc << uidDesc << QCanBusFrame(123, QByteArray(8, 0x01))
            << QCanFrameProcessor::Error::NoError << QString()
            << expectedWarnings << QtCanBus::UniqueId(123)
            << QVariantMap({ qMakePair("s0", 0x0101), qMakePair("s1", 0x01) });
}

void tst_QCanFrameProcessor::parseWithErrorsAndWarnings()
{
    QFETCH(QCanMessageDescription, messageDescription);
    QFETCH(QCanUniqueIdDescription, uniqueIdDescription);
    QFETCH(QCanBusFrame, frame);
    QFETCH(QCanFrameProcessor::Error, expectedError);
    QFETCH(QString, expectedErrorDescription);
    QFETCH(QStringList, expectedWarnings);
    QFETCH(QtCanBus::UniqueId, expectedUniqueId);
    QFETCH(QVariantMap, expectedResult);

    QCanFrameProcessor parser;
    parser.setUniqueIdDescription(uniqueIdDescription);
    parser.addMessageDescriptions({ messageDescription });

    const auto result = parser.parseFrame(frame);

    QCOMPARE(parser.error(), expectedError);
    QCOMPARE(parser.errorDescription(), expectedErrorDescription);

    // signal descriptions are stored as a QHash internally, so they are
    // processed in a hash-dependent order. Because of that the order of
    // warnings cannot be defined. So we sort them alphabetically before
    // comparing.
    std::sort(expectedWarnings.begin(), expectedWarnings.end());
    QStringList warnings = parser.warnings();
    std::sort(warnings.begin(), warnings.end());
    QCOMPARE(warnings, expectedWarnings);

    QCOMPARE(result.uniqueId, expectedUniqueId);

    // The values can be doubles, so we need to compare them explicitly
    const auto signalValues = result.signalValues;
    QCOMPARE(signalValues.keys(), expectedResult.keys());
    for (auto it = expectedResult.cbegin(); it != expectedResult.cend(); ++it) {
        const auto &expectedValue = it.value();
        const auto &key = it.key();
        if (expectedValue.canConvert(QMetaType::fromType<double>()))
            QCOMPARE(signalValues.value(key).toDouble(), expectedValue.toDouble());
        else
            QCOMPARE(signalValues.value(key), expectedValue);
    }
}

void tst_QCanFrameProcessor::extractUniqueId_data()
{
    QTest::addColumn<QCanUniqueIdDescription>("uniqueIdDescription");
    QTest::addColumn<QCanBusFrame>("frame");
    QTest::addColumn<QtCanBus::UniqueId>("expectedUniqueId");

    // unique id in frame id
    QCanUniqueIdDescription uidDesc;
    uidDesc.setSource(QtCanBus::DataSource::FrameId);
    uidDesc.setBitLength(12);

    const QtCanBus::UniqueId uniqueIdLe = 0x0965;
    const QtCanBus::UniqueId uniqueIdBe = 0x0596;
    for (quint16 startBit = 0; startBit < 8; ++startBit) {
        uidDesc.setStartBit(startBit);
        uidDesc.setEndian(QtCanBus::DataEndian::LittleEndian);
        const QCanBusFrame::FrameId frameId = qToLittleEndian(uniqueIdLe) << startBit;
        QTest::addRow("frameId, LE, offset %d", startBit)
                << uidDesc
                << QCanBusFrame(frameId, QByteArray(1, 0x01))
                << uniqueIdLe;

        uidDesc.setEndian(QtCanBus::DataEndian::BigEndian);
        QTest::addRow("frameId, BE, offset %d", startBit)
                << uidDesc
                << QCanBusFrame(frameId, QByteArray(1, 0x01))
                << uniqueIdBe;
    }

    // unique id in payload, the 1st byte of the payload is used for signal,
    // so we start from the 2nd byte
    uidDesc.setSource(QtCanBus::DataSource::Payload);
    for (quint16 startBit = 8; startBit < 16; ++startBit) {
        uidDesc.setStartBit(startBit);
        uidDesc.setEndian(QtCanBus::DataEndian::LittleEndian);
        const quint32 payload = 0x01 | (qToLittleEndian(uniqueIdLe) << startBit);
        const QByteArray payloadData(reinterpret_cast<const char *>(&payload), sizeof(payload));
        QTest::addRow("payload, LE, offset %d", startBit)
                << uidDesc
                << QCanBusFrame(0, payloadData)
                << uniqueIdLe;

        uidDesc.setEndian(QtCanBus::DataEndian::BigEndian);
        QTest::addRow("payload, BE, offset %d", startBit)
                << uidDesc
                << QCanBusFrame(0, payloadData)
                << uniqueIdBe;
    }
}

void tst_QCanFrameProcessor::extractUniqueId()
{
    QFETCH(QCanUniqueIdDescription, uniqueIdDescription);
    QFETCH(QCanBusFrame, frame);
    QFETCH(QtCanBus::UniqueId, expectedUniqueId);

    // Some valid message description.
    QCanSignalDescription signalDesc;
    signalDesc.setName("s0");
    signalDesc.setStartBit(0);
    signalDesc.setBitLength(8);

    QCanMessageDescription messageDesc;
    messageDesc.setName("test");
    messageDesc.setUniqueId(expectedUniqueId);
    messageDesc.setSize(frame.payload().size());
    messageDesc.addSignalDescription(signalDesc);

    QCanFrameProcessor processor;
    processor.setUniqueIdDescription(uniqueIdDescription);
    processor.setMessageDescriptions({ messageDesc });
    auto result = processor.parseFrame(frame);
    QCOMPARE(result.uniqueId, expectedUniqueId);
}

void tst_QCanFrameProcessor::prepareFrameId_data()
{
    QTest::addColumn<quint32>("initialFrameId");
    QTest::addColumn<quint16>("startBit");
    QTest::addColumn<quint16>("bitLength");
    QTest::addColumn<QtCanBus::DataFormat>("format");
    QTest::addColumn<QtCanBus::DataEndian>("endian");
    QTest::addColumn<QVariant>("signalValue");
    QTest::addColumn<quint32>("expectedFrameId");

    const quint32 initialFrameId = 1;

    // Every next value in the list is supposed to have a length in bits cut
    // by one.
    static constexpr quint16 leValues[] = { 0xCFC7, 0x67E3, 0x33F1, 0x19F8,
                                            0x0CFC, 0x067E, 0x033F, 0x019F };
    const quint16 startIdx = 4;
    for (quint16 i = 0; i < 8; ++i) {
        quint32 expectedFrameId = (leValues[i] << (startIdx + i)) | initialFrameId;
        QTest::addRow("int, le, start %d, length %d", (startIdx + i), 16 - i)
                << initialFrameId << quint16(startIdx + i) << quint16(16 - i)
                << QtCanBus::DataFormat::SignedInteger
                << QtCanBus::DataEndian::LittleEndian
                << QVariant(static_cast<qint16>(leValues[i]))
                << expectedFrameId;
        QTest::addRow("uint, le, start %d, length %d", (startIdx + i), 16 - i)
                << initialFrameId << quint16(startIdx + i) << quint16(16 - i)
                << QtCanBus::DataFormat::UnsignedInteger
                << QtCanBus::DataEndian::LittleEndian
                << QVariant(leValues[i])
                << expectedFrameId;

        const uchar *leValueData = reinterpret_cast<const uchar *>(&leValues[i]);
        const quint16 beValue = (0x00FF & (leValueData[1] << i)) + (leValueData[0] << 8);
        expectedFrameId = (beValue << startIdx) | initialFrameId;
        QTest::addRow("int, be, start %d, length %d", (startIdx + i), 16 - i)
                << initialFrameId << quint16(startIdx + i) << quint16(16 - i)
                << QtCanBus::DataFormat::SignedInteger
                << QtCanBus::DataEndian::BigEndian
                << QVariant(static_cast<qint16>(leValues[i]))
                << expectedFrameId;
        QTest::addRow("uint, be, start %d, length %d", (startIdx + i), 16 - i)
                << initialFrameId << quint16(startIdx + i) << quint16(16 - i)
                << QtCanBus::DataFormat::UnsignedInteger
                << QtCanBus::DataEndian::BigEndian
                << QVariant(leValues[i])
                << expectedFrameId;
    }
}

void tst_QCanFrameProcessor::prepareFrameId()
{
    QFETCH(quint32, initialFrameId);
    QFETCH(quint16, startBit);
    QFETCH(quint16, bitLength);
    QFETCH(QtCanBus::DataFormat, format);
    QFETCH(QtCanBus::DataEndian, endian);
    QFETCH(QVariant, signalValue);
    QFETCH(quint32, expectedFrameId);

    QCanSignalDescription signalDesc;
    signalDesc.setName("s0");
    signalDesc.setDataSource(QtCanBus::DataSource::FrameId);
    signalDesc.setDataFormat(format);
    signalDesc.setDataEndian(endian);
    signalDesc.setStartBit(startBit);
    signalDesc.setBitLength(bitLength);

    QCanUniqueIdDescription uidDesc;
    uidDesc.setBitLength(4); // see the test data -> first 4 bits never change

    QCanMessageDescription messageDesc;
    messageDesc.setUniqueId(initialFrameId);
    messageDesc.setSize(0);
    messageDesc.addSignalDescription(signalDesc);

    const QVariantMap signalValues = { qMakePair(signalDesc.name(), signalValue) };

    QCanFrameProcessor processor;
    processor.setUniqueIdDescription(uidDesc);
    processor.addMessageDescriptions({ messageDesc });
    const QCanBusFrame frame = processor.prepareFrame(initialFrameId, signalValues);

    QVERIFY(frame.isValid());
    QCOMPARE(frame.frameId(), expectedFrameId);
}

void tst_QCanFrameProcessor::preparePayload_data()
{
    QTest::addColumn<quint16>("startBit");
    QTest::addColumn<quint16>("bitLength");
    QTest::addColumn<QtCanBus::DataFormat>("format");
    QTest::addColumn<QtCanBus::DataEndian>("endian");
    QTest::addColumn<QVariant>("signalValue");
    QTest::addColumn<QByteArray>("expectedPayload");


    // (u)int, LE/BE, variable length and start
    // Every next value in the list is supposed to have a length in bits cut
    // by one.
    static constexpr quint16 leValues[] = { 0xCFC7, 0x67E3, 0x33F1, 0x19F8,
                                            0x0CFC, 0x067E, 0x033F, 0x019F };
    const quint16 startIdx = 4;
    for (quint16 i = 0; i < 8; ++i) {
        quint32 expectedPayloadData = (leValues[i] << (startIdx + i));
        QByteArray expectedPayload(reinterpret_cast<const char *>(&expectedPayloadData),
                                   sizeof(quint32));
        QTest::addRow("int, le, start %d, length %d", (startIdx + i), 16 - i)
                << quint16(startIdx + i) << quint16(16 - i)
                << QtCanBus::DataFormat::SignedInteger
                << QtCanBus::DataEndian::LittleEndian
                << QVariant(static_cast<qint16>(leValues[i]))
                << expectedPayload;
        QTest::addRow("uint, le, start %d, length %d", (startIdx + i), 16 - i)
                << quint16(startIdx + i) << quint16(16 - i)
                << QtCanBus::DataFormat::UnsignedInteger
                << QtCanBus::DataEndian::LittleEndian
                << QVariant(leValues[i])
                << expectedPayload;

        const quint16 beValue = qToBigEndian(leValues[i]);
        const uchar *beValueData = reinterpret_cast<const uchar *>(&beValue);
        quint32 beValueForPayload = ((beValueData[0] << i) + (beValueData[1] << 8)) << startIdx;
        expectedPayload = QByteArray(reinterpret_cast<const char*>(&beValueForPayload),
                                     sizeof(quint32));
        QTest::addRow("int, be, start %d, length %d", (startIdx + i), 16 - i)
                << quint16(startIdx + i) << quint16(16 - i)
                << QtCanBus::DataFormat::SignedInteger
                << QtCanBus::DataEndian::BigEndian
                << QVariant(static_cast<qint16>(leValues[i]))
                << expectedPayload;
        QTest::addRow("uint, be, start %d, length %d", (startIdx + i), 16 - i)
                << quint16(startIdx + i) << quint16(16 - i)
                << QtCanBus::DataFormat::UnsignedInteger
                << QtCanBus::DataEndian::BigEndian
                << QVariant(leValues[i])
                << expectedPayload;
    }

    // float, LE/BE, variable start
    const float floatVal = 123.456f;
    for (quint16 i = 0; i < 8; ++i) {
        quint64 valueForPayload = 0;
        const float leFloatVal = qToLittleEndian(floatVal);
        memcpy(&valueForPayload, &leFloatVal, sizeof(float));
        valueForPayload = valueForPayload << i;
        QByteArray expectedPayload(reinterpret_cast<const char*>(&valueForPayload),
                                   sizeof(quint64));
        QTest::addRow("float, le, from %d", i)
                << i << quint16(32)
                << QtCanBus::DataFormat::Float
                << QtCanBus::DataEndian::LittleEndian
                << QVariant(floatVal)
                << expectedPayload;

        valueForPayload = 0;
        const float beFloatVal = qToBigEndian(floatVal);
        memcpy(&valueForPayload, &beFloatVal, sizeof(float));
        valueForPayload = valueForPayload << i;
        expectedPayload = QByteArray(reinterpret_cast<const char*>(&valueForPayload),
                                     sizeof(quint64));
        QTest::addRow("float, be, from %d", i)
                << i << quint16(32)
                << QtCanBus::DataFormat::Float
                << QtCanBus::DataEndian::BigEndian
                << QVariant(floatVal)
                << expectedPayload;
    }

    // double, LE/BE
    {
        const double doubleVal = 1.234e-15;
        const double leDoubleVal = qToLittleEndian(doubleVal);
        QByteArray expectedPayload(reinterpret_cast<const char*>(&leDoubleVal), sizeof(double));
        QTest::addRow("double, le")
                << quint16(0) << quint16(64)
                << QtCanBus::DataFormat::Double
                << QtCanBus::DataEndian::LittleEndian
                << QVariant(doubleVal)
                << expectedPayload;

        const double beDoubleVal = qToBigEndian(doubleVal);
        expectedPayload = QByteArray(reinterpret_cast<const char*>(&beDoubleVal),
                                     sizeof(double));
        QTest::addRow("double, be")
                << quint16(0) << quint16(64)
                << QtCanBus::DataFormat::Double
                << QtCanBus::DataEndian::BigEndian
                << QVariant(doubleVal)
                << expectedPayload;
    }

    // ASCII
    {
        const QByteArray expectedData("abc", 3);
        const quint16 offset = 8; // start from the 2nd byte
        for (quint16 i = 0; i < 8; ++i) {
            quint64 valueForPayload = 0;
            memcpy(&valueForPayload, expectedData.constData(), expectedData.size());
            valueForPayload = valueForPayload << (offset + i);
            const QByteArray payload(reinterpret_cast<const char *>(&valueForPayload),
                                     sizeof(quint64));
            QTest::addRow("ascii, offset %d", offset + i)
                    << quint16(offset + i) << quint16(expectedData.size() * 8)
                    << QtCanBus::DataFormat::Ascii
                    << QtCanBus::DataEndian::LittleEndian
                    << QVariant(expectedData)
                    << payload;
        }
    }
    // ASCII - not enough data
    {
        const QByteArray data("ab", 2);
        QByteArray expectedPayload = data;
        expectedPayload.push_back('\0'); // ab\0

        QTest::addRow("ascii, not enough data")
                << quint16(0) << quint16(expectedPayload.size() * 8)
                << QtCanBus::DataFormat::Ascii
                << QtCanBus::DataEndian::LittleEndian
                << QVariant(data)
                << expectedPayload;
    }
    // ASCII - too much data
    {
        const QByteArray data("abcd", 4);
        const QByteArray expectedPayload = data.left(3); // abc

        QTest::addRow("ascii, too much data")
                << quint16(0) << quint16(expectedPayload.size() * 8)
                << QtCanBus::DataFormat::Ascii
                << QtCanBus::DataEndian::LittleEndian
                << QVariant(data)
                << expectedPayload;
    }
}

void tst_QCanFrameProcessor::preparePayload()
{
    QFETCH(quint16, startBit);
    QFETCH(quint16, bitLength);
    QFETCH(QtCanBus::DataFormat, format);
    QFETCH(QtCanBus::DataEndian, endian);
    QFETCH(QVariant, signalValue);
    QFETCH(QByteArray, expectedPayload);

    QCanSignalDescription signalDesc;
    signalDesc.setName("s0");
    signalDesc.setDataFormat(format);
    signalDesc.setDataEndian(endian);
    signalDesc.setStartBit(startBit);
    signalDesc.setBitLength(bitLength);

    const QtCanBus::UniqueId uniqueId = 123;

    QCanMessageDescription messageDesc;
    messageDesc.setUniqueId(uniqueId);
    messageDesc.setSize(expectedPayload.size());
    messageDesc.addSignalDescription(signalDesc);

    // Assume that our uniqueId is the whole FrameId
    QCanUniqueIdDescription uidDesc;
    uidDesc.setBitLength(29);

    const QVariantMap signalValues = { qMakePair(signalDesc.name(), signalValue) };

    QCanFrameProcessor processor;
    processor.setUniqueIdDescription(uidDesc);
    processor.addMessageDescriptions({ messageDesc });
    const QCanBusFrame frame = processor.prepareFrame(uniqueId, signalValues);

    QVERIFY(frame.isValid());
    QCOMPARE(frame.frameId(), uniqueId);
    QCOMPARE(frame.payload(), expectedPayload);
}

void tst_QCanFrameProcessor::prepareWithValueConversion_data()
{
    QTest::addColumn<double>("factor");
    QTest::addColumn<double>("offset");
    QTest::addColumn<double>("scaling");
    QTest::addColumn<QVariant>("signalValue");
    QTest::addColumn<QByteArray>("expectedPayload");

    // payloadValue = (signalValue / scaling - offset) / factor;
    QTest::newRow("nan, nan, nan") << qQNaN() << qQNaN() << qQNaN() << QVariant(30) << QByteArray(1, 30);
    QTest::newRow("0.75, nan, nan") << 0.75 << qQNaN() << qQNaN() << QVariant(22.5) << QByteArray(1, 30);
    QTest::newRow("0.75, 15.3, nan") << 0.75 << 15.3 << qQNaN() << QVariant(37.8) << QByteArray(1, 30);
    QTest::newRow("0.75, 15.3, 100") << 0.75 << 15.3 << 100. << QVariant(3780.) << QByteArray(1, 30);
    QTest::newRow("0.75, nan, 100") << 0.75 << qQNaN() << 100. << QVariant(2250.) << QByteArray(1, 30);
    QTest::newRow("nan, 15.3, 100") << qQNaN() << 15.3 << 100. << QVariant(4530.) << QByteArray(1, 30);
    // check that setting factor and/or scaling to 0 is equivalent to NaN
    QTest::newRow("0, 15.3, 100") << 0. << 15.3 << 100. << QVariant(4530.) << QByteArray(1, 30);
    QTest::newRow("0, 15.3, 0") << 0. << 15.3 << 0. << QVariant(45.3) << QByteArray(1, 30);
}

void tst_QCanFrameProcessor::prepareWithValueConversion()
{
    QFETCH(double, factor);
    QFETCH(double, offset);
    QFETCH(double, scaling);
    QFETCH(QVariant, signalValue);
    QFETCH(QByteArray, expectedPayload);

    QCanSignalDescription signalDesc;
    signalDesc.setName("s0");
    signalDesc.setStartBit(0);
    signalDesc.setBitLength(8);
    signalDesc.setFactor(factor);
    signalDesc.setOffset(offset);
    signalDesc.setScaling(scaling);

    const QtCanBus::UniqueId uniqueId = 123;

    QCanMessageDescription messageDesc;
    messageDesc.setUniqueId(uniqueId);
    messageDesc.setSize(expectedPayload.size());
    messageDesc.addSignalDescription(signalDesc);

    // Assume that our uniqueId is the whole FrameId
    QCanUniqueIdDescription uidDesc;
    uidDesc.setBitLength(29);

    const QVariantMap signalValues = { qMakePair(signalDesc.name(), signalValue) };

    QCanFrameProcessor processor;
    processor.setUniqueIdDescription(uidDesc);
    processor.addMessageDescriptions({ messageDesc });
    const QCanBusFrame frame = processor.prepareFrame(uniqueId, signalValues);

    QVERIFY(frame.isValid());
    QCOMPARE(frame.frameId(), uniqueId);
    QCOMPARE(frame.payload(), expectedPayload);
}

void tst_QCanFrameProcessor::prepareMultiplexedPayload_data()
{
    QTest::addColumn<QByteArray>("expectedPayload");
    QTest::addColumn<QVariantMap>("signalValues");

    const QString s0Name("s0");
    const QString s1Name("s1");
    const QString s2Name("s2");
    const QString s3Name("s3");
    const QString s4Name("s4");

    QTest::newRow("s0 and s2") << QByteArray::fromHex("2334")
                               << QVariantMap({ qMakePair(s0Name, 3),
                                                qMakePair(s2Name, 0x342) });

    QTest::newRow("s0, s1 and s3") << QByteArray::fromHex("1134")
                                   << QVariantMap({ qMakePair(s0Name, 1),
                                                    qMakePair(s1Name, 1),
                                                    qMakePair(s3Name, 0x34) });

    QTest::newRow("s0, s1 and s4") << QByteArray::fromHex("5134")
                                   << QVariantMap({ qMakePair(s0Name, 1),
                                                    qMakePair(s1Name, 5),
                                                    qMakePair(s4Name, 0x34) });

    QTest::newRow("s0 and s1") << QByteArray::fromHex("3100")
                               << QVariantMap({ qMakePair(s0Name, 1),
                                                qMakePair(s1Name, 3) });

    QTest::newRow("only s0") << QByteArray::fromHex("0300")
                             << QVariantMap({ qMakePair(s0Name, 3) });
}

void tst_QCanFrameProcessor::prepareMultiplexedPayload()
{
    QFETCH(QByteArray, expectedPayload);
    QFETCH(QVariantMap, signalValues);

    QCanSignalDescription s0; // multiplexor 1
    s0.setName("s0");
    s0.setDataEndian(QtCanBus::DataEndian::LittleEndian);
    s0.setStartBit(0);
    s0.setBitLength(4);
    s0.setMultiplexState(QtCanBus::MultiplexState::MultiplexorSwitch);

    QCanSignalDescription s1; // multiplexor 2, also depends on the value of s0
    s1.setName("s1");
    s1.setDataEndian(QtCanBus::DataEndian::LittleEndian);
    s1.setStartBit(4);
    s1.setBitLength(4);
    s1.setMultiplexState(QtCanBus::MultiplexState::SwitchAndSignal);
    s1.addMultiplexSignal(s0.name(), 1);

    QCanSignalDescription s2; // depends on mux1 only, contains 12 bits of data
    s2.setName("s2");
    s2.setDataEndian(QtCanBus::DataEndian::LittleEndian);
    s2.setStartBit(4);
    s2.setBitLength(12);
    s2.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    const QCanSignalDescription::MultiplexValues muxValuesS0 { qMakePair(2, 4) };
    s2.addMultiplexSignal(s0.name(), muxValuesS0);

    QCanSignalDescription s3; // depends on mux2 (so indirectly on mux1)
    s3.setName("s3");
    s3.setDataEndian(QtCanBus::DataEndian::LittleEndian);
    s3.setStartBit(8);
    s3.setBitLength(8);
    s3.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    s3.addMultiplexSignal(s1.name(), 1);

    QCanSignalDescription s4; // depends on mux2 (so indirectly on mux1)
    s4.setName("s4");
    s4.setDataEndian(QtCanBus::DataEndian::LittleEndian);
    s4.setStartBit(8);
    s4.setBitLength(8);
    s4.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    const QCanSignalDescription::MultiplexValues muxValuesS1 { qMakePair(2, 3), qMakePair(5, 5) };
    s4.addMultiplexSignal(s1.name(), muxValuesS1);

    const QtCanBus::UniqueId uniqueId = 123;

    QCanMessageDescription msg;
    msg.setName("test");
    msg.setUniqueId(uniqueId);
    msg.setSize(expectedPayload.size());
    msg.setSignalDescriptions({ s0, s1, s2, s3, s4 });

    // Assume that our uniqueId is the whole FrameId
    QCanUniqueIdDescription uidDesc;
    uidDesc.setBitLength(29);

    QCanFrameProcessor processor;
    processor.setUniqueIdDescription(uidDesc);
    processor.addMessageDescriptions({ msg });

    const QCanBusFrame frame = processor.prepareFrame(msg.uniqueId(), signalValues);

    QVERIFY(frame.isValid());
    QCOMPARE(frame.frameId(), uniqueId);
    QCOMPARE(frame.payload(), expectedPayload);
}

void tst_QCanFrameProcessor::prepareWithErrorsAndWarnings_data()
{
    QTest::addColumn<QCanMessageDescription>("messageDescription");
    QTest::addColumn<QCanUniqueIdDescription>("uniqueIdDescription");
    QTest::addColumn<QtCanBus::UniqueId>("uniqueId");
    QTest::addColumn<QVariantMap>("signalValues");
    QTest::addColumn<QCanFrameProcessor::Error>("expectedError");
    QTest::addColumn<QString>("expectedErrorDescription");
    QTest::addColumn<QStringList>("expectedWarnings");
    QTest::addColumn<QCanBusFrame::FrameId>("expectedFrameId");
    QTest::addColumn<QByteArray>("expectedPayload");

    QTest::addRow("no UID description")
            << QCanMessageDescription() << QCanUniqueIdDescription()
            << QtCanBus::UniqueId(0) << QVariantMap()
            << QCanFrameProcessor::Error::EncodingError
            << tr("No valid unique identifier description is specified.")
            << QStringList() << QCanBusFrame::FrameId(0) << QByteArray();

    QCanUniqueIdDescription uidDesc;
    uidDesc.setBitLength(29);

    QTest::addRow("no message description")
            << QCanMessageDescription() << uidDesc << QtCanBus::UniqueId(123)
            << QVariantMap() << QCanFrameProcessor::Error::EncodingError
            << tr("Failed to find message description for unique id 123.")
            << QStringList() << QCanBusFrame::FrameId(0) << QByteArray();

    QCanSignalDescription signalDesc;
    signalDesc.setName("s0");
    signalDesc.setStartBit(0);
    signalDesc.setBitLength(4);

    QCanMessageDescription messageDesc;
    messageDesc.setUniqueId(123);
    messageDesc.setSize(2);
    messageDesc.addSignalDescription(signalDesc);

    uidDesc.setSource(QtCanBus::DataSource::Payload);
    uidDesc.setStartBit(8);
    uidDesc.setBitLength(16); // does not fit into the payload

    QTest::addRow("set UID fail")
            << messageDesc << uidDesc << messageDesc.uniqueId()
            << QVariantMap() << QCanFrameProcessor::Error::EncodingError
            << tr("Failed to encode unique id 123 into the frame")
            << QStringList() << QCanBusFrame::FrameId(0) << QByteArray();

    // reset uidDesc
    uidDesc = QCanUniqueIdDescription();
    uidDesc.setBitLength(29);

    QStringList warnings = { tr("Skipping signal s1. It is not found in message "
                                "description for unique id 123.") };

    QTest::addRow("no signal description")
            << messageDesc << uidDesc << messageDesc.uniqueId()
            << QVariantMap({ qMakePair("s0", 1), qMakePair("s1", 2) })
            << QCanFrameProcessor::Error::NoError << QString()
            << warnings << QCanBusFrame::FrameId(messageDesc.uniqueId())
            << QByteArray::fromHex("0100");

    // invalid signal desc - 0 length
    signalDesc.setName("s2");
    signalDesc.setStartBit(4);
    signalDesc.setBitLength(0);

    messageDesc.addSignalDescription(signalDesc);

    warnings.push_back(tr("Skipping signal s2. Its description is invalid."));

    QTest::addRow("invalid signal description")
            << messageDesc << uidDesc << messageDesc.uniqueId()
            << QVariantMap({ qMakePair("s0", 1), qMakePair("s1", 2), qMakePair("s2", 3) })
            << QCanFrameProcessor::Error::NoError << QString()
            << warnings << QCanBusFrame::FrameId(messageDesc.uniqueId())
            << QByteArray::fromHex("0100");

    // start + length exceed data length
    signalDesc.setName("s3");
    signalDesc.setStartBit(8);
    signalDesc.setBitLength(10);

    messageDesc.addSignalDescription(signalDesc);

    warnings.push_back(tr("Skipping signal s3. Its start + length exceeds the "
                          "expected message length."));

    QTest::addRow("invalid signal length")
            << messageDesc << uidDesc << messageDesc.uniqueId()
            << QVariantMap({ qMakePair("s0", 1), qMakePair("s1", 2),
                             qMakePair("s2", 3), qMakePair("s3", 4) })
            << QCanFrameProcessor::Error::NoError << QString()
            << warnings << QCanBusFrame::FrameId(messageDesc.uniqueId())
            << QByteArray::fromHex("0100");

    // Test that multiplexed signals are used only when proper multiplexor
    // values are provided.
    messageDesc.clearSignalDescriptions();

    // first multiplexor
    signalDesc.setName("s0");
    signalDesc.setMultiplexState(QtCanBus::MultiplexState::MultiplexorSwitch);
    signalDesc.setStartBit(0);
    signalDesc.setBitLength(4);
    messageDesc.addSignalDescription(signalDesc);

    // s1 is used only when s0 is in range [2, 4] or 6
    signalDesc.setName("s1");
    signalDesc.setStartBit(4);
    signalDesc.setBitLength(12);
    signalDesc.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    const QCanSignalDescription::MultiplexValues muxValuesS0 { qMakePair(2, 4), qMakePair(6, 6) };
    signalDesc.addMultiplexSignal("s0", muxValuesS0);
    messageDesc.addSignalDescription(signalDesc);

    // second multiplexor, used when s0 == 1
    signalDesc.setName("s2");
    signalDesc.setStartBit(4);
    signalDesc.setBitLength(4);
    signalDesc.setMultiplexState(QtCanBus::MultiplexState::SwitchAndSignal);
    signalDesc.addMultiplexSignal("s0", 1);
    messageDesc.addSignalDescription(signalDesc);

    // s3 depends on two multiplexors
    signalDesc.setName("s3");
    signalDesc.setStartBit(8);
    signalDesc.setBitLength(8);
    signalDesc.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    signalDesc.addMultiplexSignal("s0", 1);
    const QCanSignalDescription::MultiplexValues muxValuesS2 { qMakePair(1, 3), qMakePair(5, 6) };
    signalDesc.addMultiplexSignal("s2", muxValuesS2);
    messageDesc.addSignalDescription(signalDesc);

    // s4 depends on two multiplexors
    signalDesc.setName("s4");
    signalDesc.setStartBit(8);
    signalDesc.setBitLength(8);
    signalDesc.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    signalDesc.addMultiplexSignal("s0", 1);
    signalDesc.addMultiplexSignal("s2", 4);
    messageDesc.addSignalDescription(signalDesc);

    warnings.clear();
    warnings.push_back(tr("Skipped signal s1. Proper multiplexor values not found."));
    warnings.push_back(tr("Skipped signal s4. Proper multiplexor values not found."));

    QTest::addRow("multiplexed signals")
            << messageDesc << uidDesc << messageDesc.uniqueId()
            << QVariantMap({ qMakePair("s0", 1), qMakePair("s1", 10), qMakePair("s2", 2),
                             qMakePair("s3", 5), qMakePair("s4", 6) })
            << QCanFrameProcessor::Error::NoError << QString()
            << warnings << QCanBusFrame::FrameId(messageDesc.uniqueId())
            << QByteArray::fromHex("2105");
}

void tst_QCanFrameProcessor::prepareWithErrorsAndWarnings()
{
    QFETCH(QCanMessageDescription, messageDescription);
    QFETCH(QCanUniqueIdDescription, uniqueIdDescription);
    QFETCH(QtCanBus::UniqueId, uniqueId);
    QFETCH(QVariantMap, signalValues);
    QFETCH(QCanFrameProcessor::Error, expectedError);
    QFETCH(QString, expectedErrorDescription);
    QFETCH(QStringList, expectedWarnings);
    QFETCH(QCanBusFrame::FrameId, expectedFrameId);
    QFETCH(QByteArray, expectedPayload);

    QCanFrameProcessor processor;
    processor.setUniqueIdDescription(uniqueIdDescription);
    processor.setMessageDescriptions({ messageDescription });

    const QCanBusFrame frame = processor.prepareFrame(uniqueId, signalValues);

    QCOMPARE(processor.error(), expectedError);
    QCOMPARE(processor.errorDescription(), expectedErrorDescription);
    // signal descriptions are stored as a QHash internally, so they are
    // processed in a hash-dependent order. Because of that the order of
    // warnings cannot be defined. So we sort them alphabetically before
    // comparing.
    std::sort(expectedWarnings.begin(), expectedWarnings.end());
    QStringList warnings = processor.warnings();
    std::sort(warnings.begin(), warnings.end());
    QCOMPARE(warnings, expectedWarnings);

    QCOMPARE(frame.frameId(), expectedFrameId);
    QCOMPARE(frame.payload(), expectedPayload);
}

void tst_QCanFrameProcessor::prepareUniqueId_data()
{
    QTest::addColumn<QCanUniqueIdDescription>("uniqueIdDescription");
    QTest::addColumn<QCanMessageDescription>("messageDescription");
    QTest::addColumn<QtCanBus::UniqueId>("uniqueId");
    QTest::addColumn<QVariantMap>("signalValues");
    QTest::addColumn<QCanBusFrame>("expectedFrame");

    const QtCanBus::UniqueId uniqueIdLe = 0x0965;
    const QtCanBus::UniqueId uniqueIdBe = 0x0596;

    QCanSignalDescription signalDesc;
    signalDesc.setName("s0");
    signalDesc.setDataSource(QtCanBus::DataSource::Payload);
    signalDesc.setDataEndian(QtCanBus::DataEndian::LittleEndian);
    signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
    signalDesc.setStartBit(0);
    signalDesc.setBitLength(8);

    QCanMessageDescription messageDesc;
    messageDesc.setName("test");
    messageDesc.setSize(1);
    messageDesc.addSignalDescription(signalDesc);

    const QVariantMap signalValues { qMakePair("s0", 1) };

    // Encode unique id into frame id.
    QCanUniqueIdDescription uidDesc;
    uidDesc.setSource(QtCanBus::DataSource::FrameId);
    uidDesc.setBitLength(12);
    for (quint8 startBit = 0; startBit < 8; ++startBit) {
        messageDesc.setUniqueId(uniqueIdLe);
        uidDesc.setStartBit(startBit);
        uidDesc.setEndian(QtCanBus::DataEndian::LittleEndian);
        const QCanBusFrame::FrameId frameId = qFromLittleEndian(uniqueIdLe) << startBit;
        QTest::addRow("frameId, LE, offser %d", startBit)
                << uidDesc << messageDesc << uniqueIdLe << signalValues
                << QCanBusFrame(frameId, QByteArray(1, 0x01));

        messageDesc.setUniqueId(uniqueIdBe);
        uidDesc.setEndian(QtCanBus::DataEndian::BigEndian);
        QTest::addRow("frameId, BE, offser %d", startBit)
                << uidDesc << messageDesc << uniqueIdBe << signalValues
                << QCanBusFrame(frameId, QByteArray(1, 0x01));
    }

    // Encode unique id into payload. The 1st byte of the payload is already
    // populated with a signal value, so we start from the 2nd byte
    uidDesc.setSource(QtCanBus::DataSource::Payload);
    messageDesc.setSize(sizeof(quint32));
    for (quint8 startBit = 8; startBit < 16; ++startBit) {
        messageDesc.setUniqueId(uniqueIdLe);
        uidDesc.setStartBit(startBit);
        uidDesc.setEndian(QtCanBus::DataEndian::LittleEndian);
        const quint32 payload = 0x01 | qFromLittleEndian(uniqueIdLe) << startBit;
        const QByteArray payloadData(reinterpret_cast<const char *>(&payload), sizeof(payload));
        QTest::addRow("payload, LE, offser %d", startBit)
                << uidDesc << messageDesc << uniqueIdLe << signalValues
                << QCanBusFrame(0, payloadData);

        messageDesc.setUniqueId(uniqueIdBe);
        uidDesc.setEndian(QtCanBus::DataEndian::BigEndian);
        QTest::addRow("payload, BE, offser %d", startBit)
                << uidDesc << messageDesc << uniqueIdBe << signalValues
                << QCanBusFrame(0, payloadData);
    }
}

void tst_QCanFrameProcessor::prepareUniqueId()
{
    QFETCH(QCanUniqueIdDescription, uniqueIdDescription);
    QFETCH(QCanMessageDescription, messageDescription);
    QFETCH(QtCanBus::UniqueId, uniqueId);
    QFETCH(QVariantMap, signalValues);
    QFETCH(QCanBusFrame, expectedFrame);

    QCanFrameProcessor processor;
    processor.setUniqueIdDescription(uniqueIdDescription);
    processor.setMessageDescriptions({ messageDescription });

    const auto result = processor.prepareFrame(uniqueId, signalValues);
    QCOMPARE(processor.error(), QCanFrameProcessor::Error::NoError);
    QCOMPARE(result.frameId(), expectedFrame.frameId());
    QCOMPARE(result.payload(), expectedFrame.payload());
}

QTEST_MAIN(tst_QCanFrameProcessor)

#include "tst_qcanframeprocessor.moc"
