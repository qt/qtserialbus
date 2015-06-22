/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtBus module of the Qt Toolkit.
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

#ifndef QSERIALBUS_H
#define QSERIALBUS_H

#include <QtSerialBus/qserialbusglobal.h>

#include <QtCore/qobject.h>
#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QSerialBusPrivate;
class QSerialBusBackend;
class QSerialBusBackendFactory;
class QFactoryLoader;

class Q_SERIALBUS_EXPORT QSerialBus : public QObject
{
    Q_OBJECT

public:
    static QSerialBus *instance();

    QList<QByteArray> plugins();
    static void registerBackend(const QByteArray &identifier, QSerialBusBackendFactory *factory);
    QSerialBusBackend *createBackend(const QByteArray &identifier,
                                     const QString &type, const QString &name) const;

private:
    QSerialBus(QObject *parent = 0);

    static QSerialBus *serialBus;

    Q_DECLARE_PRIVATE(QSerialBus)
    Q_DISABLE_COPY(QSerialBus)
};


class Q_SERIALBUS_EXPORT QSerialBusBackendFactory
{
public:
    virtual QSerialBusBackend *createBackend(const QString &busBackend, const QString &name) const = 0;
protected:
    virtual ~QSerialBusBackendFactory();
};


QT_END_NAMESPACE

#endif // QSERIALBUS_H
