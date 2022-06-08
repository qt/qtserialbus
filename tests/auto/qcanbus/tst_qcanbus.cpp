// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtSerialBus/qcanbus.h>
#include <QtSerialBus/qcanbusfactory.h>

#include <QtTest/qtest.h>
#include <QtCore/QtPlugin>

class tst_QCanBus : public QObject
{
    Q_OBJECT
public:
    explicit tst_QCanBus();

private slots:
    void initTestCase();
    void plugins();
    void interfaces();
    void createDevice();

private:
    QCanBus *bus = nullptr;
};

tst_QCanBus::tst_QCanBus()
{
}

void tst_QCanBus::initTestCase()
{
#if QT_CONFIG(library)
    /*
     * Set custom path since CI doesn't install test plugins
     */
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath()
                                     + QStringLiteral("/../../../plugins"));
#ifdef Q_OS_WIN
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath()
                                     + QStringLiteral("/../../../../plugins"));
#endif
#endif // QT_CONFIG(library)
    bus = QCanBus::instance();
    QVERIFY(bus);
    QCanBus *sameInstance = QCanBus::instance();
    QCOMPARE(bus, sameInstance);
}

void tst_QCanBus::plugins()
{
    const QStringList pluginList = bus->plugins();
    QVERIFY(!pluginList.isEmpty());
    QVERIFY(pluginList.contains("testcan"));
}

void tst_QCanBus::interfaces()
{
    const QList<QCanBusDeviceInfo> pluginList = bus->availableDevices("testcan");
    QCOMPARE(pluginList.size(), 1);
    QCOMPARE(pluginList.at(0).plugin(), QStringLiteral("testcan"));
    QCOMPARE(pluginList.at(0).name(), QStringLiteral("can0"));
    QVERIFY(pluginList.at(0).isVirtual());
    QVERIFY(pluginList.at(0).hasFlexibleDataRate());
}

void tst_QCanBus::createDevice()
{
    QString error, error2;
    QCanBusDevice *testcan = bus->createDevice("testcan", "unused");
    QCanBusDevice *testcan2 = bus->createDevice("testcan", "unused");
    QCanBusDevice *faulty = bus->createDevice("testcan", "invalid", &error);
    QCanBusDevice *faulty2 = bus->createDevice("faulty", "faulty", &error2);
    QVERIFY(testcan);
    QVERIFY(testcan2);

    QVERIFY(!faulty);
    QCOMPARE(error, tr("No such interface: 'invalid'"));

    QVERIFY(!faulty2);
    QCOMPARE(error2, tr("No such plugin: 'faulty'"));

    delete testcan;
    delete testcan2;
}

QTEST_MAIN(tst_QCanBus)
Q_IMPORT_PLUGIN(TestCanBusPlugin)

#include "tst_qcanbus.moc"
