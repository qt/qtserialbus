/****************************************************************************
**
** Copyright (C) 2017 Andre Hartmann <aha_1980@gmx.de>
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

#ifndef QCANBUSDEVICEINFO_H
#define QCANBUSDEVICEINFO_H

#include <QtCore/qshareddata.h>
#include <QtCore/qstring.h>
#include <QtSerialBus/qtserialbusglobal.h>

QT_BEGIN_NAMESPACE

class QCanBusDeviceInfoPrivate;

class Q_SERIALBUS_EXPORT QCanBusDeviceInfo
{
public:
    QCanBusDeviceInfo() = delete;
    QCanBusDeviceInfo(const QCanBusDeviceInfo &other);
    ~QCanBusDeviceInfo();

    void swap(QCanBusDeviceInfo &other) noexcept
    {
         d_ptr.swap(other.d_ptr);
    }

    QCanBusDeviceInfo &operator=(const QCanBusDeviceInfo &other);
    QCanBusDeviceInfo &operator=(QCanBusDeviceInfo &&other) noexcept
    {
        swap(other);
        return *this;
    }

    QString plugin() const;
    QString name() const;
    QString description() const;
    QString serialNumber() const;
    QString alias() const;
    int channel() const;

    bool hasFlexibleDataRate() const;
    bool isVirtual() const;

private:
    friend class QCanBusDevice;

    explicit QCanBusDeviceInfo(QCanBusDeviceInfoPrivate &dd);

    QSharedDataPointer<QCanBusDeviceInfoPrivate> d_ptr;
};

Q_DECLARE_SHARED(QCanBusDeviceInfo)

QT_END_NAMESPACE

#endif // QCANBUSDEVICEINFO_H
