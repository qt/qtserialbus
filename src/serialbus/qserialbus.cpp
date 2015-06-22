/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
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

QSerialBus *QSerialBus::serialBus = Q_NULLPTR;

class QSerialBusPrivate
{
    friend class QSerialBus;
public:
    QSerialBusPrivate()
        : loader(new QFactoryLoader("org.qt-project.Qt.QSerialBusPluginInterface",
                                    QLatin1String("/serialbuses"))) {}
    void loadPlugins();
    QSerialBusBackend *createBackend(const QByteArray &identifier,
                                     const QString &type, const QString &name) const;

    QHash<QByteArray, QSerialBusBackendFactory*> factoryByIdentifier;

private:
    QPointer<QFactoryLoader> loader;
};

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
    if (!serialBus)
        serialBus = new QSerialBus();

    return serialBus;
}

Q_GLOBAL_STATIC(QSerialBusPrivate, serialBusPrivate)

QSerialBus::QSerialBus(QObject *parent) :
    QObject(parent)
{
    QSerialBusPrivate *d = serialBusPrivate();
    d->loadPlugins();
}

void QSerialBusPrivate::loadPlugins()
{
    const QList<QJsonObject> meta = loader->metaData();
    for (int i = 0; i < meta.count(); i++) {
        QSerialBusPluginInterface *plugin
            = qobject_cast<QSerialBusPluginInterface*>(loader->instance(i));
        if (plugin)
            plugin->registerBus();
    }
}

QSerialBusBackend *QSerialBusPrivate::createBackend(const QByteArray &identifier,
                                                    const QString &type, const QString &name) const
{
    if (!(factoryByIdentifier[identifier]))
        return Q_NULLPTR;

    return factoryByIdentifier[identifier]->createBackend(type, name);
}

/*!
 * Create a bus backend for \a identifier with \a type with \a name
 * Returns \c null if no suitable \a identifier can be found.
 */
QSerialBusBackend *QSerialBus::createBackend(const QByteArray &identifier,
                                             const QString &type, const QString &name) const
{
    QSerialBusPrivate *d = serialBusPrivate();
    return d->createBackend(identifier, type, name);
}

/*!
 *   Register backend for the \a identifier. The \a identifier must be unique.
 *   The \a factory will be asked to create instances of the backend.
 */
void QSerialBus::registerBackend(const QByteArray &identifier, QSerialBusBackendFactory *factory)
{
    QSerialBusPrivate *d = serialBusPrivate();
    if (!d->factoryByIdentifier.contains(identifier))
        d->factoryByIdentifier[identifier] = factory;
}

/*!
 * Returns QList of identifiers for all loaded plugins.
 */
QList<QByteArray> QSerialBus::plugins()
{
    QSerialBusPrivate *d = serialBusPrivate();
    return d->factoryByIdentifier.keys();
}


QSerialBusBackendFactory::~QSerialBusBackendFactory()
{
}

#include "qserialbus.moc"
