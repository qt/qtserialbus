/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

#ifndef QMODBUSRTUSERIALCLIENT_H
#define QMODBUSRTUSERIALCLIENT_H

#include <QtSerialBus/qmodbusclient.h>

QT_BEGIN_NAMESPACE

class QModbusRtuSerialClientPrivate;

class Q_SERIALBUS_EXPORT QModbusRtuSerialClient : public QModbusClient
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModbusRtuSerialClient)

public:
    explicit QModbusRtuSerialClient(QObject *parent = nullptr);
    ~QModbusRtuSerialClient();

    int interFrameDelay() const;
    void setInterFrameDelay(int microseconds);

    int turnaroundDelay() const;
    void setTurnaroundDelay(int turnaroundDelay);

protected:
    QModbusRtuSerialClient(QModbusRtuSerialClientPrivate &dd, QObject *parent = nullptr);

    void close() override;
    bool open() override;
};

#if QT_DEPRECATED_SINCE(6, 2)
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wattributes")
using QModbusRtuSerialMaster
    Q_DECL_DEPRECATED_X("Please port your application to QModbusRtuSerialClient.") = QModbusRtuSerialClient;
#endif
QT_WARNING_POP
QT_END_NAMESPACE

#endif // QMODBUSRTUSERIALCLIENT_H
