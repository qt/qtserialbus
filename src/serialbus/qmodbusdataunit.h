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

#ifndef QMODBUSDATAUNIT_H
#define QMODBUSDATAUNIT_H

#include <QtCore/qmap.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qvector.h>

QT_BEGIN_NAMESPACE

class QModbusDataUnit
{
public:
    enum RegisterType {
        Invalid,
        DiscreteInputs,
        Coils,
        InputRegisters,
        HoldingRegisters
    };

    QModbusDataUnit() Q_DECL_EQ_DEFAULT;

    explicit QModbusDataUnit(RegisterType regType)
        : QModbusDataUnit(regType, 0, 0)
    {}

    QModbusDataUnit(RegisterType regType, int dataAddress, quint16 initSize)
        : QModbusDataUnit(regType, dataAddress, QVector<quint16>(initSize))
    {}

    QModbusDataUnit(RegisterType regType, int newStartAddress, const QVector<quint16> &newData)
        : rType(regType)
        , sAddress(newStartAddress)
        , dataValue(newData)
        , dataRange(newData.size())
    {}

    RegisterType registerType() const { return rType; }
    void setRegisterType(RegisterType newRegisterType)
    {
        rType = newRegisterType;
    }

    inline int startAddress() const { return sAddress; }
    inline void setStartAddress(int newAddress) { sAddress = newAddress; }

    inline QVector<quint16> values() const { return dataValue; }
    inline void setValues(const QVector<quint16> &newValue)
    {
        dataValue = newValue;
        dataRange = newValue.size();
    }

    // TODO: Maybe introduce Range.
    inline int valueCount() const { return dataRange; }
    inline void setValueCount(int newCount) { dataRange = newCount; }

    inline void setValue(int index, quint16 newValue)
    {
        if (dataValue.isEmpty() || index >= dataValue.size())
            return;
        dataValue[index] = newValue;
    }
    inline quint16 value(int index) const { return dataValue.value(index); }

    // TODO: Do we really need the next two functions and the 'Invalid' enum value.
    inline void reset() {
        rType = Invalid;
        sAddress = -1;
        dataRange = 0;
        dataValue = {};
    }
    bool isValid() const { return rType != Invalid && sAddress != -1; }

private:
    RegisterType rType = Invalid;
    int sAddress = -1;
    QVector<quint16> dataValue;
    int dataRange = 0;
};
typedef QMap<QModbusDataUnit::RegisterType, QModbusDataUnit> QModbusDataUnitMap;

Q_DECLARE_TYPEINFO(QModbusDataUnit, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(QModbusDataUnit::RegisterType, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QModbusDataUnit::RegisterType)

#endif // QMODBUSDATAUNIT_H
