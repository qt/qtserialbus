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

#ifndef QMODBUSTCPSERVER_H
#define QMODBUSTCPSERVER_H

#include <QtSerialBus/qmodbuspdu.h>
#include <QtSerialBus/qmodbusserver.h>

QT_BEGIN_NAMESPACE

class QModbusTcpServerPrivate;
class QTcpSocket;

class Q_SERIALBUS_EXPORT QModbusTcpConnectionObserver
{
public:
    virtual ~QModbusTcpConnectionObserver();

    virtual bool acceptNewConnection(QTcpSocket *newClient) = 0;
};

class Q_SERIALBUS_EXPORT QModbusTcpServer : public QModbusServer
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModbusTcpServer)

public:
    explicit QModbusTcpServer(QObject *parent = nullptr);
    ~QModbusTcpServer();

    void installConnectionObserver(QModbusTcpConnectionObserver *observer);

Q_SIGNALS:
    void modbusClientDisconnected(QTcpSocket *modbusClient);

protected:
    QModbusTcpServer(QModbusTcpServerPrivate &dd, QObject *parent = nullptr);

    bool open() override;
    void close() override;

    QModbusResponse processRequest(const QModbusPdu &request) override;
};

QT_END_NAMESPACE

#endif // QMODBUSTCPSERVER_H
