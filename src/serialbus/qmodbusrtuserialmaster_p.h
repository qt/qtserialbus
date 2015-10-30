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

            qCDebug(QT_MODBUS)<< "Received response (incl ADU)" << completeAduFrame.toHex();
            if (QT_MODBUS().isDebugEnabled() && !responseBuffer.isEmpty())
                qCDebug(QT_MODBUS_LOW) << "Pending buffer:" << responseBuffer.toHex();

            // check CRC
            quint16 receivedCrc = completeAduFrame[aduSize - 2] << 8 | completeAduFrame[aduSize - 1];
            if (checkCRC(completeAduFrame, completeAduFrame.size(), receivedCrc)) {
                qCWarning(QT_MODBUS) << "Discarding request with wrong CRC, received:"
                                     << receivedCrc << "got:"
                                     << calculateCRC(completeAduFrame, completeAduFrame.size());
                return;
            }

            if (m_pendingReply.isNull()) {
                qCWarning(QT_MODBUS) << "Cannot associate response with QModbusReply. "
                                        "Ignoring response.";
                return;
            }

            // match slave address
            if (completeAduFrame.at(0) != m_pendingReply->slaveAddress()) {
                qCWarning(QT_MODBUS) << "Cannot match response with open request due to slave"
                                        " address mismatch" << hex << m_pendingReply->slaveAddress()
                                     << completeAduFrame.at(0);
                return;
            }

            const QModbusResponse response(fcode,
                                           completeAduFrame.mid(2, completeAduFrame.size() - 4));
            if (!response.isException()) {
                QModbusDataUnit unit = m_pendingReply->readRequestDetails;
                if (processResponse(response, &unit)) {
                    m_pendingReply->setResult(unit);
                } else {
                    m_pendingReply->setError(
                            QModbusPdu::UnknownError,
                            QModbusRtuSerialMaster::tr("An invalid response has been received."));
                }
            } else {
                QModbusExceptionResponse exception(response);
                m_pendingReply->setError(exception.exceptionCode(), QString());
            }

            m_pendingReply->setFinished(true);
            m_pendingReply.clear();
        });

        QObject::connect(m_serialPort, &QSerialPort::aboutToClose, [this]() {
            Q_Q(QModbusRtuSerialMaster);
            if (q->state() != QModbusDevice::ClosingState)
                q->close();
        });
    }

    QByteArray wrapInADU(const QModbusRequest &request, int slaveAddress) const
    {
        //Q_Q(QModbusRtuSerialMaster);

        QByteArray result;
        QDataStream out(&result, QIODevice::WriteOnly);
        out << quint8(slaveAddress);
        out << request;
        out << calculateCRC(result, result.size());

        return result;
    }

    QSerialPort *m_serialPort;
    QPointer<QModbusReply> m_pendingReply;
    QByteArray responseBuffer;
};

QT_END_NAMESPACE

#endif // QMODBUSSERIALMASTER_P_H

