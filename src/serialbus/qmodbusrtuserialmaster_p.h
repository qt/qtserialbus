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

#ifndef QMODBUSSERIALMASTER_P_H
#define QMODBUSSERIALMASTER_P_H

#include <QtSerialBus/qmodbusrtuserialmaster.h>
#include <QtSerialBus/private/qmodbusadu_p.h>
#include <QtSerialBus/private/qmodbusclient_p.h>

#include <QtCore/qloggingcategory.h>
#include <QtCore/qpointer.h>
#include <QtCore/qqueue.h>
#include <QtCore/qtimer.h>
#include <QtSerialPort/qserialport.h>

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

class QModbusRtuSerialMasterPrivate : public QModbusClientPrivate
{
    Q_DECLARE_PUBLIC(QModbusRtuSerialMaster)

public:
    QModbusRtuSerialMasterPrivate() Q_DECL_EQ_DEFAULT;

    void setupSerialPort()
    {
        Q_Q(QModbusRtuSerialMaster);

        m_serialPort = new QSerialPort(q);

        m_responseTimer.setSingleShot(true);
        m_responseTimer.setInterval(m_responseTimeoutDuration);
        q->connect(q, &QModbusClient::timeoutChanged, &m_responseTimer, &QTimer::setInterval);
        QObject::connect(&m_responseTimer, &QTimer::timeout, q, [this]() {
            if (m_queue.isEmpty())
                return;
            m_processesTimeout = true;
            QueueElement elem = m_queue.head();
            if (elem.reply.isNull() || elem.numberOfRetries <= 0) {
                elem = m_queue.dequeue();
                if (elem.numberOfRetries <= 0) {
                    elem.reply->setError(QModbusReply::TimeoutError,
                        QModbusClient::tr("Request timeout."));
                    qCDebug(QT_MODBUS) << "(RTU client) Timeout of request" << elem.requestPdu;
                }
            } else {
                m_queue[0].numberOfRetries--;
                qCDebug(QT_MODBUS) << "(RTU client) Resend request:" << elem.requestPdu;
            }
            m_processesTimeout = false;
            // go to next request or send request again
            QTimer::singleShot(0, [this]() { sendNextRequest(); });
        });

        QObject::connect(m_serialPort, &QSerialPort::readyRead, [this]() {
            responseBuffer += m_serialPort->read(m_serialPort->bytesAvailable());
            qCDebug(QT_MODBUS_LOW) << "(RTU client) Response buffer:" << responseBuffer.toHex();

            if (responseBuffer.size() < 2) {
                qCDebug(QT_MODBUS) << "(RTU client) Modbus ADU not complete";
                return;
            }

            const QModbusSerialAdu tmpAdu(QModbusSerialAdu::Rtu, responseBuffer);
            const QModbusResponse tmpPdu = tmpAdu.pdu();
            int pduSizeWithoutFcode = QModbusResponse::calculateDataSize(tmpPdu, tmpPdu.data());
            if (pduSizeWithoutFcode < 0) {
                // wait for more data
                qCDebug(QT_MODBUS) << "(RTU client) Cannot calculate PDU size for function code:"
                                   << tmpPdu.functionCode() << " , delaying pending frame";
                return;
            }

            // server address byte + function code byte + PDU size + 2 bytes CRC
            const int aduSize = 2 + pduSizeWithoutFcode + 2;
            if (tmpAdu.rawSize() < aduSize) {
                qCDebug(QT_MODBUS) << "(RTU client) Incomplete ADU received, ignoring";
                return;
            }

            m_responseTimer.stop();

            const QModbusSerialAdu adu(QModbusSerialAdu::Rtu, responseBuffer.left(aduSize));
            responseBuffer.remove(0, aduSize);

            qCDebug(QT_MODBUS)<< "(RTU client) Received ADU:" << adu.rawData().toHex();
            if (QT_MODBUS().isDebugEnabled() && !responseBuffer.isEmpty())
                qCDebug(QT_MODBUS_LOW) << "(RTU client) Pending buffer:" << responseBuffer.toHex();

            // check CRC
            if (!adu.matchingChecksum()) {
                qCWarning(QT_MODBUS) << "(RTU client) Discarding response with wrong CRC, received:"
                                     << adu.checksum<quint16>() << ", calculated CRC:"
                                     << QModbusSerialAdu::calculateCRC(adu.data(), adu.size());
                return;
            }

            const QModbusResponse response = adu.pdu();
            if (!canMatchRequestAndResponse(response, adu.serverAddress())) {
                qCWarning(QT_MODBUS) << "(RTU client) Cannot match response with open request, "
                    "ignoring";
                return;
            }

            processQueueElement(response, m_queue.dequeue());
            QTimer::singleShot(0, [this]() { sendNextRequest(); });
        });

        QObject::connect(m_serialPort, &QSerialPort::aboutToClose, [this]() {
            Q_Q(QModbusRtuSerialMaster);
            if (q->state() != QModbusDevice::ClosingState)
                q->close();
        });
    }

    void updateSerialPortConnectionInfo() {
        if (m_serialPort) {
            m_serialPort->setPortName(m_comPort);
            m_serialPort->setParity(m_parity);
            m_serialPort->setBaudRate(m_baudRate);
            m_serialPort->setDataBits(m_dataBits);
            m_serialPort->setStopBits(m_stopBits);
        }
    }

    bool sendNextAdu(const QModbusRequest &request, int serverAddress)
    {
        Q_Q(QModbusRtuSerialMaster);

        const QByteArray adu = QModbusSerialAdu::create(QModbusSerialAdu::Rtu, serverAddress,
                                                        request);
        m_serialPort->clear(QSerialPort::Output);
        int writtenBytes = m_serialPort->write(adu);
        if (writtenBytes == -1 || writtenBytes < adu.size()) {
            qCDebug(QT_MODBUS_LOW) << "(RTU client) Cannot send Serial ADU:" << adu.toHex();
            q->setError(QModbusClient::tr("Could not write request to serial bus."),
                        QModbusDevice::WriteError);
            return false;
        }
        qCDebug(QT_MODBUS) << "(RTU client) Sent Serial PDU:" << request;
        qCDebug(QT_MODBUS_LOW)<< "(RTU client) Sent Serial ADU:" << adu.toHex();

        return true;
    }

    void sendNextRequest()
    {
        if (m_queue.isEmpty())
            m_responseTimer.stop();

        if (m_responseTimer.isActive() || m_processesTimeout)
            return;

        QueueElement elem = m_queue.head();
        if (elem.reply.isNull()) { // reply deleted, skip it
            m_queue.dequeue();
            QTimer::singleShot(0, [this]() { sendNextRequest(); });
            return;
        }

        bool success = sendNextAdu(elem.requestPdu, elem.reply->serverAddress());
        if (!success) {
            elem = m_queue.dequeue();
            elem.reply->setError(QModbusReply::WriteError,
                                 QModbusClient::tr("Could not write message to serial bus."));
            QTimer::singleShot(0, [this]() { sendNextRequest(); });
            return;
        }

        if (elem.reply->serverAddress() == 0) {
            // broadcasts return immediately but we delay a bit to avoid spaming of bus
            elem = m_queue.dequeue();
            elem.reply->setFinished(true);

            QTimer::singleShot(0, [this]() { sendNextRequest(); });
            return;
        }

        // regular send -> keep in queue
        m_responseTimer.start();
    }

    QModbusReply *enqueueRequest(const QModbusRequest &request, int slaveAddress,
                                 const QModbusDataUnit &unit,
                                 QModbusReply::ReplyType type) Q_DECL_OVERRIDE
    {
        Q_Q(QModbusRtuSerialMaster);

        QModbusReply *reply = new QModbusReply(type, slaveAddress, q);
        m_queue.enqueue(QueueElement{ reply, request, unit, m_numberOfRetries });

        q->connect(reply, &QObject::destroyed, q, [this](QObject *obj) {
            foreach (const QueueElement &element, m_queue) {
                if (element.reply != obj)
                    continue;
                m_queue.removeAll(element);
                QTimer::singleShot(0, [this]() { sendNextRequest(); });
            }
        });

        if (!m_responseTimer.isActive())
            QTimer::singleShot(0, [this]() { sendNextRequest(); });
        return reply;
    }

    bool canMatchRequestAndResponse(const QModbusResponse &response, int sendingServer) const
    {
        if (m_queue.isEmpty()) // nothing pending
            return false;

        const QueueElement &head = m_queue.head(); // reply deleted
        if (head.reply.isNull())
            return false;

        if (head.reply->serverAddress() != sendingServer) // server mismatch
            return false;

        // request for different fcode
        if (head.requestPdu.functionCode() != response.functionCode())
            return false;

        return true;
    }

    // TODO: Review once we have a transport layer in place.
    bool isOpen() const Q_DECL_OVERRIDE
    {
        if (m_serialPort)
            return m_serialPort->isOpen();
        return false;
    }

    QSerialPort *m_serialPort;
    QByteArray responseBuffer;
    QQueue<QueueElement> m_queue;
    QTimer m_responseTimer;
    bool m_processesTimeout = false;
};

QT_END_NAMESPACE

#endif // QMODBUSSERIALMASTER_P_H
