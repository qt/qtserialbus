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

#ifndef QMODBUSCLIENT_H
#define QMODBUSCLIENT_H

#include <QtSerialBus/qserialbusglobal.h>
#include <QtSerialBus/qmodbusdevice.h>
#include <QtSerialBus/qmodbusdataunit.h>
#include <QtSerialBus/qmodbuspdu.h>
#include <QtSerialBus/qmodbusreply.h>
#include <QtSerialBus/qmodbusreplyex.h>

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QModbusClientPrivate;

class Q_SERIALBUS_EXPORT QModbusClient : public QModbusDevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModbusClient)
public:

    explicit QModbusClient(QObject *parent = 0);
    virtual ~QModbusClient();

    virtual QModbusReplyEx *sendRequest(const QModbusDataUnit &request, int slaveId = 1) = 0;

    //TODO to be removed once new setup ready
    virtual QModbusReply *write(const QModbusDataUnit &request, int slaveId = 1) = 0;
    virtual QModbusReply *read(const QModbusDataUnit &request, int slaveId = 1) = 0;

Q_SIGNALS:
    void requestFinished(QModbusReplyEx *result);

protected:
    QModbusClient(QModbusClientPrivate &dd, QObject *parent = Q_NULLPTR);

    virtual bool processResponse(const QModbusResponse &response, QModbusDataUnit *data);
    virtual bool processCustomResponse(const QModbusResponse &response, QModbusDataUnit *data);

};

QT_END_NAMESPACE

#endif // QMODBUSCLIENT_H
