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

#include "libmodbusmaster.h"

#include <QtCore/qdebug.h>
#include <QtCore/qdatastream.h>

QT_BEGIN_NAMESPACE

LibModBusMaster::LibModBusMaster(QSerialPort *transport) :
    QModBusMaster(),
    serialPort(transport),
    connected(false),
    adu(QModBusDevice::RemoteTerminalUnit)
{
}

bool LibModBusMaster::setADU(ApplicationDataUnit adu)
{
    //Only RTU supported at the moment
    if (adu == QModBusDevice::RemoteTerminalUnit)
        return true;
    return false;
}

QModBusReply* LibModBusMaster::write(const QModBusDataUnit &request)
{
    Q_UNUSED(request);
    return 0;
}

QModBusReply* LibModBusMaster::write(const QList<QModBusDataUnit> &requests)
{
    Q_UNUSED(requests);
    return 0;
}

QModBusReply* LibModBusMaster::read(QModBusDataUnit &request, int slaveId)
{
    QList<QModBusDataUnit> units;
    units.append(request);
    return read(units, slaveId);
}

QModBusReply* LibModBusMaster::read(QList<QModBusDataUnit> &requests, int slaveId)
{
    if (requests.empty())
        return 0;

    const QModBusDevice::ModBusTable readTable(requests.first().tableType());
    int address = requests.first().address();

    for (int i = 1; i < requests.size(); i++) {
        address++;
        if ((requests.at(i).tableType() != readTable) ||
            (requests.at(i).address() != address))
            return 0;
    }

    Reply *reply = new Reply();
    reply->read(requests.first().tableType(), requests.first().address(),
                requests.size(), slaveId, context);
    return reply;
}

bool LibModBusMaster::open()
{
    if (connected)
        return true;

    QChar parity;

    switch (serialPort->parity()) {
    case QSerialPort::NoParity:
        parity = 'N';
        break;
    case QSerialPort::EvenParity:
        parity = 'E';
        break;
    case QSerialPort::OddParity:
        parity = 'O';
        break;
    default:
        return false;
    }

    QString location = portNameToSystemLocation(serialPort->portName());

    context = modbus_new_rtu(location.toLatin1(),
                             serialPort->baudRate(),
                             parity.toLatin1(),
                             serialPort->dataBits(),
                             serialPort->stopBits());
    if (context == NULL) {
        qWarning() << "Unable to create the libmodbus context";
        return false;
    }

    modbus_set_debug(context, TRUE);
    if (modbus_connect(context) == -1) {
        qWarning() << qt_error_string(errno);
        close();
        return false;
    }

    connected = true;
    setState(QModBusDevice::ConnectedState);
    return true;
}

void LibModBusMaster::close()
{
    connected = false;
    setState(QModBusDevice::UnconnectedState);
}

QString LibModBusMaster::portNameToSystemLocation(QString source)
{
#if defined(Q_OS_WINCE)
    return source.endsWith(QLatin1Char(':'))
            ? source : (source + QLatin1Char(':'));
#elif defined(Q_OS_WIN32)
    return source.startsWith(QLatin1String("COM"))
            ? (QLatin1String("\\\\.\\") + source) : source;
#elif defined(Q_OS_UNIX)
    return (source.startsWith(QLatin1Char('/'))
            || source.startsWith(QLatin1String("./"))
            || source.startsWith(QLatin1String("../")))
            ? source : (QLatin1String("/dev/") + source);
#else
#  error Unsupported OS
#endif
}

QT_END_NAMESPACE
