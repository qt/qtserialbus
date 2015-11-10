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

#include "qmodbusrtuserialmaster.h"
#include "qmodbusclient_p.h"

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
    QModbusRtuSerialMasterPrivate()
    {
    }

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

            if (responseBuffer.size() < 2) {
                qCDebug(QT_MODBUS) << "Modbus ADU not complete";
                return;
            }

            int aduSize = 2; //slave address & function code

            QModbusPdu::FunctionCode fcode = QModbusPdu::FunctionCode(responseBuffer.at(1));
            int pduSizeWithoutFcode = QModbusResponse::calculateDataSize(fcode, responseBuffer.mid(2));
            if (pduSizeWithoutFcode < 0) {
                // wait for more data
                qCDebug(QT_MODBUS) << "Cannot calculate PDU size for fcode, delaying pending frame"
                                   << fcode;
                return;
            }
            aduSize += pduSizeWithoutFcode;

            aduSize += 2; //CRC
            if (responseBuffer.size() < aduSize) {
                qCDebug(QT_MODBUS) << "ADU too short, ignoring pending frame";
                return;
            }

            const QByteArray completeAduFrame = responseBuffer.left(aduSize);
            responseBuffer.remove(0, aduSize);

            stopResponseTimer();

            qCDebug(QT_MODBUS)<< "Received response (incl ADU)" << completeAduFrame.toHex();
            if (QT_MODBUS().isDebugEnabled() && !responseBuffer.isEmpty())
                qCDebug(QT_MODBUS_LOW) << "Pending buffer:" << responseBuffer.toHex();

            // check CRC
            quint16 receivedCrc = quint8(completeAduFrame[aduSize - 2]) << 8
                                  | quint8(completeAduFrame[aduSize - 1]);
            if (!matchingCRC(completeAduFrame, completeAduFrame.size() - 2, receivedCrc)) {
                qCWarning(QT_MODBUS) << "Discarding request with wrong CRC, received:"
                                     << receivedCrc << "got:"
                                     << calculateCRC(completeAduFrame, completeAduFrame.size());
                return;
            }


            const QModbusResponse response(fcode,
                                           completeAduFrame.mid(2, completeAduFrame.size() - 4));

            if (!canMatchRequestAndResponse(response, quint8(completeAduFrame.at(0)))) {
                qCWarning(QT_MODBUS) << "Cannot match response with open request. "
                                        "Ignoring response.";
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
                            QModbusRtuSerialMaster::tr("An invalid response has been received."));
                }
            } else {
               headOfQueue.reply->setProtocolError(response.exceptionCode(), QString());
            }

            sendNextRequest(); // go to next request
        });

        QObject::connect(m_serialPort, &QSerialPort::aboutToClose, [this]() {
            Q_Q(QModbusRtuSerialMaster);
            if (q->state() != QModbusDevice::ClosingState)
                q->close();
        });
    }

    QByteArray wrapInADU(const QModbusRequest &request, int slaveAddress) const
    {
        QByteArray result;
        QDataStream out(&result, QIODevice::WriteOnly);
        out << quint8(slaveAddress);
        out << request;
        out << calculateCRC(result, result.size());

        return result;
    }

    bool sendNextAdu(const QModbusRequest &request, int slaveAddress)
    {
        Q_Q(QModbusRtuSerialMaster);

        const QByteArray adu = wrapInADU(request, slaveAddress);
        int writtenBytes = m_serialPort->write(adu);
        if (writtenBytes == -1 || writtenBytes < adu.size()) {
            qCDebug(QT_MODBUS) << "Cannot write request to serial port. Failed ADU"
                               << adu;
            q->setError(QModbusRtuSerialMaster::tr("Could not write request to serial bus"),
                        QModbusDevice::WriteError);
            return false;
        } else {
            qCDebug(QT_MODBUS)<< "Sent request (incl ADU)" << adu.toHex();
        }

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
                                 QModbusRtuSerialMaster::tr("Could not write message to serial bus."));

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
                             QModbusRtuSerialMaster::tr("Request timeout."));
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

