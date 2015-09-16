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

#ifndef LIBMODBUSBACKEND_H
#define LIBMODBUSBACKEND_H

#include <QtSerialBus/qmodbusslave.h>

#include <QtCore/qstring.h>
#include <QtCore/qthread.h>
#include <QtCore/qpointer.h>
#include <QtCore/qmap.h>

#include <modbus.h>

QT_BEGIN_NAMESPACE

class ListenThread : public QObject
{
    Q_OBJECT

    enum FunctionId {
        ReadCoils = 1,
        ReadDiscreteInputs = 2,
        ReadHoldingRegisters = 3,
        ReadInputRegisters = 4,
        WriteSingleCoil = 5,
        WriteSingleRegister = 6,
        WriteMultipleCoils = 15,
        WriteMultipleRegisters = 16,
        ReadWriteRegisters = 23
    };

public slots:
    void doWork();

public:
    modbus_t *context;
    modbus_mapping_t *mapping;

Q_SIGNALS:
    void error(int errorNumber);
    void slaveRead();
    void slaveWritten(QModBusRegister::RegisterType table, int address, int size);
};

class LibModBusSlave : public QModBusSlave
{
    Q_OBJECT
public:
    LibModBusSlave();
    ~LibModBusSlave();

    bool setMap(const QModBusRegister &newRegister) Q_DECL_OVERRIDE;

    int slaveId() const Q_DECL_OVERRIDE;
    void setSlaveId(int id) Q_DECL_OVERRIDE;

    bool data(QModBusRegister::RegisterType table, quint16 address, quint16 *data) Q_DECL_OVERRIDE;
    bool setData(QModBusRegister::RegisterType table, quint16 address, quint16 data) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void operate();

private Q_SLOTS:
    void handleError(int errorNumber);

private:
    bool open() Q_DECL_OVERRIDE;
    void close() Q_DECL_OVERRIDE;
    static QString portNameToSystemLocation(const QString &source);

private:
    QPointer<ListenThread> listener;
    QThread thread;
    modbus_t *context;
    modbus_mapping_t *mapping;
    bool connected;
    int slave;
    QMap<QModBusRegister::RegisterType, int> mappingTable;
};

QT_END_NAMESPACE

#endif // LIBMODBUSBACKEND_H
