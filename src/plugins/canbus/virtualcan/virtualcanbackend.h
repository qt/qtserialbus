/****************************************************************************
**
** Copyright (C) 2018 Andre Hartmann <aha_1980@gmx.de>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

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
