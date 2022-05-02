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

#ifndef QMODBUSREPLY_H
#define QMODBUSREPLY_H

#include <QtCore/qlist.h>
#include <QtSerialBus/qmodbusdataunit.h>
#include <QtSerialBus/qmodbusdevice.h>
#include <QtSerialBus/qmodbuspdu.h>

QT_BEGIN_NAMESPACE

class QModbusReplyPrivate;

class Q_SERIALBUS_EXPORT QModbusReply : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModbusReply)

public:
    enum ReplyType {
        Raw,
        Common,
        Broadcast
    };
    Q_ENUM(ReplyType)

    QModbusReply(ReplyType type, int serverAddress, QObject *parent = nullptr);

    ReplyType type() const;
    int serverAddress() const;

    bool isFinished() const;

    QModbusDataUnit result() const;
    QModbusResponse rawResult() const;

    QString errorString() const;
    QModbusDevice::Error error() const;

    void setResult(const QModbusDataUnit &unit);
    void setRawResult(const QModbusResponse &unit);

    void setFinished(bool isFinished);
    void setError(QModbusDevice::Error error, const QString &errorText);

    QList<QModbusDevice::IntermediateError> intermediateErrors() const;
    void addIntermediateError(QModbusDevice::IntermediateError error);

Q_SIGNALS:
    void finished();
    void errorOccurred(QModbusDevice::Error error);
    void intermediateErrorOccurred(QModbusDevice::IntermediateError error);
};
Q_DECLARE_TYPEINFO(QModbusReply::ReplyType, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

#endif // QMODBUSREPLY_H
