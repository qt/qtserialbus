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

#ifndef QMODBUSTCPCLIENT_P_H
#define QMODBUSTCPCLIENT_P_H

#include "qmodbusclient_p.h"
#include "qmodbustcpclient.h"

#include <QtCore/qloggingcategory.h>
#include <QtNetwork/qhostaddress.h>
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

class QModbusTcpClientPrivate : public QModbusClientPrivate
{
    Q_DECLARE_PUBLIC(QModbusTcpClient)

public:
    QModbusTcpClientPrivate() Q_DECL_EQ_DEFAULT;

    void setupTcpSocket()
    {
        Q_Q(QModbusTcpClient);

        m_socket = new QTcpSocket(q);

        QObject::connect(m_socket, &QAbstractSocket::connected, [this]() {
            qCDebug(QT_MODBUS) << "Connected to" << m_socket->peerAddress()
                               << "on port" << m_socket->peerPort();
            Q_Q(QModbusTcpClient);
            responseBuffer.clear();
            q->setState(QModbusDevice::ConnectedState);
        });

        QObject::connect(m_socket, &QAbstractSocket::disconnected, [this]() {
           qCDebug(QT_MODBUS)  << "Connection closed.";
           Q_Q(QModbusTcpClient);
           q->setState(QModbusDevice::UnconnectedState);
        });

        using TypeId = void (QAbstractSocket::*)(QAbstractSocket::SocketError);
        QObject::connect(m_socket, static_cast<TypeId>(&QAbstractSocket::error),
                         [this](QAbstractSocket::SocketError /*error*/)
        {
            Q_Q(QModbusTcpClient);

            q->setError(QModbusClient::tr("TCP socket error (%1).").arg(m_socket->errorString()),
                        QModbusDevice::ConnectionError);

            if (m_socket->state() == QAbstractSocket::UnconnectedState)
                q->setState(QModbusDevice::UnconnectedState);
        });

        QObject::connect(m_socket, &QIODevice::readyRead, [this](){
            responseBuffer += m_socket->read(m_socket->bytesAvailable());
            qCDebug(QT_MODBUS_LOW) << "Response buffer:" << responseBuffer.toHex();

            while (!responseBuffer.isEmpty()) {
                // can we read enough for Modbus ADU header?
                if (responseBuffer.size() < mbpaHeaderSize) {
                    qCDebug(QT_MODBUS_LOW) << "Modbus ADU not complete";
                    return;
                }

                quint8 serverAddress;
                quint16 transactionId, bytesPdu, protocolId;
                QDataStream input(responseBuffer);
                input >> transactionId >> protocolId >> bytesPdu >> serverAddress;

                qCDebug(QT_MODBUS) << "tid:" << hex << transactionId << "size:" << bytesPdu
                                   << "server address:" << serverAddress;

                // The length field is the byte count of the following fields, including the Unit
                // Identifier and the PDU, so we remove on byte.
                bytesPdu--;

                int tcpAduSize = mbpaHeaderSize + bytesPdu;
                if (responseBuffer.size() < tcpAduSize) {
                    qCDebug(QT_MODBUS) << "PDU too short. Waiting for more data";
                    return;
                }

                QModbusResponse responsePdu;
                input >> responsePdu;
                qCDebug(QT_MODBUS) << "Received PDU:" << responsePdu.functionCode()
                                   << responsePdu.data().toHex();

                responseBuffer.remove(0, tcpAduSize);

                if (!m_transactionStore.contains(transactionId)) {
                    qCDebug(QT_MODBUS) << "No pending request for response with given transaction "
                                          "ID, ignoring response message.";
                    continue;
                }

                const QueueElement elem = m_transactionStore.take(transactionId);
                if (!responsePdu.isException()) {
                    QModbusDataUnit unit = elem.unit;
                    if (processResponse(responsePdu, &unit)) {
                        elem.reply->setResult(unit);
                        elem.reply->setFinished(true);
                    } else {
                        elem.reply->setError(
                                    QModbusReply::UnknownError,
                                    QModbusClient::tr("An invalid response has been received."));
                    }
                } else {
                    elem.reply->setProtocolError(responsePdu.exceptionCode(),
                                                 QModbusClient::tr("Modbus Exception Response."));
                }
            }
        });
    }

    void handleResponseTimeout() Q_DECL_OVERRIDE
    {
        // TODO: Implement!
    }

    QModbusReply *enqueueRequest(const QModbusRequest &request, int serverAddress,
                                 const QModbusDataUnit &unit) Q_DECL_OVERRIDE
    {
        Q_Q(QModbusTcpClient);

        quint16 tId = qrand() % 0x10000;
        QByteArray buffer;
        QDataStream output(&buffer, QIODevice::WriteOnly);
        output << tId << quint16(0) << quint16(request.size() + 1)
               << quint8(serverAddress) << request;


        int writtenBytes = m_socket->write(buffer);
        if (writtenBytes == -1 || writtenBytes < buffer.size()) {
            qCDebug(QT_MODBUS) << "Cannot write request to socket.";
            q->setError(QModbusTcpClient::tr("Could not write request to socket."),
                        QModbusDevice::WriteError);
            return Q_NULLPTR;
        }

        qCDebug(QT_MODBUS_LOW) << "Sent TCP ADU:" << buffer.toHex();
        qCDebug(QT_MODBUS) << "Sent TCP PDU:" << request;

        QModbusReply *reply = new QModbusReply(serverAddress, q);
        if (m_responseTimeoutDuration >= 0) {
            QTimer::singleShot(m_responseTimeoutDuration, [this, tId]() {
                if (!m_transactionStore.contains(tId)) {
                    qCDebug(QT_MODBUS) << "No pending request for time out with given transaction ID";
                    return;
                }

                QueueElement elem = m_transactionStore.take(tId);
                if (elem.reply.isNull())
                    return;

                qCDebug(QT_MODBUS) << "Timeout of request with id" << tId;
                elem.reply->setError(QModbusReply::TimeoutError,
                                     QModbusClient::tr("Request timeout"));
            });
        }

        QueueElement elem = { reply, request, unit };
        m_transactionStore.insert(tId, elem);

        return reply;
    }

    // TODO: Review once we have a transport layer in place.
    bool isOpen() const Q_DECL_OVERRIDE
    {
        if (m_socket)
            return m_socket->isOpen();
        return false;
    }

    QTcpSocket *m_socket = Q_NULLPTR;
    QByteArray responseBuffer;
    QHash<quint16, QueueElement> m_transactionStore;
    int mbpaHeaderSize = 7;
};

QT_END_NAMESPACE

#endif // QMODBUSTCPCLIENT_P_H
