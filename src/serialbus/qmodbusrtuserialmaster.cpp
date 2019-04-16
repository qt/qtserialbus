/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "qmodbusrtuserialmaster.h"
#include "qmodbusrtuserialmaster_p.h"

#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_MODBUS)
Q_DECLARE_LOGGING_CATEGORY(QT_MODBUS_LOW)

/*!
    \class QModbusRtuSerialMaster
    \inmodule QtSerialBus
    \since 5.8

    \brief The QModbusRtuSerialMaster class represents a Modbus client
    that uses a serial bus for its communication with the Modbus server.

    Communication via Modbus requires the interaction between a single
    Modbus client instance and multiple Modbus servers. This class
    provides the client implementation via a serial port.
*/

/*!
    Constructs a serial Modbus master with the specified \a parent.
*/
QModbusRtuSerialMaster::QModbusRtuSerialMaster(QObject *parent)
    : QModbusClient(*new QModbusRtuSerialMasterPrivate, parent)
{
    Q_D(QModbusRtuSerialMaster);
    d->setupSerialPort();
}

/*!
    \internal
*/
QModbusRtuSerialMaster::~QModbusRtuSerialMaster()
{
    close();
}

/*!
    Returns the amount of microseconds for the silent interval between two
    consecutive Modbus messages.

    \sa setInterFrameDelay()
*/
int QModbusRtuSerialMaster::interFrameDelay() const
{
    Q_D(const QModbusRtuSerialMaster);
    return d->m_interFrameDelayMilliseconds * 1000;
}

/*!
    Sets the amount of \a microseconds for the silent interval between two
    consecutive Modbus messages. By default, the class implementation will use
    a pre-calculated value according to the Modbus specification. A active or
    running connection is not affected by such delay changes.

    \note If \a microseconds is set to -1 or \a microseconds is less than the
    pre-calculated delay then this pre-calculated value is used as frame delay.
*/
void QModbusRtuSerialMaster::setInterFrameDelay(int microseconds)
{
    Q_D(QModbusRtuSerialMaster);
    d->m_interFrameDelayMilliseconds = qCeil(qreal(microseconds) / 1000.);
    d->calculateInterFrameDelay();
}

/*!
    \since 5.13

    Returns the amount of milliseconds for the silent interval between a Modbus
    broadcast and a consecutive Modbus messages. The default value is set to
    \c 100 milliseconds.
*/
int QModbusRtuSerialMaster::turnaroundDelay() const
{
    Q_D(const QModbusRtuSerialMaster);
    return d->m_turnaroundDelay;
}

/*!
    \since 5.13

    Sets the amount of milliseconds for the silent interval between a Modbus
    broadcast and a consecutive Modbus messages to \a turnaroundDelay.
    Typically the turnaround delay is in the range of \c 100 to \c 200
    milliseconds.
*/
void QModbusRtuSerialMaster::setTurnaroundDelay(int turnaroundDelay)
{
    Q_D(QModbusRtuSerialMaster);
    d->m_turnaroundDelay = turnaroundDelay;
}

/*!
    \internal
*/
QModbusRtuSerialMaster::QModbusRtuSerialMaster(QModbusRtuSerialMasterPrivate &dd, QObject *parent)
    : QModbusClient(dd, parent)
{
    Q_D(QModbusRtuSerialMaster);
    d->setupSerialPort();
}

/*!
     \reimp

     \note When calling this function, existing buffered data is removed from
     the serial port.
*/
bool QModbusRtuSerialMaster::open()
{
    if (state() == QModbusDevice::ConnectedState)
        return true;

    Q_D(QModbusRtuSerialMaster);
    d->setupEnvironment(); // to be done before open
    if (d->m_serialPort->open(QIODevice::ReadWrite)) {
        setState(QModbusDevice::ConnectedState);
        d->m_serialPort->clear(); // only possible after open
    } else {
        setError(d->m_serialPort->errorString(), QModbusDevice::ConnectionError);
    }
    return (state() == QModbusDevice::ConnectedState);
}

/*!
     \reimp
*/
void QModbusRtuSerialMaster::close()
{
    if (state() == QModbusDevice::UnconnectedState)
        return;

    setState(QModbusDevice::ClosingState);

    Q_D(QModbusRtuSerialMaster);

    if (d->m_serialPort->isOpen())
        d->m_serialPort->close();

    int numberOfAborts = 0;
    while (!d->m_queue.isEmpty()) {
        // Finish each open reply and forget them
        QModbusRtuSerialMasterPrivate::QueueElement elem = d->m_queue.dequeue();
        if (!elem.reply.isNull()) {
            elem.reply->setError(QModbusDevice::ReplyAbortedError,
                                 QModbusClient::tr("Reply aborted due to connection closure."));
            numberOfAborts++;
        }
    }

    if (numberOfAborts > 0)
        qCDebug(QT_MODBUS_LOW) << "(RTU client) Aborted replies:" << numberOfAborts;

    setState(QModbusDevice::UnconnectedState);
}

QT_END_NAMESPACE
