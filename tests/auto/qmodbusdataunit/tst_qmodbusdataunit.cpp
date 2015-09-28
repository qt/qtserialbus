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
#include <QtSerialBus/QModbusDataUnit>

class tst_QModbusDataUnit : public QObject
{
    Q_OBJECT
public:
    explicit tst_QModbusDataUnit();

private slots:
    void constructors();
    void setters();
};

tst_QModbusDataUnit::tst_QModbusDataUnit()
{
}

void tst_QModbusDataUnit::constructors()
{
    QModbusDataUnit unit1(QModbusRegister::Coils);

    QCOMPARE(unit1.registerType(), QModbusRegister::Coils);
    QCOMPARE(unit1.startAddress(), 0);
    QCOMPARE(unit1.valueCount(), 0);
    QVERIFY(unit1.values().isEmpty());

    QModbusDataUnit unit2(QModbusRegister::HoldingRegisters, 3, 9);
    QCOMPARE(unit2.registerType(), QModbusRegister::HoldingRegisters);
    QCOMPARE(unit2.startAddress(), 3);
    QCOMPARE(unit2.values().size(), 1);
    QCOMPARE(unit2.values().at(0), (quint16) 9);
    QCOMPARE(unit2.valueCount(), 1);

    QVector<quint16> data;
    data.append(5);
    data.append(6);
    data.append(7);

    QModbusDataUnit unit3(QModbusRegister::InputRegisters, 2, data);
    QCOMPARE(unit3.registerType(), QModbusRegister::InputRegisters);
    QCOMPARE(unit3.startAddress(), 2);
    QCOMPARE(unit3.values().size(), 3);
    QCOMPARE(unit3.values().at(0), (quint16) 5);
    QCOMPARE(unit3.values().at(1), (quint16) 6);
    QCOMPARE(unit3.values().at(2), (quint16) 7);
    QCOMPARE(unit3.valueCount(), 3);
}

void tst_QModbusDataUnit::setters()
{
    QModbusDataUnit unit(QModbusRegister::HoldingRegisters, 3, 9);
    QCOMPARE(unit.valueCount(), 1);

    unit.setRegisterType(QModbusRegister::InputRegisters);
    unit.setStartAddress(2);
    QVector<quint16> data;
    data.append(9u);
    data.append(5u);
    unit.setValues(data);

    QCOMPARE(unit.registerType(), QModbusRegister::InputRegisters);
    QCOMPARE(unit.startAddress(), 2);
    QCOMPARE(unit.valueCount(), 2);
    QCOMPARE(unit.values().size(), 2);
    QCOMPARE(unit.values().at(0), (quint16) 9);
    QCOMPARE(unit.values().at(1), (quint16) 5);

    //valueCount can be adjusted but values() stays the same
    unit.setValueCount(1);
    QCOMPARE(unit.valueCount(), 1);
    QCOMPARE(unit.values().size(), 2);
    QCOMPARE(unit.values().at(0), (quint16) 9);
    QCOMPARE(unit.values().at(1), (quint16) 5);
}

QTEST_MAIN(tst_QModbusDataUnit)

#include "tst_qmodbusdataunit.moc"
