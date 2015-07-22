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

#define QModBusFactory_iid "org.qt-project.Qt.QModBusFactory"

QT_BEGIN_NAMESPACE

struct QModBusPrivate
{
public:
    QModBusPrivate() : factory(Q_NULLPTR), index(-1) { }

    QJsonObject meta;
    QModBusFactory *factory;
    int index;
};

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, qFactoryLoader,
    (QModBusFactory_iid, QLatin1String("/modbus")))

typedef QHash<QByteArray, QModBusPrivate> QModBusPluginStore;
Q_GLOBAL_STATIC(QModBusPluginStore, qModBusPlugins)

static QModBus *globalInstance = Q_NULLPTR;

static void loadPlugins()
{
    const QList<QJsonObject> meta = qFactoryLoader()->metaData();
    for (int i = 0; i < meta.count(); i++) {
        const QJsonObject obj = meta.at(i).value(QStringLiteral("MetaData")).toObject();
        if (obj.isEmpty())
            continue;

        QModBusPrivate d;
        d.index = i;
        d.meta = obj;
        qModBusPlugins()->insert(
                    obj.value(QStringLiteral("Key")).toString().toLatin1(), d);
    }
}

QModBus *QModBus::instance()
{
    if (!globalInstance)
        globalInstance = new QModBus();
    return globalInstance;
}

QList<QByteArray> QModBus::plugins() const
{
    return qModBusPlugins()->keys();
}

QModBusDevice *QModBus::createDevice(const QByteArray &plugin,
                            const QString &interfaceName) const
{
    if (!qModBusPlugins()->contains(plugin))
        return Q_NULLPTR;

    QModBusPrivate d = qModBusPlugins()->value(plugin);
    if (!d.factory) {
        d.factory
            = qobject_cast<QModBusFactory *>(qFactoryLoader->instance(d.index));
        if (!d.factory)
            return Q_NULLPTR;

        qModBusPlugins()->insert(plugin, d);
    }
    if (!d.factory)
        return Q_NULLPTR;

    return d.factory->createDevice(interfaceName);}

QModBus::QModBus(QObject *parent) :
    QObject(parent)
{
    loadPlugins();
}

QT_END_NAMESPACE
