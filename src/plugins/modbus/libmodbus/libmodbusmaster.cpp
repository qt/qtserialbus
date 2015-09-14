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

#if defined(Q_OS_UNIX)
# include <errno.h>
#endif

QT_BEGIN_NAMESPACE

LibModBusMaster::LibModBusMaster() :
    QModBusMaster(),
    connected(false)
{
}

bool LibModBusMaster::setDevice(QIODevice * /*transport*/, ApplicationDataUnit /*ADU*/)
{
    //TODO remove
    return true;
}

QModBusReply* LibModBusMaster::write(const QModBusDataUnit &request, int slaveId)
{
    QList<QModBusDataUnit> units;
    units.append(request);
    return write(units, slaveId);
}

QModBusReply* LibModBusMaster::write(const QList<QModBusDataUnit> &requests, int slaveId)
{
    if (requests.empty()) {
        setError(tr("Empty write request."), QModBusDevice::WriteError);
        return 0;
    }

    const QModBusDevice::ModBusTable writeTable(requests.first().registerType());

    if (writeTable != QModBusDevice::Coils
        && writeTable != QModBusDevice::HoldingRegisters) {
        setError(tr("Trying to write read only table."), QModBusDevice::WriteError);
        return 0;
    }

    int address = requests.first().startAddress();

    for (int i = 1; i < requests.size(); i++) {
        address++;
        if (requests.at(i).registerType() != writeTable) {
            setError(tr("Data units in write request must be from same table."),
                     QModBusDevice::WriteError);
            return 0;
        }

        if (requests.at(i).startAddress() != writeTable) {
            setError(tr("Data units in write request must be adjacent to each other."),
                     QModBusDevice::WriteError);
            return 0;
        }
    }
    Reply *reply = new Reply();
    reply->write(requests, slaveId, context);
    return reply;
}

QModBusReply* LibModBusMaster::read(QModBusDataUnit &request, int slaveId)
{
    QList<QModBusDataUnit> units;
    units.append(request);
    return read(units, slaveId);
}

QModBusReply* LibModBusMaster::read(QList<QModBusDataUnit> &requests, int slaveId)
{
    if (requests.empty()) {
        setError(tr("Empty read reaquest."), QModBusDevice::ReadError);
        return 0;
    }

    const QModBusDevice::ModBusTable readTable(requests.first().registerType());
    int address = requests.first().startAddress();

    for (int i = 1; i < requests.size(); i++) {
        address++;
        if (requests.at(i).registerType() != readTable) {
            setError(tr("Data units in write request must be from same table."),
                     QModBusDevice::ReadError);
            return 0;
        }
        if (requests.at(i).startAddress() != address) {
            setError(tr("Data units in read request must be adjacent to each other."),
                     QModBusDevice::ReadError);
            return 0;
        }
    }

    Reply *reply = new Reply();
    reply->read(requests, slaveId, context);
    return reply;
}

bool LibModBusMaster::open()
{
    if (connected)
        return true;

    QString location = portName();
    if (!location.isEmpty())
        location = portNameToSystemLocation(location);

    if (location.isEmpty()) {
        setError(tr("No portname for serial port specified."),
                 QModBusDevice::ConnectionError);
        return false;
    }

    //TODO For now the parameters are fixed -> need to add API to adjust this
    char parity = 'N';
    int baud = 9600;
    int stopBits = 1;
    int dataBits = 8;

    context = modbus_new_rtu(location.toLatin1(),
                             baud, parity, dataBits, stopBits);
    if (!context) {
        setError(qt_error_string(errno), QModBusDevice::ConnectionError);
        return false;
    }

    modbus_set_debug(context, TRUE);
    if (modbus_connect(context) == -1) {
        setError(qt_error_string(errno), QModBusDevice::ConnectionError);
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

QString LibModBusMaster::portNameToSystemLocation(const QString &source) const
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
    return QString();
#endif
}

QT_END_NAMESPACE
