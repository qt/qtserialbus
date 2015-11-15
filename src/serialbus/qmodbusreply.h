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

#ifndef QMODBUSREPLY_H
#define QMODBUSREPLY_H

#include <QtSerialBus/qserialbusglobal.h>
#include <QtSerialBus/qmodbusdataunit.h>
#include <QtSerialBus/qmodbuspdu.h>

QT_BEGIN_NAMESPACE

class QModbusReplyPrivate;

class Q_SERIALBUS_EXPORT QModbusReply : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModbusReply)

public:
    enum ReplyError {
        NoError =               0x00,
        ProtocolError =         0x01,
        TimeoutError =          0x02,
        ReplyAbortedError =     0x03,
        UnknownError =          0x04,
        WriteError =            0x05
    };
    Q_ENUMS(ReplyError)

    explicit QModbusReply(int slaveAddress, QObject *parent = Q_NULLPTR);

    bool isFinished() const;
    QModbusDataUnit result() const;
    int slaveAddress() const;

    ReplyError error() const;
    QModbusPdu::ExceptionCode protocolError() const;
    QString errorText() const;

    void setResult(const QModbusDataUnit &unit);
    void setFinished(bool isFinished);
    void setProtocolError(QModbusPdu::ExceptionCode error, const QString &errorText);
    void setError(QModbusReply::ReplyError error, const QString &errorText);


Q_SIGNALS:
    void finished();
    void errorOccurred(QModbusReply::ReplyError error);
};

Q_DECLARE_TYPEINFO(QModbusReply::ReplyError, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QModbusReply::ReplyError)

#endif // QMODBUSREPLY_H
