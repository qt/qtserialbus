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

#include "dummybackend.h"
#include <QtCore/qdebug.h>
#include <QtCore/qtimer.h>

DummyBackend::DummyBackend() :
    sendTimer(new QTimer(this)),
    byteArray("abc")
{
    sendTimer->setInterval(1000);
    sendTimer->setSingleShot(false);
    connect(sendTimer, SIGNAL(timeout()), this, SLOT(sendMessage()));
    sendTimer->start();
}

void DummyBackend::sendMessage()
{
    byteArray.append("def");
    emit readyRead();
}

QByteArray DummyBackend::read(qint64 maxSize)
{
    const qint64 len = byteArray.size();
    if (len > maxSize) {
        QByteArray data = byteArray.left(maxSize);
        byteArray = byteArray.mid(maxSize);
        return data;
    } else {
        const QByteArray data = byteArray;
        byteArray.clear();
        return data;
    }
}

void DummyBackend::writeToBus(const QByteArray &data) {
    qDebug() << "DummyBackend wrote: " << data.data();
}

void DummyBackend::setDataStreamVersion(int version)
{
    Q_UNUSED(version);
}

int DummyBackend::dataStreamVersion() const
{
    return 0;
}

qint64 DummyBackend::size() const
{
    return byteArray.size();
}

QByteArray DummyBackend::readAll()
{
    QByteArray temp = byteArray;
    byteArray.clear();
    return temp;
}

void DummyBackend::setConfiguration(const QPair<QString, QVariant> &conf)
{
    Q_UNUSED(conf);
}
