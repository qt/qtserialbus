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

#include "qmodbus.h"

#include <QtCore/qdebug.h>
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

        QObject::connect(m_serialPort, SIGNAL(readyRead()),
                         q, SLOT(serialPortReadyRead()));
        QObject::connect(m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)),
                         q, SLOT(handleErrorOccurred(QSerialPort::SerialPortError)));
    }

    quint8 lengthOfFunctionCodeHeader(QModbusPdu::FunctionCode code) const
    {
        switch (code) {
        case QModbusPdu::ReadCoils:
        case QModbusPdu::ReadDiscreteInputs:
        case QModbusPdu::ReadHoldingRegisters:
        case QModbusPdu::ReadInputRegisters:
        case QModbusPdu::WriteSingleCoil:
        case QModbusPdu::WriteSingleRegister:
            return 4;
        case QModbusPdu::WriteMultipleRegisters:
        case QModbusPdu::WriteMultipleCoils:
            return 5;
        case QModbusPdu::MaskWriteRegister:
            return 6;
        case QModbusPdu::ReadWriteMultipleRegisters:
            return 9;
        default:
            qWarning() << "Size for FunctionCode not known" << code;
            return 0;
        }
    }

    void handleErrorOccurred(QSerialPort::SerialPortError);
    void serialPortReadyRead();
    void aboutToClose();

    QSerialPort *m_serialPort;
    QByteArray m_pendingBuffer;
};

QT_END_NAMESPACE

#endif // QMODBUSRTUSERIALSLAVE_P_H

