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
QModbusTcpServer::QModbusTcpServer(QObject *parent) :
    QModbusServer(*new QModbusTcpServerPrivate, parent)
{
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
QModbusTcpServer::QModbusTcpServer(QModbusTcpServerPrivate &dd, QObject *parent) :
    QModbusServer(dd, parent)
{
}

/*!
    \fn void QModbusTcpServer::listen(const QString &address, quint16 port = 502)

    Tells the server to listen for incoming connections on address \a address and port \a port.
    If a connection is established, it emits stateChanged() with ModbusDeviceState::ConnectedState.

    At any point, the class can emit stateChanged() to signal the current device state or emit
    errorOccurred() to signal that an error occurred.
*/

/*!
    \fn void QModbusTcpServer::listen(const QHostAddress &address, quint16 port = 502)

    \overload

    Tells the server to listen for incoming connections on address \a address and port \a port.
*/

QT_END_NAMESPACE
