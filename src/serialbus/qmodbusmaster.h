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

#ifndef QMODBUSMASTER_H
#define QMODBUSMASTER_H

#include <QtSerialBus/qserialbusglobal.h>
#include <QtSerialBus/qmodbusdevice.h>
#include <QtSerialBus/qmodbusdataunit.h>
#include <QtSerialBus/qmodbusreply.h>

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class Q_SERIALBUS_EXPORT QModBusMaster : public QModBusDevice
{
    Q_OBJECT
public:

    explicit QModBusMaster(QModBusDevice *parent = 0);

    virtual bool setADU(QModBusDevice::ApplicationDataUnit adu) = 0;

    virtual QModBusReply *write(const QModBusDataUnit &request) = 0;
    virtual QModBusReply *write(const QList<QModBusDataUnit> &requests) = 0;
    virtual QModBusReply *read(QModBusDataUnit &request, int slaveId = 1) = 0;
    virtual QModBusReply *read(QList<QModBusDataUnit> &requests, int slaveId = 1) = 0;


protected:

    virtual bool open() = 0;
    virtual void close() = 0;
};

QT_END_NAMESPACE
#endif // QMODBUSMASTER_H
