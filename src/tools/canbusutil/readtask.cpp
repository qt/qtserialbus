/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the QtSerialBus module.
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

#include "readtask.h"

ReadTask::ReadTask(QTextStream &output, QObject *parent) :
    QObject(parent),
    m_output(output) { }

void ReadTask::setShowTimeStamp(bool showTimeStamp)
{
    m_showTimeStamp = showTimeStamp;
}

bool ReadTask::isShowFlags() const
{
    return m_showFlags;
}

void ReadTask::setShowFlags(bool showFlags)
{
    m_showFlags = showFlags;
}

void ReadTask::handleFrames() {
    auto canDevice = qobject_cast<QCanBusDevice *>(QObject::sender());
    if (canDevice == nullptr) {
        qWarning("ReadTask::handleFrames: Unknown sender.");
        return;
    }

    while (canDevice->framesAvailable()) {
        const QCanBusFrame frame = canDevice->readFrame();

        QString view;

        if (m_showTimeStamp) {
            view = QStringLiteral("%1.%2  ")
                    .arg(frame.timeStamp().seconds(), 10, 10, QLatin1Char(' '))
                    .arg(frame.timeStamp().microSeconds() / 100, 4, 10, QLatin1Char('0'));
        }

        if (m_showFlags) {
            QLatin1String flags("- - -  ");

            if (frame.hasBitrateSwitch())
                flags[0] = QLatin1Char('B');
            if (frame.hasErrorStateIndicator())
                flags[2] = QLatin1Char('E');
            if (frame.hasLocalEcho())
                flags[4] = QLatin1Char('L');

            view += flags;
        }

        if (frame.frameType() == QCanBusFrame::ErrorFrame)
            view += canDevice->interpretErrorFrame(frame);
        else
            view += frame.toString();

        m_output << view << Qt::endl;
    }
}

void ReadTask::handleError(QCanBusDevice::CanBusError /*error*/)
{
    auto canDevice = qobject_cast<QCanBusDevice *>(QObject::sender());
    if (canDevice == nullptr) {
        qWarning("ReadTask::handleError: Unknown sender.");
        return;
    }

    m_output << tr("Read error: '%1'").arg(canDevice->errorString()) << Qt::endl;
}
