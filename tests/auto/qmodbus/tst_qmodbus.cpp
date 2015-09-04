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

#include <QtSerialBus/qmodbus.h>

#include <QtTest/QtTest>

class tst_QModBus : public QObject
{
    Q_OBJECT
public:
    tst_QModBus();

private slots:
    void initTestCase();
    void plugins();
    void createBackend();
private:
    QModBus *bus;
};

tst_QModBus::tst_QModBus() : bus(0)
{
}

void tst_QModBus::initTestCase()
{
    /*
     * Set custom path since CI doesn't install test plugins
     */
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath()
                                     + QStringLiteral("/../../../plugins"));
#ifdef Q_OS_WIN
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath()
                                     + QStringLiteral("/../../../../plugins"));
#endif
    bus = QModBus::instance();
    QVERIFY(bus != 0);
    QModBus *sameInstance;
    sameInstance = QModBus::instance();
    QCOMPARE(bus, sameInstance);
}

void tst_QModBus::plugins()
{
    QCOMPARE(bus->plugins().isEmpty(), false);
    QCOMPARE(bus->plugins().contains("generic"), true);
}

void tst_QModBus::createBackend()
{
    QVERIFY(bus->createSlave("foo") == Q_NULLPTR);
    QVERIFY(bus->createMaster("foo") == Q_NULLPTR);

    QVERIFY(bus->createSlave("generic") != Q_NULLPTR);
    QVERIFY(bus->createMaster("generic") != Q_NULLPTR);
}

QTEST_MAIN(tst_QModBus)

#include "tst_qmodbus.moc"
