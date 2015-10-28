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

#include "qmodbusrtuserialslave.h"
#include "qmodbusrtuserialslave_p.h"

#include <QtCore/qloggingcategory.h>


QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_MODBUS)
Q_DECLARE_LOGGING_CATEGORY(QT_MODBUS_LOW)

/*!
    \class QModbusRtuSerialSlave
    \inmodule QtSerialBus
    \since 5.6

    \brief The QModbusRtuSerialSlave class represents a Modbus server
    that uses a serial port for its communication with the Modbus client.

    Communication via Modbus requires the interaction between a single Modbus client instance and
    multiple Modbus server. This class provides the Modbus server implementation via a serial port.

    Since multiple Modbus server instances can interact with a Modbus client at the same time
    (using a serial bus), servers are identified by their \l slaveAddress().
*/

/*!
    Constructs a serial Modbus slave with the specified \a parent.
 */
QModbusRtuSerialSlave::QModbusRtuSerialSlave(QObject *parent)
    : QModbusServer(*new QModbusRtuSerialSlavePrivate, parent)
{
    Q_D(QModbusRtuSerialSlave);
    d->setupSerialPort();
}

/*!
    \internal
 */
QModbusRtuSerialSlave::~QModbusRtuSerialSlave()
{
}


/*!
    \internal
 */
QModbusRtuSerialSlave::QModbusRtuSerialSlave(QModbusRtuSerialSlavePrivate &dd, QObject *parent)
    : QModbusServer(dd, parent)
{
    Q_D(QModbusRtuSerialSlave);
    d->setupSerialPort();
}

/*!
    \reimp
 */
bool QModbusRtuSerialSlave::open()
{
    if (state() == QModbusDevice::ConnectedState)
        return true;

    Q_D(QModbusRtuSerialSlave);
    d->m_serialPort->setPortName(portName());
    if (d->m_serialPort->open(QIODevice::ReadWrite))
        setState(QModbusDevice::ConnectedState);

    return (state() == QModbusDevice::ConnectedState);
}

/*!
    \reimp
 */
void QModbusRtuSerialSlave::close()
{
    Q_D(QModbusRtuSerialSlave);

    if (d->m_serialPort->isOpen())
        d->m_serialPort->close();

    setState(QModbusDevice::UnconnectedState);
}

void QModbusRtuSerialSlavePrivate::handleErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError)
        return;

    qCDebug(QT_MODBUS) << "QSerialPort error:" << error
                       << ((m_serialPort != Q_NULLPTR) ? m_serialPort->errorString() : QString());

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
}

void QModbusRtuSerialSlavePrivate::serialPortReadyRead()
{
    Q_Q(QModbusRtuSerialSlave);

    const int size = m_serialPort->size();
    const QByteArray pendingBuffer = m_serialPort->read(size);

    // Index                         -> description
    // SlaveId                       -> 1 byte
    // FunctionCode                  -> 1 byte
    // FunctionCode specific content -> 0-252 bytes
    // CRC                           -> 2 bytes

    QModbusPdu::FunctionCode code = QModbusPdu::Invalid;
    // function code available?
    if (pendingBuffer.size() >= 2) {
        code = QModbusRequest::FunctionCode(pendingBuffer.at(1));
    } else {
        qCWarning(QT_MODBUS) << "Invalid modbus PDU received";
        return;
    }

    int aduSize = 2; //slaveId & function code counted
    int pduSize_without_fcode = QModbusRequest::calculateDataSize(code, pendingBuffer.mid(2));
    if (pduSize_without_fcode < 0) {
        qCWarning(QT_MODBUS) << "Cannot calculate PDU size for fcode" << code;
        return;
    }
    aduSize += pduSize_without_fcode;
    //obtain CRC which is last 2 bytes
    quint16 receivedCrc = pendingBuffer[aduSize] << 8 | pendingBuffer[aduSize+1];
    aduSize += 2; // crc size

    const QByteArray completeAduFrame = pendingBuffer.left(aduSize);
    if (completeAduFrame.size() < aduSize) {
        // TODO should such cases wait for next bytes to arrive and keep m_pendingBuffer around
        qCWarning(QT_MODBUS) << "ADU too short, ignoring pending frame";
        return;
    }

    if (aduSize > size)
        qCWarning(QT_MODBUS) << "Ignoring remaining data beyond expected ADU size";

    qCDebug(QT_MODBUS_LOW) << "Received request (incl ADU)" << completeAduFrame.toHex();
    if (QT_MODBUS().isDebugEnabled() && (aduSize > size)) {
        qCDebug(QT_MODBUS_LOW) << "Pending buffer:" << pendingBuffer.toHex();
        qCDebug(QT_MODBUS) << "Ignoring remaining data beyond expected ADU size";
    }


    // for this slave address ?
    // TODO deal with broadcast id 0
    if (q->slaveAddress() != completeAduFrame.at(0)) {
        //ignore wrong slaveId
        qCDebug(QT_MODBUS) << "Wrong slave address, expected" << q->slaveAddress() << "got" << (quint8) completeAduFrame.at(0);
        return;
    }

    if (checkCRC(completeAduFrame, completeAduFrame.size(), receivedCrc)) {
        qCWarning(QT_MODBUS) << "Ignoring request with wrong crc";
        return;
    }

    //remove slave address, fcode & crc
    QModbusRequest req(code, completeAduFrame.mid(2, completeAduFrame.size()-4));
    qCDebug(QT_MODBUS) << "Request PDU" << req;
    QModbusResponse response = q->processRequest(req);

    qCDebug(QT_MODBUS) << "Response PDU:" << response;

    QByteArray result;
    QDataStream out(&result, QIODevice::WriteOnly);
    out << (quint8) q->slaveAddress();
    out << response;

    quint16 resultCrc = calculateCRC(result, result.size());
    out << resultCrc;

    qCDebug(QT_MODBUS_LOW) << "Sending response (incl ADU):" << result.toHex();
    m_serialPort->write(result);
}

void QModbusRtuSerialSlavePrivate::aboutToClose()
{
    Q_Q(QModbusRtuSerialSlave);
    q->close();
}

#include "moc_qmodbusrtuserialslave.cpp"

QT_END_NAMESPACE


