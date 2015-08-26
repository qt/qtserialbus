/****************************************************************************
**
** Copyright (C) 2015 Denis Shienkov <denis.shienkov@gmail.com>
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

#include "peakcanbackend.h"
#include "peakcanbackend_p.h"

#include <QtSerialBus/qcanbusdevice.h>

QT_BEGIN_NAMESPACE

PeakCanBackend::PeakCanBackend(const QString &name, QObject *parent)
    : QCanBusDevice(parent)
    , d_ptr(new PeakCanBackendPrivate(this))
{
    Q_D(PeakCanBackend);

    d->setupChannel(name);
    d->setupDefaultConfigurations();
}

PeakCanBackend::~PeakCanBackend()
{
    close();
    delete d_ptr;
}

bool PeakCanBackend::open()
{
    Q_D(PeakCanBackend);

    if (!d->isOpen) {
        if (!d->open()) {
            close(); // sets UnconnectedState
            return false;
        }
    }

    setState(QCanBusDevice::ConnectedState);
    return true;
}

void PeakCanBackend::close()
{
    Q_D(PeakCanBackend);

    d->close();

    setState(QCanBusDevice::UnconnectedState);
}

void PeakCanBackend::setConfigurationParameter(int key, const QVariant &value)
{
    Q_D(PeakCanBackend);

    if (d->setConfigurationParameter(key, value))
        QCanBusDevice::setConfigurationParameter(key, value);
}

bool PeakCanBackend::writeFrame(const QCanBusFrame &newData)
{
    Q_D(PeakCanBackend);

    if (state() != QCanBusDevice::ConnectedState)
        return false;

    if (newData.frameType() != QCanBusFrame::DataFrame
            && newData.frameType() != QCanBusFrame::RemoteRequestFrame) {
        setError(tr("Unable to write a frame with unacceptable type"),
                 QCanBusDevice::WriteError);
        return false;
    }

    if (newData.payload().size() > 8) {
        setError(tr("Unable to write a frame with unacceptable payload size"),
                 QCanBusDevice::WriteError);
        return false;
    }

    d->outgoingFrames.append(newData);

#if defined(Q_OS_WIN32)
    d->enableWriteNotification(true);
#endif

    return true;
}

// TODO: Implement me
QString PeakCanBackend::interpretErrorFrame(const QCanBusFrame &errorFrame)
{
    Q_UNUSED(errorFrame);

    return QString();
}

QT_END_NAMESPACE
