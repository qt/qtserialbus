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

#include "qmodbustcpclient.h"
#include "qmodbustcpclient_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QModbusTcpClient
    \inmodule QtSerialBus
    \since 5.6

    \brief The QModbusTcpClient class is the interface class for Modbus TCP client device.

    QModbusTcpClient communicates with the Modbus backend providing users with a convenient API.
*/

/*!
    Constructs a QModbusTcpClient with the specified \a parent.
 */
QModbusTcpClient::QModbusTcpClient(QObject *parent)
    : QModbusClient(*new QModbusTcpClientPrivate, parent)
{
}

/*!
    Destroys the QModbusTcpClient instance.
*/
QModbusTcpClient::~QModbusTcpClient()
{
}

/*!
    \internal
*/
QModbusTcpClient::QModbusTcpClient(QModbusTcpClientPrivate &dd, QObject *parent)
    : QModbusClient(dd, parent)
{
}

/*!
    \fn void QModbusTcpClient::connectDevice(const QString &hostName, quint16 port = 502)

    Attempts to make a connection to \a hostName on the given \a port. If a connection is
    established, it emits stateChanged() with ModbusDeviceState::ConnectedState.

    At any point, the class can emit stateChanged() to signal the current device state or emit
    errorOccurred() to signal that an error occurred.

    \note \a hostName may be an IP address in string form (e.g., "43.195.83.32"), or it may be a
    host name (e.g., "example.com"). \a port is in native byte order and defaults to number 502.
*/

/*!
    \fn void QModbusTcpClient::connectDevice(const QHostAddress &address, quint16 port = 502)

    \overload

    Attempts to make a connection to \a address on port \a port.
*/

/*
    TODO: implement
*/
QModbusReplyEx *QModbusTcpClient::sendRequest(const QModbusDataUnit &request, int slaveId)
{
    Q_UNUSED(request)
    Q_UNUSED(slaveId)
    return Q_NULLPTR;
}

QT_END_NAMESPACE
