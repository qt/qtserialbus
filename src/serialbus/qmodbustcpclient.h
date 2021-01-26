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

#ifndef QMODBUSTCPCLIENT_H
#define QMODBUSTCPCLIENT_H

#include <QtSerialBus/qmodbusclient.h>

QT_BEGIN_NAMESPACE

class QModbusTcpClientPrivate;

class Q_SERIALBUS_EXPORT QModbusTcpClient : public QModbusClient
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModbusTcpClient)

public:
    explicit QModbusTcpClient(QObject *parent = nullptr);
    ~QModbusTcpClient();

protected:
    QModbusTcpClient(QModbusTcpClientPrivate &dd, QObject *parent = nullptr);

    bool open() override;
    void close() override;
};

QT_END_NAMESPACE

#endif // QMODBUSTCPCLIENT_H
