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

#include "libmodbusslave.h"

#include <QtCore/qdebug.h>

#if defined(Q_OS_WIN)
#include <winsock2.h>
#elif defined(Q_OS_UNIX)
#include <errno.h>
#include <sys/select.h>
#endif

#define MODBUS_SERIAL_ADU_SIZE 256
#define FUNCTION_ID 1
#define START_ADDRESS_HI 2
#define START_ADDRESS_LO 3
#define QUANTITY_HI 4
#define QUANTITY_LO 5

QT_BEGIN_NAMESPACE

LibModBusSlave::LibModBusSlave() :
    QModbusServer(),
    context(0),
    mapping(modbus_mapping_new(0,0,0,0)),
    connected(false),
    slave(1)
{
}

LibModBusSlave::~LibModBusSlave()
{
    close();
    modbus_mapping_free(mapping);
    mapping = 0;
}

bool LibModBusSlave::setMap(const QModbusDataUnitMap &map)
{
    if (connected) {
        // TODO this limitation is not required. As long as the slave
        // is not serving an active request, the mapping can be changed.
        setError(tr("Cannot set maps when slave is connected to the network."),
                 QModbusDevice::WriteError);
        return false;
    }

    mappingTable[QModbusDataUnit::DiscreteInputs]
            = map.value(QModbusDataUnit::DiscreteInputs).valueCount();
    mappingTable[QModbusDataUnit::Coils]
            = map.value(QModbusDataUnit::Coils).valueCount();
    mappingTable[QModbusDataUnit::InputRegisters]
            = map.value(QModbusDataUnit::InputRegisters).valueCount();
    mappingTable[QModbusDataUnit::HoldingRegisters]
            = map.value(QModbusDataUnit::HoldingRegisters).valueCount();

    return true;
}

void LibModBusSlave::handleError(int errorNumber)
{
    setError(qt_error_string(errorNumber), QModbusDevice::ReadError);
    close();
}

bool LibModBusSlave::open()
{
    if (connected)
        return true;

    QString location = portName();
    if (!location.isEmpty())
        location = portNameToSystemLocation(location);

    if (location.isEmpty()) {
        setError(tr("No portname for serial port specified."),
                 QModbusDevice::ConnectionError);
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
        setError(qt_error_string(errno), QModbusDevice::ConnectionError);
        return false;
    }
    modbus_set_slave(context, slave);

    modbus_set_debug(context, TRUE);
    if (modbus_connect(context) == -1) {
        setError(qt_error_string(errno), QModbusDevice::ConnectionError);
        close();
        return false;
    }

    if (mapping) {
        modbus_mapping_free(mapping);
        mapping = 0;
    }
    mapping = modbus_mapping_new(mappingTable[QModbusDataUnit::Coils],
                                 mappingTable[QModbusDataUnit::DiscreteInputs],
                                 mappingTable[QModbusDataUnit::HoldingRegisters],
                                 mappingTable[QModbusDataUnit::InputRegisters]);
    if (mapping == NULL) {
        setError(qt_error_string(errno), QModbusDevice::ConnectionError);
        return false;
    }

    listener = new ListenThread();
    listener->context = context;
    listener->mapping = mapping;
    listener->moveToThread(&thread);
    connect(&thread, &QThread::finished, listener.data(), &QObject::deleteLater);
    connect(this, &LibModBusSlave::operate, listener.data(), &ListenThread::doWork);
    connect(listener.data(), &ListenThread::error, this, &LibModBusSlave::handleError);
    connect(listener.data(), &ListenThread::dataWritten, this, &LibModBusSlave::dataWritten);
    thread.start();
    emit operate();
    connected = true;
    setState(QModbusDevice::ConnectedState);
    return true;
}

void LibModBusSlave::close()
{
    thread.requestInterruption();
    thread.quit();
    thread.wait();
    listener.clear();

    modbus_close(context);
    modbus_free(context);
    context = 0;
    connected = false;
    setState(QModbusDevice::UnconnectedState);
}

QString LibModBusSlave::portNameToSystemLocation(const QString &source)
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

void ListenThread::doWork()
{
    quint8 query[MODBUS_SERIAL_ADU_SIZE];

    while (!QThread::currentThread()->isInterruptionRequested()) {
        // The FD_ZERO and FD_SET need to be done at every loop.
        int socket = modbus_get_socket(context);
        fd_set rset;
        FD_ZERO(&rset);
        FD_SET(socket, &rset);

        // Because modbus_receive doesn't obey timeouts properly, first wait with select until there
        // is something to read.
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int req = select(socket + 1, &rset, NULL, NULL, &timeout);

        // Read only if select returned that there is something to read
        if (req > 0)
            req = modbus_receive(context, query);

        // Check for errors on both select and modbus_receive
        if (req == -1) {
            emit error(errno);
            break;
        }

        // Send reply only if data was received.
        if (req > 1) {
            modbus_reply(context, query, req, mapping);

            //emit proper signal(s)
            const int functionId = query[FUNCTION_ID];
            quint16 startAddress = 0;
            quint16 quantity = 0;
            QModbusDataUnit::RegisterType table;
            switch (functionId) {
            case ReadCoils:
            case ReadDiscreteInputs:
            case ReadHoldingRegisters:
            case ReadInputRegisters:
                break;
            case WriteSingleCoil:
                table = QModbusDataUnit::Coils;
                startAddress = (query[START_ADDRESS_HI] << 8) + query[START_ADDRESS_LO];
                emit dataWritten(table, startAddress, 1);
                break;
            case WriteMultipleCoils:
                table = QModbusDataUnit::Coils;
                startAddress = (query[START_ADDRESS_HI] << 8) + query[START_ADDRESS_LO];
                quantity = (query[QUANTITY_HI] << 8) + query[QUANTITY_LO];
                emit dataWritten(table, startAddress, quantity);
                break;
            case WriteSingleRegister:
                table = QModbusDataUnit::HoldingRegisters;
                startAddress = (query[START_ADDRESS_HI] << 8) + query[START_ADDRESS_LO];
                emit dataWritten(table, startAddress, 1);
                break;
            case WriteMultipleRegisters:
                table = QModbusDataUnit::HoldingRegisters;
                startAddress = (query[START_ADDRESS_HI] << 8) + query[START_ADDRESS_LO];
                quantity = (query[QUANTITY_HI] << 8) + query[QUANTITY_LO];
                emit dataWritten(table, startAddress, quantity);
                break;
            case ReadWriteRegisters:
                table = QModbusDataUnit::HoldingRegisters;
                startAddress = (query[START_ADDRESS_HI] << 8) + query[START_ADDRESS_LO];
                quantity = (query[QUANTITY_HI] << 8) + query[QUANTITY_LO];
                emit dataWritten(table, startAddress, quantity);
                break;
            default:
                break;
            }
        }
    }
}

QT_END_NAMESPACE
