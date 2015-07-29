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

#include "qmodbusslave.h"

QT_BEGIN_NAMESPACE

/*!
    \class QModBusSlave
    \inmodule QtSerialBus
    \since 5.6

    \brief The QCanBusDevice class is the interface class for modbus.

    QCanBusDevice communicates with a modbus backend providing users with a convenient API.
    The modbus backend must be specified during the object creation.
 */

/*!
    Constructs a serial bus device with the specified \a parent.
 */
QModBusSlave::QModBusSlave(QObject *parent) :
    QObject(parent)
{
}

/*!
    \fn bool QModBusSlave::open()

    Connects QModBusSlave to Modbus network.
 */

/*!
    \fn void QModBusSlave::close()

    This function is responsible for closing the Modbus connection.

    \sa disconnectDevice()
 */

/*!
    \fn int QModBusSlave::slaveId()
    Multiple Modbus devices can be connected together on the same physical link,
    slave id will be used to filter received messages. Each slave should have unique id.
    Returns slave id.

    \sa setSlaveId()
 */

/*!
    \fn void QModBusSlave::setSlaveId(int id)
    Multiple Modbus devices can be connected together on the same physical link,
    slave id will be used to filter received messages. Each slave should have unique id.
    Sets \a id as slave id.

    \sa slaveId()
 */

QT_END_NAMESPACE
