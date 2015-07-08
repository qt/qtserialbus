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

#include "qserialbusplugininterface.h"
#include "qserialbus.h"
#include "qserialbusdevice.h"

#include <QtCore/qobject.h>
#include <QtCore/qpluginloader.h>
#include <QtCore/qdebug.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qglobalstatic.h>

#include <private/qfactoryloader_p.h>
#include <private/qlibrary_p.h>

#define QSerialBusPluginInterface_iid "org.qt-project.Qt.QSerialBusPluginInterface"

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, qFactoryLoader,
    (QSerialBusPluginInterface_iid, QLatin1String("/serialbuses")))

typedef QHash<QByteArray, QSerialBusBackendFactory*> QSerialBusPluginsHash;
Q_GLOBAL_STATIC(QSerialBusPluginsHash, qSerialBusPlugins)

static QSerialBus *globalInstance = Q_NULLPTR;

static void loadPlugins()
{
    const QList<QJsonObject> meta = qFactoryLoader()->metaData();
    for (int i = 0; i < meta.count(); i++) {
        if (QSerialBusPluginInterface *plugin = qobject_cast<QSerialBusPluginInterface*>(qFactoryLoader->instance(i)))
            plugin->registerBus();
    }
}

/*!
    \class QSerialBus
    \inmodule QtSerialBus

    \brief The QSerialBus class handles registration and creation of bus backends.

    Bus plugins register backends using the registerBackend() function.
*/

/*!
 * \brief Returns pointer to the QSerialBus Class. The object is loaded if necessary.
 */
QSerialBus *QSerialBus::instance()
{
    if (!globalInstance)
        globalInstance = new QSerialBus();
    return globalInstance;
}

/*!
 *   Register backend for the \a identifier. The \a identifier must be unique.
 *   The \a factory will be asked to create instances of the backend.
 */
void QSerialBus::registerBackend(const QByteArray &identifier, QSerialBusBackendFactory *factory)
{
    if (!qSerialBusPlugins()->contains(identifier))
        qSerialBusPlugins()->insert(identifier, factory);
}

/*!
 * Returns QList of identifiers for all loaded plugins.
 */
QList<QByteArray> QSerialBus::plugins()
{
    return qSerialBusPlugins()->keys();
}

/*!
 * Create a bus backend for \a identifier with \a type with \a name
 * Returns \c null if no suitable \a identifier can be found.
 */
QSerialBusBackend *QSerialBus::createBackend(const QByteArray &identifier,
                                             const QString &type, const QString &name) const
{
    if (QSerialBusBackendFactory *factory = qSerialBusPlugins()->value(identifier))
        return factory->createBackend(type, name);
    return Q_NULLPTR;
}

/*!
 * Returns a list of available backends names for \a identifier; otherwise
 * returns an empty list if no suitable \a identifier can be found.
 */
QStringList QSerialBus::availableBackends(const QByteArray &identifier) const
{
    if (QSerialBusBackendFactory *factory = qSerialBusPlugins()->value(identifier))
        return factory->availableBackends();
    return QStringList();
}

QSerialBus::QSerialBus(QObject *parent) :
    QObject(parent)
{
    loadPlugins();
}


QSerialBusBackendFactory::~QSerialBusBackendFactory()
{
}

QT_END_NAMESPACE
