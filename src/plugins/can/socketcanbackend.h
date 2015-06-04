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

class QSocketNotifier;

struct canfd_frame;

class SocketCanBackend : public QSerialBusBackend
{
    Q_OBJECT
public:
    explicit SocketCanBackend(const QString &name);
    QByteArray readAll();
    QByteArray read(qint64);
    void writeToBus(const QByteArray &data);
    void setDataStreamVersion(int version);
    void setConfiguration(const QPair<QString, QVariant> &conf);
    int dataStreamVersion() const;
    qint64 size() const;

private:
    qint64 connectSocket();
    QByteArray serialize(const canfd_frame &frame, const timeval &time);
    canfd_frame deserialize(const QByteArray &frame);
    canfd_frame readFrame();

    qint64 canSocket;
    QPointer<QSocketNotifier> notifier;
    QByteArray frameArray;
    QString canSocketName;
    int version;
};

#endif // SOCKETCANBACKEND_H
