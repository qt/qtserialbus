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

#include <QtSerialBus/qcanbus.h>

#include <QtTest/QtTest>

class tst_QCanBus : public QObject
{
    Q_OBJECT
public:
    explicit tst_QCanBus();

private slots:
    void initTestCase();
    void plugins();
    void createBackend();

private:
    QCanBus *bus;
};

tst_QCanBus::tst_QCanBus() :
    bus(0)
{
}

void tst_QCanBus::initTestCase()
{
    bus = QCanBus::instance();
    QVERIFY(bus != 0);
    QCanBus *sameInstance;
    sameInstance = QCanBus::instance();
    QCOMPARE(bus, sameInstance);
}

void tst_QCanBus::plugins()
{
    QVERIFY(!bus->plugins().isEmpty());
    QVERIFY(bus->plugins().contains("generic"));

}

void tst_QCanBus::createBackend()
{
    //TODO this test needs serious overhaul. For now it is the bare minimum

    QCanBusDevice *dummy = bus->createDevice("generic", "unused");
    QVERIFY(dummy);

    QCanBusDevice *dummy2 = bus->createDevice("generic", "unused");
    QVERIFY(dummy2);
    QVERIFY(dummy != dummy2);
}

QTEST_MAIN(tst_QCanBus)

#include "tst_qcanbus.moc"
