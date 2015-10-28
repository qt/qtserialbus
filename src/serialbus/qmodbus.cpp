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

#include "qmodbus.h"
#include "qmodbusfactory.h"

#include <QtCore/qpluginloader.h>
#include <QtCore/qglobalstatic.h>

#include <private/qfactoryloader_p.h>
#include <private/qlibrary_p.h>

#define QModbusFactory_iid "org.qt-project.Qt.QModbusFactory"

QT_BEGIN_NAMESPACE

struct QModbusPrivate
{
public:
    QModbusPrivate() : factory(Q_NULLPTR), index(-1) { }

    QJsonObject meta;
    QModbusFactory *factory;
    int index;
};

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, qFactoryLoader,
    (QModbusFactory_iid, QLatin1String("/modbus")))

typedef QHash<QByteArray, QModbusPrivate> QModbusPluginStore;
Q_GLOBAL_STATIC(QModbusPluginStore, qModBusPlugins)

static QModbus *globalInstance = Q_NULLPTR;

static void loadPlugins()
{
    const QList<QJsonObject> meta = qFactoryLoader()->metaData();
    for (int i = 0; i < meta.count(); i++) {
        const QJsonObject obj = meta.at(i).value(QStringLiteral("MetaData")).toObject();
        if (obj.isEmpty())
            continue;

        QModbusPrivate d;
        d.index = i;
        d.meta = obj;
        qModBusPlugins()->insert(
                    obj.value(QStringLiteral("Key")).toString().toLatin1(), d);
    }
}

/*!
    \class QModbus
    \inmodule QtSerialBus
    \since 5.6

    \brief The QModbus class handles registration and creation of Modbus backends.

    QModbus loads Modbus plugins at runtime. The ownership of serial bus backends is
    transferred to the loader.
*/

/*!
    Returns a pointer to the QModbus class. The object is loaded if necessary. QModbus
    uses the singleton design pattern.
 */
QModbus *QModbus::instance()
{
    if (!globalInstance)
        globalInstance = new QModbus();
    return globalInstance;
}

/*!
    Returns a list of identifiers for all loaded plugins.
 */
QList<QByteArray> QModbus::plugins() const
{
    return qModBusPlugins()->keys();
}

QModbusPrivate setFactory(const QByteArray &plugin)
{
    QModbusPrivate d = qModBusPlugins()->value(plugin);
    if (!d.factory) {
        d.factory
            = qobject_cast<QModbusFactory *>(qFactoryLoader->instance(d.index));
        if (!d.factory)
            return d;

        qModBusPlugins()->insert(plugin, d);
    }
    return d;
}

/*!
    Creates a Modbus server. \a plugin is the name of the plugin as returned by the \l plugins()
    method.

    Ownership of the returned backend is transferred to the caller.
    Returns \c null if no suitable device can be found.
 */
QModbusServer *QModbus::createServer(const QByteArray &plugin,
                                   QModbusDevice::ModbusConnection type) const
{
    if (!qModBusPlugins()->contains(plugin))
        return Q_NULLPTR;
    QModbusPrivate d = setFactory(plugin);
    if (!d.factory)
        return Q_NULLPTR;
    return d.factory->createServer(type);
}

/*!
    Creates a Modbus client. \a plugin is the name of the plugin as returned by the \l plugins()
    method.

    Ownership of the returned backend is transferred to the caller.
    Returns \c null if no suitable device can be found.
 */
QModbusClient *QModbus::createClient(const QByteArray &plugin,
                                     QModbusDevice::ModbusConnection type) const
{
    if (!qModBusPlugins()->contains(plugin))
        return Q_NULLPTR;
    QModbusPrivate d = setFactory(plugin);
    if (!d.factory)
        return Q_NULLPTR;
    return d.factory->createClient(type);
}

QModbus::QModbus(QObject *parent) :
    QObject(parent)
{
    loadPlugins();
}

QT_END_NAMESPACE
