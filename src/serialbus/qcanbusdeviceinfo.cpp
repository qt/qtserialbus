/****************************************************************************
**
** Copyright (C) 2017 Andre Hartmann <aha_1980@gmx.de>
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

#include "qcanbusdeviceinfo.h"
#include "qcanbusdeviceinfo_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCanBusDeviceInfo
    \inmodule QtSerialBus
    \since 5.9

    \brief The QCanBusDeviceInfo provides information about CAN interfaces.

    Each plugin may support one or more interfaces with different
    capabilities. This class provides information about available functions.
*/

/*!
    Constructs a new empty QCanBusDeviceInfo.
*/
QCanBusDeviceInfo::QCanBusDeviceInfo() :
    d_ptr(new QCanBusDeviceInfoPrivate)
{
}

/*!
    Constructs a new QCanBusDeviceInfo from \a other.
*/
QCanBusDeviceInfo::QCanBusDeviceInfo(const QCanBusDeviceInfo &other) :
    d_ptr(other.d_ptr)
{
}

/*!
    Constructs a new QCanBusDeviceInfo from the QCanBusDeviceInfoPrivate \a dd.
    \internal
*/
QCanBusDeviceInfo::QCanBusDeviceInfo(QCanBusDeviceInfoPrivate &dd) :
    d_ptr(new QCanBusDeviceInfoPrivate(dd))
{
}

/*!
    Destroys the QCanBusDeviceInfo object. References to the values in the
    object become invalid.
*/
QCanBusDeviceInfo::~QCanBusDeviceInfo()
{
}

/*!
    Swap this instance's shared data pointer with the shared data pointer in
    \a other.
    \internal
*/
void QCanBusDeviceInfo::swap(QCanBusDeviceInfo other)
{
    d_ptr.swap(other.d_ptr);
}

/*!
    Sets the QCanBusDeviceInfo object to be equal to \a other.
*/
QCanBusDeviceInfo &QCanBusDeviceInfo::operator=(const QCanBusDeviceInfo &other)
{
    QCanBusDeviceInfo(other).swap(*this);
    return *this;
}

/*!
    Returns the interface name of this CAN bus interface, e.g. can0.
*/
QString QCanBusDeviceInfo::name() const
{
    return d_ptr->name;
}

/*!
    Returns true, if the CAN bus interface is CAN FD (flexible data rate) capable.

    If this information is not available, false is returned.
*/
bool QCanBusDeviceInfo::hasFlexibleDataRate() const
{
    return d_ptr->hasFlexibleDataRate;
}

/*!
    Returns true, if the CAN bus interface is virtual (i.e. not connected to real
    CAN hardware).

    If this information is not available, false is returned.
*/
bool QCanBusDeviceInfo::isVirtual() const
{
    return d_ptr->isVirtual;
}

QT_END_NAMESPACE
