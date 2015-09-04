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
#include <QtSerialBus/QModBusDataUnit>

class tst_QModBusDataUnit : public QObject
{
    Q_OBJECT
public:
    explicit tst_QModBusDataUnit();

private slots:
    void constructors();
    void setters();
};

tst_QModBusDataUnit::tst_QModBusDataUnit()
{
}

void tst_QModBusDataUnit::constructors()
{
    QModBusDataUnit unit1(QModBusDevice::Coils);
    QModBusDataUnit unit2(QModBusDevice::HoldingRegisters, 3, 9);

    QCOMPARE(unit1.tableType(), QModBusDevice::Coils);
    QCOMPARE(unit1.address(), 0);
    quint16 value = 0;
    QCOMPARE(unit1.value(), value);

    QCOMPARE(unit2.tableType(), QModBusDevice::HoldingRegisters);
    QCOMPARE(unit2.address(), 3);
    value = 9;
    QCOMPARE(unit2.value(), value);
}

void tst_QModBusDataUnit::setters()
{
    QModBusDataUnit unit(QModBusDevice::HoldingRegisters, 3, 9);
    unit.setTableType(QModBusDevice::InputRegisters);
    unit.setAddress(2);
    unit.setValue(5);

    QCOMPARE(unit.tableType(), QModBusDevice::InputRegisters);
    QCOMPARE(unit.address(), 2);
    quint16 value = 5;
    QCOMPARE(unit.value(), value);
}

QTEST_MAIN(tst_QModBusDataUnit)

#include "tst_qmodbusdataunit.moc"
