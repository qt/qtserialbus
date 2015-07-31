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

#include "libmodbusbackend.h"

#include <QtCore/qdebug.h>

#if defined(Q_OS_WIN)
#include <winsock2.h>
#elif defined(Q_OS_UNIX)
#include <sys/select.h>
#endif

//TODO: proper error handling
//TODO: read/write mapping

const int MODBUS_SERIAL_ADU_SIZE = 256;

QT_BEGIN_NAMESPACE

LibModBusBackend::LibModBusBackend(QSerialPort *transport) :
    QModBusSlave(),
    serialPort(transport),
    context(0),
    mapping(modbus_mapping_new(0,0,0,0)),
    connected(false),
    slave(1)
{
}

LibModBusBackend::~LibModBusBackend()
{
    close();
    modbus_mapping_free(mapping);
    mapping = 0;
}

bool LibModBusBackend::setMapping(int discreteInputMax, int coilMax,
                                  int inputRegisterMax, int holdingRegisterMax)
{
    if (connected)
        return false;
    if (mapping) {
        modbus_mapping_free(mapping);
        mapping = 0;
    }
    mapping = modbus_mapping_new(discreteInputMax, coilMax,
                                 inputRegisterMax, holdingRegisterMax);
    if (mapping == NULL) {
        qWarning() << qt_error_string(errno);
        modbus_mapping_free(mapping);
        mapping = 0;
        return false;
    }
    return true;
}

bool LibModBusBackend::open()
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
    modbus_set_slave(context, slave);

    modbus_set_debug(context, TRUE);
    if (modbus_connect(context) == -1) {
        qWarning() << qt_error_string(errno);
        close();
        return false;
    }

    listener = new ListenThread();
    listener->context = context;
    listener->mapping = mapping;
    listener->moveToThread(&thread);
    connect(&thread, &QThread::finished, listener, &QObject::deleteLater);
    connect(this, &LibModBusBackend::operate, listener, &ListenThread::doWork);
    connect(listener, &ListenThread::fail, this, &LibModBusBackend::close);
    thread.start();
    emit operate();
    connected = true;
    setState(ConnectedState);
    return true;
}

void LibModBusBackend::close()
{
    thread.requestInterruption();
    thread.quit();
    thread.wait();
    listener.clear();

    modbus_close(context);
    modbus_free(context);
    context = 0;
    connected = false;
    setState(UnconnectedState);
}

int LibModBusBackend::slaveId() const
{
    return slave;
}

void LibModBusBackend::setSlaveId(int id)
{
    slave = id;
    modbus_set_slave(context, slave);
}

QString LibModBusBackend::portNameToSystemLocation(QString source)
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

void ListenThread::doWork()
{
    quint8 query[MODBUS_SERIAL_ADU_SIZE];
    struct timeval timeout;
    struct timeval *timeoutptr = &timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    while (!QThread::currentThread()->isInterruptionRequested()) {
        // The FD_ZERO and FD_SET need to be done at every loop.
        int socket = modbus_get_socket(context);
        fd_set rset;
        FD_ZERO(&rset);
        FD_SET(socket, &rset);

        // Because modbus_receive doesn't obey timeouts properly, first wait with select until there
        // is something to read.
        int req = select(socket+1, &rset, NULL, NULL, timeoutptr);

        // Read only if select returned that there is something to read
        if (req > 0)
            req = modbus_receive(context, query);

        // Check for errors on both select and modbus_receive
        if (req == -1) {
            qWarning() << qt_error_string(errno);
            emit fail();
            break;
        }

        // Send reply only if data was received.
        if (req > 0)
            modbus_reply(context, query, req, mapping);
    }
}

QT_END_NAMESPACE
