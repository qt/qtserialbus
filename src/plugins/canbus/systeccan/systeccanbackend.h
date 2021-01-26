/****************************************************************************
**
** Copyright (C) 2017 Andre Hartmann <aha_1980@gmx.de>
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
****************************************************************************/

#ifndef SYSTECCANBACKEND_H
#define SYSTECCANBACKEND_H

#include <QtSerialBus/qcanbusframe.h>
#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusdeviceinfo.h>

#include <QtCore/qvariant.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class SystecCanBackendPrivate;

class SystecCanBackend : public QCanBusDevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SystecCanBackend)
    Q_DISABLE_COPY(SystecCanBackend)
public:
    explicit SystecCanBackend(const QString &name, QObject *parent = nullptr);
    ~SystecCanBackend();

    bool open() override;
    void close() override;

    void setConfigurationParameter(int key, const QVariant &value) override;

    bool writeFrame(const QCanBusFrame &newData) override;

    QString interpretErrorFrame(const QCanBusFrame &errorFrame) override;

    static QList<QCanBusDeviceInfo> interfaces();
    static bool canCreate(QString *errorReason);

    // This function needs to be public as it is accessed by a callback
    static QCanBusDeviceInfo createDeviceInfo(const QString &serialNumber,
                                              const QString &description,
                                              uint deviceNumber,
                                              int channelNumber);

private:
    void resetController();
    QCanBusDevice::CanBusStatus busStatus();

    SystecCanBackendPrivate * const d_ptr;
};

QT_END_NAMESPACE

#endif // SYSTECCANBACKEND_H
