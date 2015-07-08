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

#ifndef QSERIALBUSBACKEND_H
#define QSERIALBUSBACKEND_H

#include <QtSerialBus/qserialbusglobal.h>

#include <QtCore/qobject.h>
#include <QtCore/qiodevice.h>

QT_BEGIN_NAMESPACE

class Q_SERIALBUS_EXPORT QSerialBusBackend : public QObject
{
    Q_OBJECT
public:
    virtual bool open(QIODevice::OpenMode openMode) = 0;
    virtual void close() = 0;
    virtual qint64 read(char *buffer, qint64 maxSize) = 0;
    virtual void setConfigurationParameter(const QString &key, const QVariant &value) = 0;
    virtual QVariant configurationParameter(const QString &key) const = 0;
    virtual QVector<QString> configurationKeys() const = 0;
    virtual qint64 write(const char* buffer, qint64 len) = 0;
    virtual qint64 bytesAvailable() const = 0;

Q_SIGNALS:
    void readyRead();
    void error(QString, int);
};

QT_END_NAMESPACE

#endif // QSERIALBUSBACKEND_H
