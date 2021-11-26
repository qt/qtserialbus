/****************************************************************************
**
** Copyright (C) 2018 Andre Hartmann <aha_1980@gmx.de>
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

#ifndef VIRTUALCANBACKEND_H
#define VIRTUALCANBACKEND_H

#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusdeviceinfo.h>
#include <QtSerialBus/qcanbusframe.h>

#include <QtCore/qlist.h>
#include <QtCore/qurl.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QTcpServer;
class QTcpSocket;

class VirtualCanServer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(VirtualCanServer)

public:
    explicit VirtualCanServer(QObject *parent = nullptr);
    ~VirtualCanServer() override;

    void start(quint16 port);

private:
    void connected();
    void disconnected();
    void readyRead();

    QTcpServer *m_server = nullptr;
    QList<QTcpSocket *> m_serverSockets;
};

class VirtualCanBackend : public QCanBusDevice
{
    Q_OBJECT
    Q_DISABLE_COPY(VirtualCanBackend)

public:
    explicit VirtualCanBackend(const QString &interface, QObject *parent = nullptr);
    ~VirtualCanBackend() override;

    bool open() override;
    void close() override;

    void setConfigurationParameter(ConfigurationKey key, const QVariant &value) override;

    bool writeFrame(const QCanBusFrame &frame) override;

    QString interpretErrorFrame(const QCanBusFrame &errorFrame) override;

    static QList<QCanBusDeviceInfo> interfaces();

    QCanBusDeviceInfo deviceInfo() const override;

private:
    static QCanBusDeviceInfo virtualCanDeviceInfo(uint channel);

    void clientConnected();
    void clientDisconnected();
    void clientReadyRead();

    QUrl m_url;
    uint m_channel = 0;
    QTcpSocket *m_clientSocket = nullptr;
};

QT_END_NAMESPACE

#endif // VIRTUALCANBACKEND_H
