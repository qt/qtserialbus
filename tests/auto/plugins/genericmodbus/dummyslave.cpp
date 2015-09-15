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

#include "dummyslave.h"

#include <QIODevice>

DummySlave::DummySlave(QObject *parent) :
    QModBusSlave(parent)
{
}

bool DummySlave::setDevice(QIODevice *transport, ApplicationDataUnit ADU)
{
    Q_UNUSED(transport);
    Q_UNUSED(ADU);

    return true;
}

bool DummySlave::open()
{
    return true;
}

void DummySlave::close()
{

}

bool DummySlave::setMap(QModBusRegister::RegisterType table, quint16 size)
{
    Q_UNUSED(table);
    Q_UNUSED(size);
    return true;
}

bool DummySlave::setMap(const QModBusRegister &/*newRegister*/)
{
    return false;
}

void DummySlave::setSlaveId(int id)
{
    Q_UNUSED(id);
}

int DummySlave::slaveId() const
{
    return 1;
}

bool DummySlave::data(QModBusRegister::RegisterType table, quint16 address, quint16 *data)
{
    Q_UNUSED(table);
    Q_UNUSED(address);
    *data = 9;
    return true;
}

bool DummySlave::setData(QModBusRegister::RegisterType table, quint16 address, quint16 data)
{
    Q_UNUSED(table);
    Q_UNUSED(address);
    Q_UNUSED(data);
    return true;
}
