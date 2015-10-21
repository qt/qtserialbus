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

#ifndef QMODBUSTCPSERVER_P_H
#define QMODBUSTCPSERVER_P_H

#include "qmodbusserver_p.h"
#include "qmodbustcpserver.h"

#include <QtCore/qdatastream.h>
#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qobject.h>
#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_MODBUS)
Q_DECLARE_LOGGING_CATEGORY(QT_MODBUS_LOW)

class TcpConnection : public QTcpSocket
{
    Q_OBJECT
    Q_DISABLE_COPY(TcpConnection)

public:
    enum ReadState {
        ReadMbpa,
        ReadPdu
    };

    TcpConnection(QModbusTcpServer *handler, QObject *parent = Q_NULLPTR)
        : QTcpSocket(parent)
        , requestHandler(handler)
    {
        connect(this, &QTcpSocket::readyRead, this, &TcpConnection::processReadyRead);
    }

private Q_SLOTS:
    void processReadyRead()
    {
        buffer += readAll();

        if (buffer.size() > maxBytesModbusADU) {
            abort();
            return;
        }
        qCDebug(QT_MODBUS_LOW).noquote() << "Read buffer: 0x" + buffer.toHex();

        if (readState == ReadState::ReadMbpa) {
            if (buffer.size() < mbpaHeaderSize)
                return;

            QDataStream input(buffer);
            input >> transactionId >> protocolId >> bytesPdu >> unitId;

            qCDebug(QT_MODBUS) << "Request MBPA:" << "Transaction Id:" << transactionId
                << "Protocol Id:" << protocolId << "Number of following bytes:" << bytesPdu
                << "Unit Id:" << unitId;

            // The length field is the byte count of the following fields, including the Unit
            // Identifier and the PDU, so we remove on byte.
            bytesPdu--;
            readState = ReadPdu;
        }

        if (readState == ReadState::ReadPdu) {
            if (buffer.size() < mbpaHeaderSize + bytesPdu)
                return;

            QDataStream input(buffer.mid(mbpaHeaderSize));
            QModbusRequest request;
            input >> request;

            qCDebug(QT_MODBUS_LOW) << "Request PDU:" << request;
            qCDebug(QT_MODBUS).noquote() << "Request PDU, Function code: 0x" + QByteArray(1,
                request.functionCode()).toHex() << "Data: 0x" + request.data().toHex();

            const QModbusResponse response = requestHandler->processRequest(request);

            qCDebug(QT_MODBUS_LOW) << "Response PDU:" << response;
            qCDebug(QT_MODBUS).noquote() << "Response PDU, Function code: 0x" + QByteArray(1,
                request.functionCode()).toHex() << "Data: 0x" + response.data().toHex();

            QDataStream output(this);
            // The length field is the byte count of the following fields, including the Unit
            // Identifier and PDU fields, so we add one byte to the response size.
            output << transactionId << protocolId << quint16(response.size() + 1)
                    << unitId << response;

            qCDebug(QT_MODBUS) << "Response MBPA:" << "Transaction Id:" << transactionId <<
                "Protocol Id:" << protocolId << "Number of following bytes:" << quint16(response
                .size() + 1) << "Unit Id:" << unitId;

            buffer.clear();
            readState = ReadMbpa;
        }
    }

private:
    quint8 unitId;
    quint16 transactionId;
    quint16 bytesPdu;
    quint16 protocolId;

    QByteArray buffer;
    ReadState readState = ReadMbpa;
    static const qint8 mbpaHeaderSize = 7;
    static const qint16 maxBytesModbusADU = 260;
    QModbusTcpServer *requestHandler = Q_NULLPTR;
};

class TcpServer : public QTcpServer
{
    Q_OBJECT
    Q_DISABLE_COPY(TcpServer)

public:
    TcpServer() Q_DECL_EQ_DEFAULT;
    ~TcpServer() { shutdown = true; }

    void disconnect()
    {
        foreach (TcpConnection *connection, connections)
            connection->disconnectFromHost();
        connections.clear();
    }

    void incomingConnection(qintptr socketDescriptor) Q_DECL_OVERRIDE
    {
        if (!shutdown) {
            TcpConnection *connection = new TcpConnection(requestHandler, this);
            connections.append(connection);
            connection->setSocketDescriptor(socketDescriptor);

            connect(connection, &TcpConnection::disconnected, [this]() {
                if (TcpConnection *connection = qobject_cast<TcpConnection *> (sender()))
                    connection->deleteLater();
            });
        }
    }

    bool shutdown = false;
    QList<TcpConnection*> connections;
    QModbusTcpServer *requestHandler = Q_NULLPTR;
};

class QModbusTcpServerPrivate : public QModbusServerPrivate
{
    Q_DECLARE_PUBLIC(QModbusTcpServer)

public:
    QModbusTcpServerPrivate()
    {
    }

    TcpServer m_server;
};

QT_END_NAMESPACE

#endif // QMODBUSTCPSERVER_P_H

