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

#include <QtCore/qdatetime.h>
#include <QtCore/qurl.h>
#include <QtNetwork/qhostaddress.h>

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
    Q_D(QModbusTcpClient);
    d->setupTcpSocket();

    qsrand(QTime::currentTime().msec());
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
    Q_D(QModbusTcpClient);
    d->setupTcpSocket();

    qsrand(QTime::currentTime().msec());
}

/*!
    \reimp
*/
QModbusReply *QModbusTcpClient::sendReadRequest(const QModbusDataUnit &read, int slaveAddress)
{
    Q_D(QModbusTcpClient);

    if (!d->m_socket->isOpen()|| state() != QModbusDevice::ConnectedState) {
        setError(tr("Device not connected."), QModbusDevice::ConnectionError);
        qCWarning(QT_MODBUS) << "TCP client is not connected";
        return Q_NULLPTR;
    }

    QModbusRequest request = d->createReadRequest(read);
    if (!request.isValid())
        return Q_NULLPTR;

    return d->enqueueRequest(request, slaveAddress, read);
}

/*!
    \reimp
*/
QModbusReply *QModbusTcpClient::sendWriteRequest(const QModbusDataUnit &write, int slaveAddress)
{
    Q_D(QModbusTcpClient);

    if (!d->m_socket->isOpen() || state() != QModbusDevice::ConnectedState) {
        setError(tr("Device not connected."), QModbusDevice::ConnectionError);
        qCWarning(QT_MODBUS) << "TCP client is not connected";
        return Q_NULLPTR;
    }

    QModbusRequest request = d->createWriteRequest(write);
    if (!request.isValid())
        return Q_NULLPTR;

    return d->enqueueRequest(request, slaveAddress, write);
}

/*!
    \reimp
*/
QModbusReply *QModbusTcpClient::sendReadWriteRequest(const QModbusDataUnit &read,
                                                     const QModbusDataUnit &write, int slaveAddress)
{
    Q_D(QModbusTcpClient);

    if (!d->m_socket->isOpen() || state() != QModbusDevice::ConnectedState) {
        setError(tr("Device not connected."), QModbusDevice::ConnectionError);
        qCWarning(QT_MODBUS) << "TCP client is not connected";
        return Q_NULLPTR;
    }

    QModbusRequest request = d->createRWRequest(read, write);
    if (!request.isValid())
        return Q_NULLPTR;

    return d->enqueueRequest(request, slaveAddress, read); // only need to remember read

}

/*!
     \reimp
*/
bool QModbusTcpClient::open()
{
    if (state() == QModbusDevice::ConnectedState)
        return true;

    Q_D(QModbusTcpClient);
    if (d->m_socket->state() != QAbstractSocket::UnconnectedState)
        return false;

    const QUrl url = QUrl::fromUserInput(portName());
    if (!url.isValid()) {
        qCWarning(QT_MODBUS) << "Invalid host:" << url.host();
        return false;
    }

    d->m_socket->connectToHost(url.host(), url.port());

    return true;
}

/*!
     \reimp
*/
void QModbusTcpClient::close()
{
    Q_D(QModbusTcpClient);

    d->m_socket->disconnectFromHost();
}

QT_END_NAMESPACE
