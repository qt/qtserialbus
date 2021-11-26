/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qcanbusfactory.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCanBusFactory
    \inmodule QtSerialBus
    \since 5.9

    \brief The QCanBusFactory class is a factory class used as the
    plugin interface for CAN bus plugins.

    All plugins must implement the functions provided by this factory class.
*/

/*!
    \fn QCanBusDevice *QCanBusFactory::createDevice(const QString &interfaceName,
                                                    QString *errorMessage) const

    Creates a new QCanBusDevice. The caller must take ownership of the returned pointer.

    \a interfaceName is the CAN interface name and
    \a errorMessage contains an error description in case of failure.

    If the factory cannot create a plugin, it returns \c nullptr.
*/

/*!
    \fn QList<QCanBusDeviceInfo> QCanBusFactory::availableDevices(QString *errorMessage) const

    Returns the list of available devices and their capabilities for the QCanBusDevice.

    \a errorMessage contains an error description in case of failure.
*/

/*!
 * \internal
 */
QCanBusFactory::~QCanBusFactory()
{
}

QT_END_NAMESPACE
