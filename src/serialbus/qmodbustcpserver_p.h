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

class QModbusTcpServerPrivate : public QModbusServerPrivate
{
    Q_DECLARE_PUBLIC(QModbusTcpServer)

public:
    QModbusTcpServerPrivate()
    {
    }

    /*
        This function is a workaround since 2nd level lambda below cannot
        call protected QModbusTcpServer::processRequest(..) function on VS2013.
    */
    QModbusResponse forwardProcessRequest(const QModbusRequest &r)
    {
        Q_Q(QModbusTcpServer);
        return q->processRequest(r);
    }

    /*
        This function is a workaround since 2nd level lambda below cannot
        call protected QModbusDevice::setError(..) function on VS2013.
    */
    void forwardError(const QString &errorText, QModbusDevice::ModbusError error)
    {
        Q_Q(QModbusTcpServer);
        q->setError(errorText, error);
    }

    void setupTcpServer()
    {
        Q_Q(QModbusTcpServer);
        m_tcpServer = new QTcpServer(q);
        QObject::connect(m_tcpServer, &QTcpServer::newConnection, [this]() {
            auto *socket = m_tcpServer->nextPendingConnection();
            if (!socket)
                return;

            qCDebug(QT_MODBUS) << "Incoming socket from" << socket->peerAddress()
                               << socket->peerName() << socket->peerPort();

            connections.append(socket);

            QByteArray *buffer = new QByteArray();

            QObject::connect(socket, &QObject::destroyed, [buffer]() {
                // cleanup buffer
                delete buffer;
            });
            QObject::connect(socket, &QTcpSocket::disconnected, [socket, this]() {
                connections.removeAll(socket);
                socket->deleteLater();
            });
            QObject::connect(socket, &QTcpSocket::readyRead, [buffer, socket, this]() {
                if (!socket)
                    return;

                buffer->append(socket->readAll());
                while (!buffer->isEmpty()) {
                    qCDebug(QT_MODBUS_LOW).noquote() << "Read buffer: 0x" + buffer->toHex();

                    if (buffer->size() < mbpaHeaderSize) {
                        qCDebug(QT_MODBUS) << "ADU too short. Waiting for more data.";
                        return;
                    }

                    quint8 unitId;
                    quint16 transactionId, bytesPdu, protocolId;
                    QDataStream input(*buffer);
                    input >> transactionId >> protocolId >> bytesPdu >> unitId;

                    qCDebug(QT_MODBUS_LOW) << "Request MBPA:" << "Transaction Id:" << transactionId
                                           << "Protocol Id:" << protocolId << "PDU bytes:" << bytesPdu
                                           << "Unit Id:" << unitId;

                    // The length field is the byte count of the following fields, including the Unit
                    // Identifier and the PDU, so we remove on byte.
                    bytesPdu--;

                    if (buffer->size() < mbpaHeaderSize + bytesPdu) {
                        qCDebug(QT_MODBUS) << "PDU too short. Waiting for more data";
                        return;
                    }

                    QModbusRequest request;
                    input >> request;

                    qCDebug(QT_MODBUS) << "Request PDU:" << request;
                    const QModbusResponse response = forwardProcessRequest(request);
                    qCDebug(QT_MODBUS) << "Response PDU:" << response;

                    buffer->remove(0, mbpaHeaderSize + bytesPdu);

                    QByteArray result;
                    QDataStream output(&result, QIODevice::WriteOnly);
                    // The length field is the byte count of the following fields, including the Unit
                    // Identifier and PDU fields, so we add one byte to the response size.
                    output << transactionId << protocolId << quint16(response.size() + 1)
                           << unitId << response;

                    if (!socket->isOpen()) {
                        qCDebug(QT_MODBUS) << "Requesting socket has closed.";
                        forwardError(QModbusTcpServer::tr("Requesting socket is closed"),
                                     QModbusDevice::WriteError);
                        return;
                    }

                    int writtenBytes = socket->write(result);
                    if (writtenBytes == -1 || writtenBytes < result.size()) {
                        qCDebug(QT_MODBUS) << "Cannot write requested response to socket.";
                        forwardError(QModbusTcpServer::tr("Could not write response to client"),
                                     QModbusDevice::WriteError);
                    }
                }
            });
        });
        QObject::connect(m_tcpServer, &QTcpServer::acceptError,
                         [this](QAbstractSocket::SocketError /*sError*/) {
            Q_Q(QModbusTcpServer);

            qCWarning(QT_MODBUS) << "Server accept error";
            q->setError(m_tcpServer->errorString(), QModbusDevice::ConnectionError);
        });
    }

    QTcpServer *m_tcpServer;
    QVector<QTcpSocket *> connections;

    static const qint8 mbpaHeaderSize = 7;
    static const qint16 maxBytesModbusADU = 260;
};

QT_END_NAMESPACE

#endif // QMODBUSTCPSERVER_P_H

