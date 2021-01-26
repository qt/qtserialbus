/****************************************************************************
**
** Copyright (C) 2019 Andre Hartmann <aha_1980@gmx.de>
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

#ifndef LIBSOCKETCAN_H
#define LIBSOCKETCAN_H

#include <QtCore/qglobal.h>
#include <QtSerialBus/qcanbusdevice.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class QString;

class LibSocketCan final
{
public:
    explicit LibSocketCan(QString *errorString = nullptr);

    bool start(const QString &interface);
    bool stop(const QString &interface);
    bool restart(const QString &interface);

    quint32 bitrate(const QString &interface) const;
    bool setBitrate(const QString &interface, quint32 bitrate);

    bool hasBusStatus() const;
    QCanBusDevice::CanBusStatus busStatus(const QString &interface) const;
};

QT_END_NAMESPACE

#endif // LIBSOCKETCAN_H
