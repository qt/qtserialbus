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

#include "dummybackend.h"

#include <QtCore/qdebug.h>
#include <QtCore/qtimer.h>

#include <QtSerialBus/qcanbusdevice.h>

QT_BEGIN_NAMESPACE

DummyBackend::DummyBackend() :
    sendTimer(new QTimer(this)),
    byteArray("abc")
{
    sendTimer->setInterval(1000);
    sendTimer->setSingleShot(false);
    connect(sendTimer, SIGNAL(timeout()), this, SLOT(sendMessage()));
    sendTimer->start();
}

bool DummyBackend::open()
{
    setState(QCanBusDevice::ConnectedState);
    return true;
}

void DummyBackend::close()
{
    setState(QCanBusDevice::UnconnectedState);
}

void DummyBackend::sendMessage()
{
    byteArray.append("def");

    emit frameReceived();
}

#define MAX_PACKAGE_SIZE 16

qint64 DummyBackend::availableFrames() const
{
    // assume every frame can take max payload of 16 byte
    uint frameCount = byteArray.size() / MAX_PACKAGE_SIZE;
    if ((byteArray.size() % MAX_PACKAGE_SIZE) > 0)
        frameCount++;

    return frameCount;
}

QCanFrame DummyBackend::readFrame()
{
    QCanFrame dummyFrame;

    if (byteArray.isEmpty())
        return dummyFrame;

    dummyFrame.setFrameId(12);

    const qint64 len = byteArray.size();
    if (len > MAX_PACKAGE_SIZE) {
        const QByteArray data = byteArray.left(MAX_PACKAGE_SIZE);
        byteArray = byteArray.mid(MAX_PACKAGE_SIZE);
        dummyFrame.setPayload(data);
    } else {
        dummyFrame.setPayload(byteArray);
        byteArray.clear();
    }

    return dummyFrame;
}

bool DummyBackend::writeFrame(const QCanFrame &data)
{
    qDebug() << "DummyBackend::writeFrame: " << data.payload();
    return true;
}

void DummyBackend::setConfigurationParameter(const QString &key, const QVariant &value)
{
    Q_UNUSED(key);
    Q_UNUSED(value);
}

QVariant DummyBackend::configurationParameter(const QString&) const
{
    return QVariant();
}

QVector<QString> DummyBackend::configurationKeys() const
{
    return QVector<QString>();
}

QT_END_NAMESPACE
