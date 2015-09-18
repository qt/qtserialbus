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

#include "qmodbusserialmaster.h"
#include "qmodbusserialmaster_p.h"

#include <QtSerialBus/qmodbus.h>

QT_BEGIN_NAMESPACE

QModBusSerialMaster::QModBusSerialMaster(QObject *parent)
    : QModBusMaster(*new QModBusSerialMasterPrivate, parent)
{
    Q_D(QModBusSerialMaster);
    d->setupMaster();
}

QModBusSerialMaster::~QModBusSerialMaster()
{
}

bool QModBusSerialMaster::connectDevice(const QString &deviceName)
{
    Q_D(QModBusSerialMaster);

    if (!d->pluginMaster)
        return false;

    // TODO remove setPortName() usage
    d->pluginMaster->setPortName(deviceName);

    // TODO Implement QModBusSerialMaster::connectDevice(QString)

    return d->pluginMaster->connectDevice();
}

QModBusSerialMaster::QModBusSerialMaster(QModBusSerialMasterPrivate &dd,
                                         QObject *parent)
    : QModBusMaster(dd, parent)
{
    Q_D(QModBusSerialMaster);
    d->setupMaster();
}

bool QModBusSerialMaster::open()
{
    // TODO remove later on
    return false;
}

void QModBusSerialMaster::close()
{
    Q_D(QModBusSerialMaster);

    if (!d->pluginMaster)
        return;

    d->pluginMaster->disconnectDevice();
}

// forward the state changes
void QModBusSerialMasterPrivate::handleStateChanged(QModBusDevice::ModBusDeviceState state)
{
    Q_Q(QModBusSerialMaster);
    q->setState(state);
}

// forward the error changes
void QModBusSerialMasterPrivate::handleErrorOccurred(QModBusDevice::ModBusError error)
{
    Q_Q(QModBusSerialMaster);
    q->setError(pluginMaster ? pluginMaster->errorString() : QString(),
                error);
}

QModBusReply *QModBusSerialMaster::write(const QModBusDataUnit &request, int slaveId)
{
    Q_D(QModBusSerialMaster);

    if (!d->pluginMaster)
        return 0;

    return d->pluginMaster->write(request, slaveId);
}

QModBusReply *QModBusSerialMaster::read(const QModBusDataUnit &request, int slaveId)
{
    Q_D(QModBusSerialMaster);

    if (!d->pluginMaster)
        return 0;

    return d->pluginMaster->read(request, slaveId);

}

#include "moc_qmodbusserialmaster.cpp"

QT_END_NAMESPACE
