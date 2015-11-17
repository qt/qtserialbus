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
        m_serialPort->setBaudRate(QSerialPort::Baud9600);
        m_serialPort->setParity(QSerialPort::NoParity);
        m_serialPort->setDataBits(QSerialPort::Data8);
        m_serialPort->setStopBits(QSerialPort::OneStop);

        QObject::connect(m_serialPort, &QSerialPort::readyRead, [this]() {
            responseBuffer += m_serialPort->read(m_serialPort->bytesAvailable());
            qCDebug(QT_MODBUS_LOW) << "Response buffer:" << responseBuffer.toHex();

            if (responseBuffer.size() < 2) {
                qCDebug(QT_MODBUS) << "Modbus ADU not complete";
                return;
            }

            const QModbusSerialAdu tmpAdu(QModbusSerialAdu::Rtu, responseBuffer);
            const QModbusResponse tmpPdu = tmpAdu.pdu();
            int pduSizeWithoutFcode = QModbusResponse::calculateDataSize(tmpPdu.functionCode(),
                                                                         tmpPdu.data());
            if (pduSizeWithoutFcode < 0) {
                // wait for more data
                qCDebug(QT_MODBUS) << "Cannot calculate PDU size for function code:"
                                   << tmpPdu.functionCode() << " , delaying pending frame";
                return;
            }

            // slave address byte + function code byte + PDU size + 2 bytes CRC
            const int aduSize = 2 + pduSizeWithoutFcode + 2;
            if (tmpAdu.rawSize() < aduSize) {
                qCDebug(QT_MODBUS) << "Incomplete ADU received, ignoring";
                return;
            }

            const QModbusSerialAdu adu(QModbusSerialAdu::Rtu, responseBuffer.left(aduSize));
            responseBuffer.remove(0, aduSize);

            stopResponseTimer();

            qCDebug(QT_MODBUS)<< "Received ADU:" << adu.rawData().toHex();
            if (QT_MODBUS().isDebugEnabled() && !responseBuffer.isEmpty())
                qCDebug(QT_MODBUS_LOW) << "Pending buffer:" << responseBuffer.toHex();

            // check CRC
            if (!adu.matchingChecksum()) {
                qCWarning(QT_MODBUS) << "Discarding response with wrong CRC, received:"
                                     << adu.checksum<quint16>() << ", calculated CRC:"
                                     << QModbusSerialAdu::calculateCRC(adu.data(), adu.size());
                return;
            }

            const QModbusResponse response = adu.pdu();
            if (!canMatchRequestAndResponse(response, adu.slaveAddress())) {
                qCWarning(QT_MODBUS) << "Cannot match response with open request, ignoring";
                return;
            }

            QueueElement headOfQueue = m_queue.dequeue();
            if (!response.isException()) {
                QModbusDataUnit unit = headOfQueue.unit;
                if (processResponse(response, &unit)) {
                    headOfQueue.reply->setResult(unit);
                    headOfQueue.reply->setFinished(true);
                } else {
                    headOfQueue.reply->setError(
                            QModbusReply::UnknownError,
                            QModbusClient::tr("An invalid response has been received."));
                }
            } else {
               headOfQueue.reply->setProtocolError(response.exceptionCode(),
                                                   QModbusClient::tr("Modbus Exception Response."));
            }

            sendNextRequest(); // go to next request
        });

        QObject::connect(m_serialPort, &QSerialPort::aboutToClose, [this]() {
            Q_Q(QModbusRtuSerialMaster);
            if (q->state() != QModbusDevice::ClosingState)
                q->close();
        });
    }

    bool sendNextAdu(const QModbusRequest &request, int slaveAddress)
    {
        Q_Q(QModbusRtuSerialMaster);

        const QByteArray adu = QModbusSerialAdu::create(QModbusSerialAdu::Rtu, slaveAddress,
                                                        request);
        int writtenBytes = m_serialPort->write(adu);
        if (writtenBytes == -1 || writtenBytes < adu.size()) {
            qCDebug(QT_MODBUS) << "Cannot write request to serial port. Failed ADU" << adu;
            q->setError(QModbusClient::tr("Could not write request to serial bus."),
                        QModbusDevice::WriteError);
            return false;
        }
        qCDebug(QT_MODBUS)<< "Sent ADU:" << adu.toHex();

        return true;

    }

    void sendNextRequest()
    {
        if (m_queue.isEmpty())
            return;

        if (m_queue.head().reply.isNull()) { // reply deleted, skip it
            m_queue.dequeue();
            sendNextRequest();
            return;
        }

        bool success = sendNextAdu(m_queue.head().requestPdu,
                                   m_queue.head().reply->slaveAddress());
        if (!success) {
            QueueElement elem = m_queue.dequeue();

            elem.reply->setError(QModbusReply::WriteError,
                                 QModbusClient::tr("Could not write message to serial bus."));

            sendNextRequest();
            return;
        }

        if (!m_queue.head().reply->slaveAddress()) {
            // broadcasts return immediately but we delay a bit to avoid spaming of bus
            QueueElement elem = m_queue.dequeue();
            elem.reply->setFinished(true);

            QTimer::singleShot(0, [this]() { sendNextRequest(); });
            return;
        }

        // regular send -> keep in queue
        startResponseTimer();
    }

    QModbusReply *enqueueRequest(const QModbusRequest &request, int slaveAddress,
                                       const QModbusDataUnit unit)
    {
        Q_Q(QModbusRtuSerialMaster);

        QModbusReply *reply = new QModbusReply(slaveAddress, q);
        QueueElement elem = { reply, request, unit };
        m_queue.enqueue(elem);
        sendNextRequest();

        return reply;
    }

    bool canMatchRequestAndResponse(const QModbusResponse &response, int sendingSlave) const
    {
        if (m_queue.isEmpty()) // nothing pending
            return false;

        const QueueElement &head = m_queue.head(); // reply deleted
        if (head.reply.isNull())
            return false;

        if (head.reply->slaveAddress() != sendingSlave) // slave mismatch
            return false;

        // request for different fcode
        if (head.requestPdu.functionCode() != response.functionCode())
            return false;

        return true;
    }

    void handleResponseTimeout()
    {
        qCDebug(QT_MODBUS) << "Timeout of last request";

        if (m_queue.isEmpty())
            return;

        QueueElement elem = m_queue.dequeue();
        if (elem.reply.isNull()) {
            // reply deleted while waiting for response which timed out
            // nothing really to do here
            return;
        }

        elem.reply->setError(QModbusReply::TimeoutError,
                             QModbusClient::tr("Request timeout."));
    }

    QSerialPort *m_serialPort;
    QByteArray responseBuffer;

    struct QueueElement {
        QPointer<QModbusReply> reply;
        QModbusRequest requestPdu;
        QModbusDataUnit unit;
    };

    QQueue<QueueElement> m_queue;
};

QT_END_NAMESPACE

#endif // QMODBUSSERIALMASTER_P_H
