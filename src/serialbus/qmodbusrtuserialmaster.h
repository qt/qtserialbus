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

#ifndef QMODBUSRTUSERIALMASTER_H
#define QMODBUSRTUSERIALMASTER_H

#include <QtSerialBus/qmodbusclient.h>

QT_BEGIN_NAMESPACE

class QModbusRtuSerialMasterPrivate;

class Q_SERIALBUS_EXPORT QModbusRtuSerialMaster : public QModbusClient
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModbusRtuSerialMaster)

public:
    explicit QModbusRtuSerialMaster(QObject *parent = nullptr);
    ~QModbusRtuSerialMaster();

    int interFrameDelay() const;
    void setInterFrameDelay(int microseconds);

    int turnaroundDelay() const;
    void setTurnaroundDelay(int turnaroundDelay);

protected:
    QModbusRtuSerialMaster(QModbusRtuSerialMasterPrivate &dd, QObject *parent = nullptr);

    void close() override;
    bool open() override;
};

QT_END_NAMESPACE

#endif // QMODBUSRTUSERIALMASTER_H
