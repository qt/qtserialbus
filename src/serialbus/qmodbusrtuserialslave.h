/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef QMODBUSRTUSERIALSLAVE_H
#define QMODBUSRTUSERIALSLAVE_H

#include <QtSerialBus/qmodbusserver.h>

QT_BEGIN_NAMESPACE

class QModbusRtuSerialSlavePrivate;

class Q_SERIALBUS_EXPORT QModbusRtuSerialSlave : public QModbusServer
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModbusRtuSerialSlave)

public:
    explicit QModbusRtuSerialSlave(QObject *parent = nullptr);
    ~QModbusRtuSerialSlave();

    bool processesBroadcast() const override;

protected:
    QModbusRtuSerialSlave(QModbusRtuSerialSlavePrivate &dd, QObject *parent = nullptr);

    bool open() override;
    void close() override;

    QModbusResponse processRequest(const QModbusPdu &request) override;
};

QT_END_NAMESPACE

#endif // QMODBUSRTUSERIALSLAVE_H
