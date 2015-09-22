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

#include "qmodbusserialslave.h"
#include "qmodbusserialslave_p.h"

#include <QtSerialBus/qmodbus.h>


QT_BEGIN_NAMESPACE

QModBusSerialSlave::QModBusSerialSlave(QObject *parent)
    : QModBusSlave(*new QModBusSerialSlavePrivate, parent)
{
    Q_D(QModBusSerialSlave);
    d->setupMaster();
}

QModBusSerialSlave::~QModBusSerialSlave()
{
}

/*!
    \reimp
 */
bool QModBusSerialSlave::connectDevice()
{
    // reimp to avoid QModBusDevice implementaion
    // we want to use a forwarded version.

    Q_D(QModBusSerialSlave);

    if (!d->pluginMaster)
        return false;

    d->pluginMaster->setPortName(portName());

    return d->pluginMaster->connectDevice();
}


QModBusSerialSlave::QModBusSerialSlave(QModBusSerialSlavePrivate &dd,
                                       QObject *parent)
    : QModBusSlave(dd, parent)
{
    Q_D(QModBusSerialSlave);
    d->setupMaster();
}

bool QModBusSerialSlave::setMap(const QModBusRegister &newRegister)
{
    Q_D(QModBusSerialSlave);

    if (!d->pluginMaster)
        return false;

    return d->pluginMaster->setMap(newRegister);
}

/*!
    \reimp
 */
void QModBusSerialSlave::setSlaveId(int id)
{
    Q_D(QModBusSerialSlave);

    if (!d->pluginMaster)
        return;

    d->pluginMaster->setSlaveId(id);
}

/*!
    \reimp
 */
int QModBusSerialSlave::slaveId() const
{
    Q_D(const QModBusSerialSlave);

    if (!d->pluginMaster)
        return -1;

    return d->pluginMaster->slaveId();
}

bool QModBusSerialSlave::data(QModBusRegister::RegisterType table,
                              quint16 address, quint16 *data)
{
    Q_D(QModBusSerialSlave);

    if (!d->pluginMaster)
        return false;

    return d->pluginMaster->data(table, address, data);
}

bool QModBusSerialSlave::setData(QModBusRegister::RegisterType table,
                                 quint16 address, quint16 data)
{
    Q_D(QModBusSerialSlave);

    if (!d->pluginMaster)
        return false;

    return d->pluginMaster->setData(table, address, data);
}

bool QModBusSerialSlave::open()
{
    return false;
}

void QModBusSerialSlave::close()
{
    Q_D(QModBusSerialSlave);

    if (!d->pluginMaster)
        return;

    d->pluginMaster->disconnectDevice();
}


void QModBusSerialSlavePrivate::handleStateChanged(
                                    QModBusDevice::ModBusDeviceState state)
{
    Q_Q(QModBusSerialSlave);
    q->setState(state);
}

void QModBusSerialSlavePrivate::handleErrorOccurred(
                                    QModBusDevice::ModBusError)
{
    Q_Q(QModBusSerialSlave);
    q->setError(pluginMaster ? pluginMaster->errorString() : QString(),
                error);
}

#include "moc_qmodbusserialslave.cpp"

QT_END_NAMESPACE
