/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the tools applications of the QtSerialBus module.
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

#include "readtask.h"

using namespace std;

ReadTask::ReadTask(QTextStream &output, QObject *parent)
    : QObject(parent),
      output(output) { }

void ReadTask::checkMessages() {
    auto canDevice = qobject_cast<QCanBusDevice *>(QObject::sender());
    if (canDevice == nullptr) {
        qWarning() << "ReadTask::checkMessages: Unknown sender";
        return;
    }

    const QCanBusFrame frame = canDevice->readFrame();

    const qint32 id = frame.frameId();
    const qint8 dataLength = frame.payload().size();

    QString view;
    if (frame.frameType() == QCanBusFrame::ErrorFrame) {
        view = canDevice->interpretErrorFrame(frame);
    } else {
        view += QLatin1String("Id: ");
        view += QString::number(id, 16);
        view += QLatin1String(" bytes: ");
        view += QString::number(dataLength, 10);
        view += QLatin1String(" data:");
        QByteArray array = frame.payload();
        for (int i=0; i < array.size(); i++) {
            view += QLatin1String(" 0x");
            quint8 number = array[i];
            view += QString::number(number, 16);
        }
    }

    if (frame.frameType() == QCanBusFrame::RemoteRequestFrame) {
        output << "RTR: " << view << endl;
    } else if (frame.frameType() == QCanBusFrame::ErrorFrame) {
        output << "ERR: " << view << endl;
    } else {
        output << view << endl;
    }
}

void ReadTask::receiveError(QCanBusDevice::CanBusError /*error*/) {
    auto canDevice = qobject_cast<QCanBusDevice *>(QObject::sender());
    if (canDevice == nullptr) {
        qWarning() << "ReadTask::receiveError: Unknown sender";
        return;
    }

    output << "Read error: " << canDevice->errorString() << endl;
}
