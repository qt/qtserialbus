/****************************************************************************
**
** Copyright (C) 2017 Ford Motor Company.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef PASSTHRUCAN_PASSTHRUCANIO_H
#define PASSTHRUCAN_PASSTHRUCANIO_H

#include "j2534passthru.h"

#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusframe.h>
#include <QByteArray>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariant>

QT_BEGIN_NAMESPACE

class PassThruCanIO : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PassThruCanIO)
public:
    static const uint pollTimeout = 100; // ms

    explicit PassThruCanIO(QObject *parent = nullptr);
    virtual ~PassThruCanIO();

    Q_INVOKABLE void open(const QString &library, const QByteArray &subDev, uint bitRate);
    Q_INVOKABLE void close();
    Q_INVOKABLE void applyConfig(QCanBusDevice::ConfigurationKey key, const QVariant &value);
    Q_INVOKABLE void listen();

    // Internally locked; safe to call directly from any thread.
    bool enqueueMessage(const QCanBusFrame &frame);

Q_SIGNALS:
    void errorOccurred(const QString &description, QCanBusDevice::CanBusError error);
    void messagesReceived(QList<QCanBusFrame> frames);
    void messagesSent(qint64 count);
    void openFinished(bool success);
    void closeFinished();

private:
    bool setMessageFilters(const QList<QCanBusDevice::Filter> &filters);
    bool setConfigValue(J2534::Config::Parameter param, ulong value);
    void pollForMessages();
    bool writeMessages();
    void readMessages(bool writePending);

    J2534::PassThru *       m_passThru      = nullptr;
    J2534::PassThru::Handle m_deviceId      = 0;
    J2534::PassThru::Handle m_channelId     = 0;
    QTimer *                m_idleNotifier  = nullptr;
    QList<J2534::Message> m_ioBuffer;
    QMutex                  m_writeGuard;
    QList<QCanBusFrame>     m_writeQueue;
};

QT_END_NAMESPACE

#endif // PASSTHRUCAN_PASSTHRUCANIO_H
