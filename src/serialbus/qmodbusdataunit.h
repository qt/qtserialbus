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

#include <QtSerialBus/qmodbusregister.h>

#include <QtCore/qvector.h>

QT_BEGIN_NAMESPACE

class QModBusDataUnit
{
public:
    QModBusDataUnit(QModBusRegister::RegisterType regType,
                             int dataAddress, quint16 initValue)
        : rType(regType),
          sAddress(dataAddress),
          dataRange(1)
    {
        dataValue.fill(initValue, 1);
    }

    QModBusDataUnit(QModBusRegister::RegisterType regType,
                             int newStartAddress, const QVector<quint16> &data)
        : rType(regType),
          sAddress(newStartAddress),
          dataValue(data),
          dataRange(data.size())
    {

    }

    explicit QModBusDataUnit(QModBusRegister::RegisterType regType)
        : rType(regType),
          sAddress(0),
          dataRange(0)
    {
    }

    QModBusRegister::RegisterType registerType() const { return rType; }
    void setRegisterType(QModBusRegister::RegisterType newRegisterType)
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

    inline int valueCount() const { return dataRange; }
    inline void setValueCount(int newCount) { dataRange = newCount; }

private:
    QModBusRegister::RegisterType rType;
    int sAddress;
    QVector<quint16> dataValue;
    int dataRange;
};

Q_DECLARE_TYPEINFO(QModBusDataUnit, Q_MOVABLE_TYPE);

QT_END_NAMESPACE
#endif // QMODBUSDATAUNIT_H
