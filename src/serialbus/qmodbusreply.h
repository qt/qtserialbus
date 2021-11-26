/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
