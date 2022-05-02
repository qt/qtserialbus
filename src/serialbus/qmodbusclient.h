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

#ifndef QMODBUSCLIENT_H
#define QMODBUSCLIENT_H

#include <QtCore/qobject.h>
#include <QtSerialBus/qmodbusdataunit.h>
#include <QtSerialBus/qmodbusdevice.h>
#include <QtSerialBus/qmodbuspdu.h>
#include <QtSerialBus/qmodbusreply.h>

QT_BEGIN_NAMESPACE

class QModbusClientPrivate;

class Q_SERIALBUS_EXPORT QModbusClient : public QModbusDevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModbusClient)

public:
    explicit QModbusClient(QObject *parent = nullptr);
    ~QModbusClient();

    QModbusReply *sendReadRequest(const QModbusDataUnit &read, int serverAddress);
    QModbusReply *sendWriteRequest(const QModbusDataUnit &write, int serverAddress);
    QModbusReply *sendReadWriteRequest(const QModbusDataUnit &read, const QModbusDataUnit &write,
                                       int serverAddress);
    QModbusReply *sendRawRequest(const QModbusRequest &request, int serverAddress);

    int timeout() const;
    void setTimeout(int newTimeout);

    int numberOfRetries() const;
    void setNumberOfRetries(int number);

Q_SIGNALS:
    void timeoutChanged(int newTimeout);

protected:
    QModbusClient(QModbusClientPrivate &dd, QObject *parent = nullptr);

    virtual bool processResponse(const QModbusResponse &response, QModbusDataUnit *data);
    virtual bool processPrivateResponse(const QModbusResponse &response, QModbusDataUnit *data);
};

QT_END_NAMESPACE

#endif // QMODBUSCLIENT_H
