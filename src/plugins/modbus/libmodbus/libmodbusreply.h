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

#ifndef REPLY_H
#define REPLY_H

#include <QtCore/qobject.h>
#include <QtCore/qthread.h>
#include <QtCore/qpointer.h>
#include <QtCore/qvector.h>

#include <QtSerialBus/qmodbusreply.h>
#include <QtSerialBus/qmodbusmaster.h>

#include <modbus.h>

QT_BEGIN_NAMESPACE

class RequestThread : public QObject
{
    Q_OBJECT
public:
    QModBusDevice::ModBusTable table;
    int startAddress;
    QVector<quint16> values;
    quint16 size;
    int slaveId;
    modbus_t *context;

public Q_SLOTS:
    void read();
    void write();

Q_SIGNALS:
    void error(int errorNumber);
    void readReady(const QVector<quint16> &payload);
    void writeReady();

private:
    void readBits();
    void readBytes();
    void writeBits();
    void writeBytes();
};

class Reply : public QModBusReply
{
    Q_OBJECT
public:
    explicit Reply(QObject *parent = 0);
    ~Reply();

    void read(const QModBusDataUnit &requests, int slaveId, modbus_t *context);
    void write(const QModBusDataUnit &requests, int slaveId, modbus_t *context);

protected:
    void setFinished() Q_DECL_OVERRIDE;
    void setError(QModBusReply::RequestError errorCode,
                  const QString &errorString) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void startRead();
    void startWrite();

private:
    void setResults(const QVector<quint16> &payload);
    void handleError(int errorNumber);

    QModBusDevice::ModBusTable table;
    int startAddress;
    QPointer<RequestThread> request;
    QThread thread;
    QVector<quint16> values;
};

QT_END_NAMESPACE

#endif // REPLY_H
