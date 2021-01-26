/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#include "qcanbusfactory.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCanBusFactory
    \inmodule QtSerialBus
    \since 5.8
    \deprecated

    \brief The QCanBusFactory class is a factory class used as the
    plugin interface for CAN bus plugins.

    All plugins must implement the functions provided by this factory class.

    This class is deprecated, you should use QCanBusFactoryV2 instead.

    \sa QCanBusFactoryV2
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
    \internal

    \fn QCanBusFactory::~QCanBusFactory()
*/

/*!
    \class QCanBusFactoryV2
    \inmodule QtSerialBus
    \since 5.9

    \brief The QCanBusFactoryV2 class is a factory class used as the
    plugin interface for CAN bus plugins.

    All plugins must implement the functions provided by this factory class.
*/

/*!
    \fn QCanBusDevice *QCanBusFactoryV2::createDevice(const QString &interfaceName,
                                                    QString *errorMessage) const

    Creates a new QCanBusDevice. The caller must take ownership of the returned pointer.

    \a interfaceName is the CAN interface name and
    \a errorMessage contains an error description in case of failure.

    If the factory cannot create a plugin, it returns \c nullptr.
*/

/*!
    \fn QList<QCanBusDeviceInfo> QCanBusFactoryV2::availableDevices(QString *errorMessage) const

    Returns the list of available devices and their capabilities for the QCanBusDevice.

    \a errorMessage contains an error description in case of failure.
*/

/*!
 * \internal
 */
QCanBusFactoryV2::~QCanBusFactoryV2()
{
}

QT_END_NAMESPACE
