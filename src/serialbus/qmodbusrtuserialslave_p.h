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

#ifndef QMODBUSRTUSERIALSLAVE_P_H
#define QMODBUSRTUSERIALSLAVE_P_H

#include "qmodbusrtuserialslave.h"
#include "qmodbusserver_p.h"

#include <QtCore/qdebug.h>
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

class QModbusRtuSerialSlavePrivate : public QModbusServerPrivate
{
    Q_DECLARE_PUBLIC(QModbusRtuSerialSlave)

public:
    QModbusRtuSerialSlavePrivate()
    {
    }

    ~QModbusRtuSerialSlavePrivate()
    {
    }

    void setupSerialPort()
    {
        Q_Q(QModbusRtuSerialSlave);

        m_serialPort = new QSerialPort(q);
        m_serialPort->setBaudRate(QSerialPort::Baud9600);
        m_serialPort->setParity(QSerialPort::NoParity);
        m_serialPort->setDataBits(QSerialPort::Data8);
        m_serialPort->setStopBits(QSerialPort::OneStop);

        QObject::connect(m_serialPort, &QSerialPort::readyRead, [this]() {
            Q_Q(QModbusRtuSerialSlave);

            const int size = m_serialPort->size();
            const QByteArray buffer = m_serialPort->read(size);

            qCDebug(QT_MODBUS_LOW) << "Received buffer (incl ADU):" << buffer.toHex();

            // Index                         -> description
            // SlaveId                       -> 1 byte
            // FunctionCode                  -> 1 byte
            // FunctionCode specific content -> 0-252 bytes
            // CRC                           -> 2 bytes

            // We expect at least the slave address, function code and CRC.
            if (buffer.size() < 4) { // TODO: LRC should be 3 bytes.
                qCWarning(QT_MODBUS) << "Invalid Modbus PDU received";

                // The quantity of CRC errors encountered by the remote device since its last
                // restart, clear counters operation, or power–up. In case of a message
                // length < 4 bytes, the receiving device is not able to calculate the CRC.
                incrementCounter(QModbusServerPrivate::Counter::BusCommunicationError);
                return;
            }

            const QModbusPdu::FunctionCode code = QModbusRequest::FunctionCode(buffer.at(1));
            const int pduSizeWithoutFcode = QModbusRequest::calculateDataSize(code, buffer.mid(2));

            // slave address byte + function code byte + PDU size + 2 bytes CRC
            if ((pduSizeWithoutFcode < 0) || (2 + pduSizeWithoutFcode + 2) != buffer.size()) {
                qCWarning(QT_MODBUS) << "ADU does not match expected size, ignoring";
                // The quantity of messages addressed to the remote device that it could not
                // handle due to a character overrun condition, since its last restart, clear
                // counters operation, or power–up. A character overrun is caused by data
                // characters arriving at the port faster than they can be stored, or by the loss
                // of a character due to a hardware malfunction.
                incrementCounter(QModbusServerPrivate::Counter::BusCharacterOverrun);
                return;
            }

            const quint16 receivedCrc = quint8(buffer[buffer.size() - 2]) << 8
                                        | quint8(buffer[buffer.size() -1]);
            if (!matchingCRC(buffer.constData(), buffer.size() - 2, receivedCrc)) {
                qCWarning(QT_MODBUS) << "Ignoring request with wrong CRC";
                // The quantity of CRC errors encountered by the remote device since its last
                // restart, clear counters operation, or power–up.
                incrementCounter(QModbusServerPrivate::Counter::BusCommunicationError);
                return;
            }

            // The quantity of messages that the remote device has detected on the communications
            // system since its last restart, clear counters operation, or power–up.
            incrementCounter(QModbusServerPrivate::Counter::BusMessage);

            // Slave address is set to 0, this is a broadcast.
            m_processesBroadcast = (buffer.at(0) == 0);

            // If we do not process a Broadcast ...
            if (!q->processesBroadcast()) {
                // check if the slave address matches ...
                if (q->slaveAddress() != quint8(buffer.at(0))) {
                    // no, not our address! Ignore!
                    qCDebug(QT_MODBUS) << "Wrong slave address, expected" << q->slaveAddress()
                                       << "got" << quint8(buffer.at(0));
                    return;
                }
            } // else { Broadcast -> Slave Id will never match, deliberately ignore }

            // The quantity of messages addressed to the remote device, or broadcast, that the
            // remote device has processed since its last restart, clear counters operation, or
            // power–up.
            incrementCounter(QModbusServerPrivate::Counter::ServerMessage);

            // remove slave address, function code & CRC
            const QModbusRequest req(code, buffer.mid(2, pduSizeWithoutFcode));
            qCDebug(QT_MODBUS) << "Request PDU" << req;
            const QModbusResponse response = q->processRequest(req);

            if (q->processesBroadcast() || !response.isValid()) {
                // The quantity of messages addressed to the remote device for which it has
                // returned no response (neither a normal response nor an exception response),
                // since its last restart, clear counters operation, or power–up.
                incrementCounter(QModbusServerPrivate::Counter::ServerNoResponse);
                return;
            }

            qCDebug(QT_MODBUS) << "Response PDU:" << response;

            QByteArray result;
            QDataStream out(&result, QIODevice::WriteOnly);
            out << quint8(q->slaveAddress());
            out << response;
            out << calculateCRC(result, result.size());

            qCDebug(QT_MODBUS_LOW) << "Sending response (incl ADU):" << result.toHex();

            if (!m_serialPort->isOpen()) {
                qCDebug(QT_MODBUS) << "Requesting serial port has closed.";
                q->setError(QModbusRtuSerialSlave::tr("Requesting serial port is closed"),
                            QModbusDevice::WriteError);
                incrementCounter(QModbusServerPrivate::Counter::ServerNoResponse);
                return;
            }

            int writtenBytes = m_serialPort->write(result);
            if ((writtenBytes == -1) || (writtenBytes < result.size())) {
                qCDebug(QT_MODBUS) << "Cannot write requested response to serial port.";
                q->setError(QModbusRtuSerialSlave::tr("Could not write response to client"),
                            QModbusDevice::WriteError);
                incrementCounter(QModbusServerPrivate::Counter::ServerNoResponse);
                return;
            }

            if (response.isException()) {
                // The quantity of messages addressed to the remote device for which it returned a
                // server device busy exception response, since its last restart, clear counters
                // operation, or power–up.
                if (response.exceptionCode() == QModbusExceptionResponse::ServerDeviceBusy)
                    incrementCounter(QModbusServerPrivate::Counter::ServerBusy);

                // The quantity of messages addressed to the remote device for which it returned a
                // negative acknowledge (NAK) exception response, since its last restart, clear
                // counters operation, or power–up.
                if (response.exceptionCode() == QModbusExceptionResponse::NegativeAcknowledge)
                    incrementCounter(QModbusServerPrivate::Counter::ServerNAK);

                // The quantity of Modbus exception responses returned by the remote device since
                // its last restart, clear counters operation, or power–up.
                incrementCounter(QModbusServerPrivate::Counter::BusExceptionError);
            }
        });

        using TypeId = void (QSerialPort::*)(QSerialPort::SerialPortError);
        QObject::connect(m_serialPort, static_cast<TypeId>(&QSerialPort::error),
                         [this](QSerialPort::SerialPortError error) {
            if (error == QSerialPort::NoError)
                return;

            qCDebug(QT_MODBUS) << "QSerialPort error:" << error
                               << (m_serialPort ? m_serialPort->errorString() : QString());

            Q_Q(QModbusRtuSerialSlave);

            switch (error) {
            case QSerialPort::DeviceNotFoundError:
                q->setError(QModbusDevice::tr("Referenced serial device does not exist."),
                            QModbusDevice::ConnectionError);
                break;
            case QSerialPort::PermissionError:
                q->setError(QModbusDevice::tr("Cannot open serial device due to permissions."),
                            QModbusDevice::ConnectionError);
                break;
            case QSerialPort::OpenError:
            case QSerialPort::NotOpenError:
                q->setError(QModbusDevice::tr("Cannot open serial device."),
                            QModbusDevice::ConnectionError);
                break;
            case QSerialPort::ParityError:
                q->setError(QModbusDevice::tr("Parity error detected."),
                            QModbusDevice::ConfigurationError);
                incrementCounter(QModbusServerPrivate::Counter::BusCommunicationError);
                break;
            case QSerialPort::FramingError:
                q->setError(QModbusDevice::tr("Framing error detected."),
                            QModbusDevice::ConfigurationError);
                break;
            case QSerialPort::BreakConditionError:
                q->setError(QModbusDevice::tr("Break condition error detected."),
                            QModbusDevice::ConnectionError);
                break;
            case QSerialPort::WriteError:
                q->setError(QModbusDevice::tr("Write error."), QModbusDevice::WriteError);
                break;
            case QSerialPort::ReadError:
                q->setError(QModbusDevice::tr("Read error."), QModbusDevice::ReadError);
                break;
            case QSerialPort::ResourceError:
                q->setError(QModbusDevice::tr("Resource error."), QModbusDevice::ConnectionError);
                break;
            case QSerialPort::UnsupportedOperationError:
                q->setError(QModbusDevice::tr("Device operation is not supported error."),
                            QModbusDevice::ConfigurationError);
                break;
            case QSerialPort::TimeoutError:
                q->setError(QModbusDevice::tr("Timeout error."), QModbusDevice::TimeoutError);
                break;
            case QSerialPort::UnknownError:
                q->setError(QModbusDevice::tr("Unknown error."), QModbusDevice::UnknownError);
                break;
            default:
                qCDebug(QT_MODBUS) << "Unhandled QSerialPort error" << error;
                break;
            }
        });

        QObject::connect(m_serialPort, &QSerialPort::aboutToClose, [this]() {
            Q_Q(QModbusRtuSerialSlave);
            // update state if socket closure was caused by remote side
            if (q->state() != QModbusDevice::ClosingState)
                q->setState(QModbusDevice::UnconnectedState);
        });
    }

    void handleErrorOccurred(QSerialPort::SerialPortError);
    void serialPortReadyRead();
    void aboutToClose();

    QSerialPort *m_serialPort;
    bool m_processesBroadcast = false;
};

QT_END_NAMESPACE

#endif // QMODBUSRTUSERIALSLAVE_P_H

