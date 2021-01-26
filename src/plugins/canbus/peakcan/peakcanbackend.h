/****************************************************************************
**
** Copyright (C) 2017 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2021 The Qt Company Ltd.
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

#ifndef PEAKCANBACKEND_H
#define PEAKCANBACKEND_H

#include <QtSerialBus/qcanbusframe.h>
#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusdeviceinfo.h>

#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class PeakCanBackendPrivate;

class PeakCanBackend : public QCanBusDevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(PeakCanBackend)
    Q_DISABLE_COPY(PeakCanBackend)
public:
    explicit PeakCanBackend(const QString &name, QObject *parent = nullptr);
    ~PeakCanBackend();

    bool open() override;
    void close() override;

    void setConfigurationParameter(int key, const QVariant &value) override;

    bool writeFrame(const QCanBusFrame &newData) override;

    QString interpretErrorFrame(const QCanBusFrame &errorFrame) override;

    static bool canCreate(QString *errorReason);
    static QList<QCanBusDeviceInfo> interfaces();

private:
    void resetController();
    CanBusStatus busStatus() const;

    PeakCanBackendPrivate * const d_ptr;
};

QT_END_NAMESPACE

#endif // PEAKCANBACKEND_H
