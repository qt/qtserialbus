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

#include "qmodbustcpserver.h"
#include "qmodbustcpserver_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QModbusTcpServer
    \inmodule QtSerialBus
    \since 5.6

    \brief The QModbusTcpServer class is the interface class for Modbus TCP sever device.

    Modbus TCP networks can have multiple servers. Servers are read/written by a client device
    represented by \l QModbusTcpClient. QModbusTcpServer communicates with a Modbus backend,
    providing users with a convenient API.
*/

/*!
    Constructs a QModbusTcpServer with the specified \a parent.
*/
QModbusTcpServer::QModbusTcpServer(QObject *parent)
    : QModbusServer(*new QModbusTcpServerPrivate, parent)
{
    Q_D(QModbusTcpServer);
    d->setupTcpServer();
}

/*!
    Destroys the QModbusTcpServer instance.
*/
QModbusTcpServer::~QModbusTcpServer()
{
}

/*!
    \internal
*/
QModbusTcpServer::QModbusTcpServer(QModbusTcpServerPrivate &dd, QObject *parent)
    : QModbusServer(dd, parent)
{
    Q_D(QModbusTcpServer);
    d->setupTcpServer();
}

/*!
    \reimp
*/
bool QModbusTcpServer::open()
{
    if (state() == QModbusDevice::ConnectedState)
        return true;

    const QStringList parts =  portName().split(QLatin1Char(':'), QString::SkipEmptyParts);
    if (parts.count() < 1) {
        setError(tr("Invalid host and port for TCP communication specified."),
            QModbusDevice::ConnectionError);
        return false;
    }

    bool ok = false;
    const quint16 port = parts.value(1).toInt(&ok);
    if (!ok) {
        setError(tr("Invalid port for TCP communication specified."),
            QModbusDevice::ConnectionError);
        return false;
    }

    Q_D(QModbusTcpServer);
    if (d->m_tcpServer->listen(QHostAddress(parts.value(0)), port))
        setState(QModbusDevice::ConnectedState);
    else
        setError(d->m_tcpServer->errorString(), QModbusDevice::ConnectionError);

    return state() == QModbusDevice::ConnectedState;
}

/*!
    \reimp
*/
void QModbusTcpServer::close()
{
    Q_D(QModbusTcpServer);

    if (d->m_tcpServer->isListening())
        d->m_tcpServer->close();

    foreach (auto socket, d->connections)
        socket->disconnectFromHost();

    setState(QModbusDevice::UnconnectedState);
}

/*!
    Processes a Modbus client \a request and returns a Modbus response.

    \note The following Modbus function codes are filtered as they are serial
    line only according to the Modbus Application Protocol Specification 1.1b:
    \list
        \li \l QModbusRequest::ReadExceptionStatus
        \li \l QModbusRequest::Diagnostics
        \li \l QModbusRequest::GetCommEventCounter
        \li \l QModbusRequest::GetCommEventLog
        \li \l QModbusRequest::ReportServerId
    \endlist
    A request to the TCP server will be answered with a Modbus exception
    response with exception code QModbusExceptionResponse::IllegalFunction.

*/
QModbusResponse QModbusTcpServer::processRequest(const QModbusPdu &request)
{
    switch (request.functionCode()) {
    case QModbusRequest::ReadExceptionStatus:
    case QModbusRequest::Diagnostics:
    case QModbusRequest::GetCommEventCounter:
    case QModbusRequest::GetCommEventLog:
    case QModbusRequest::ReportServerId:
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalFunction);
    default:
        break;
    }
    return QModbusServer::processRequest(request);
}

QT_END_NAMESPACE
