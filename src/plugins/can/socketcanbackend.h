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

#ifndef SOCKETCANBACKEND_H
#define SOCKETCANBACKEND_H

#include <qserialbusbackend.h>

#include <QtCore/qpointer.h>
#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>

class QSocketNotifier;

struct canfd_frame;

class SocketCanBackend : public QSerialBusBackend
{
    Q_OBJECT
public:
    explicit SocketCanBackend(const QString &name);
    qint64 read(char *buffer, qint64 maxSize);
    qint64 write(const char *buffer, qint64 size);
    void setDataStreamVersion(int version);
    void setConfigurationParameter(const QString &key, const QVariant &value);
    QVariant configurationParameter(const QString &key) const;
    QVector<QString> configurationKeys() const;
    int dataStreamVersion() const;
    qint64 bytesAvailable() const;

private:
    qint64 connectSocket();
    void insertInConfigurations(const QString &key, const QVariant &value);
    QByteArray serialize(const canfd_frame &frame, const timeval &time);
    canfd_frame deserialize(const QByteArray &frame);
    canfd_frame readFrame();

    qint64 canSocket;
    QPointer<QSocketNotifier> notifier;
    QByteArray frameArray;
    QString canSocketName;
    int version;
    QVector<QPair<QString, QVariant>> configuration;
};

#endif // SOCKETCANBACKEND_H
