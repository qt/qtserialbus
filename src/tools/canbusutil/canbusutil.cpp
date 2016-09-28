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

#include "canbusutil.h"

CanBusUtil::CanBusUtil(QTextStream &output, QCoreApplication &app, QObject *parent)
  : QObject(parent),
    m_canBus(QCanBus::instance()),
    m_output(output),
    m_app(app),
    m_readTask(new ReadTask(output, this))
{
}

bool CanBusUtil::start(const QString &pluginName, const QString &deviceName, const QString &data)
{
    if (!m_canBus) {
        m_output << "Unable to create QCanBus" << endl;
        return false;
    }

    m_pluginName = pluginName;
    m_deviceName = deviceName;
    m_data = data;
    m_listening = data.isEmpty();

    if (!connectCanDevice())
        return false;

    if (m_listening) {
        connect(m_canDevice.data(), &QCanBusDevice::framesReceived, m_readTask, &ReadTask::checkMessages);
    } else {
        if (!sendData())
            return false;
        QTimer::singleShot(0, &m_app, SLOT(quit()));
    }

    return true;
}

void CanBusUtil::printPlugins()
{
    const QStringList plugins = m_canBus->plugins();
    m_output << "Plugins: " << endl;
    for (int i = 0; i < plugins.size(); i++)
        m_output << plugins.at(i) << endl;
}

bool CanBusUtil::parseDataField(qint32 &id, QString &payload)
{
    int hashMarkPos = m_data.indexOf('#');
    if (hashMarkPos < 0) {
        m_output << "Data field invalid: No hash mark found!" << endl;
        return false;
    }

    id = m_data.left(hashMarkPos).toInt(nullptr, 16);
    payload = m_data.right(m_data.length() - hashMarkPos - 1);

    return true;
}

bool CanBusUtil::parsePayloadField(QString payload, bool &rtrFrame,
                                   bool &fdFrame, QByteArray &bytes)
{
    fdFrame = false;
    rtrFrame = false;

    if (!payload.isEmpty() && payload.at(0).toUpper() == 'R') {
        rtrFrame = true;
        bool validPayloadLength = false;
        if (payload.size() > 1) {
            payload = payload.mid(1);
            int rtrFrameLength = payload.toInt(&validPayloadLength);

            if (validPayloadLength && rtrFrameLength > 0 && rtrFrameLength <= 64) {
                bytes = QByteArray(rtrFrameLength, 0);
                if (rtrFrameLength > 8)
                    fdFrame = true;
            } else if (validPayloadLength) {
                m_output << "The length must be larger than 0 and not exceed 64." << endl;
                validPayloadLength = false;
            }
        }

        if (!validPayloadLength) {
            m_output << "Data field invalid: RTR data frame length not specified/valid." << endl;
        }

        return validPayloadLength;
    } else if (!payload.isEmpty() && payload.at(0) == '#') {
        fdFrame = true;
        payload = payload.mid(1);
    }

    if (payload.size() % 2 != 0) {
        m_output << "Data field invalid: Size is not multiple of two." << endl;
        return false;
    }

    const QRegularExpression re(QStringLiteral("^[0-9A-Fa-f]*$"));
    if (!re.match(payload).hasMatch()) {
        m_output << "Data field invalid: Only hex numbers allowed." << endl;
        return false;
    }

    bytes = QByteArray::fromHex(payload.toLatin1());

    const int maxSize = fdFrame ? 64 : 8;
    if (bytes.size() > maxSize) {
        m_output << "Warning: Truncating payload at max. size of " << maxSize << " bytes." << endl;
        bytes.truncate(maxSize);
    }

    return true;
}

bool CanBusUtil::connectCanDevice()
{
    if (!m_canBus->plugins().contains(m_pluginName)) {
        m_output << "Could not find suitable plugin." << endl;
        printPlugins();
        return false;
    }

    m_canDevice.reset(m_canBus->createDevice(m_pluginName, m_deviceName));
    if (!m_canDevice) {
        m_output << "Unable to create QCanBusDevice with device name: " << m_deviceName << endl;
        return false;
    }
    connect(m_canDevice.data(), &QCanBusDevice::errorOccurred, m_readTask, &ReadTask::receiveError);
    if (!m_canDevice->connectDevice()) {
        m_output << "Unable to connect QCanBusDevice with device name: " << m_deviceName << endl;
        return false;
    }

    return true;
}

bool CanBusUtil::sendData()
{
    qint32 id;
    QString payload;
    bool rtrFrame;
    bool fdFrame;
    QByteArray bytes;
    QCanBusFrame frame;

    if (parseDataField(id, payload) == false)
        return false;

    if (parsePayloadField(payload, rtrFrame, fdFrame, bytes) == false)
        return false;

    if (id < 0 || id > 0x1FFFFFFF) { // 29 bits
        id = 0x1FFFFFFF;
        m_output << "Warning! Id does not fit into Extended Frame Format, setting id to: " << id << endl;
    }

    if (rtrFrame)
        frame.setFrameType(QCanBusFrame::RemoteRequestFrame);

    frame.setPayload(bytes);
    frame.setFrameId(id);

    if (fdFrame)
        m_canDevice->setConfigurationParameter(QCanBusDevice::CanFdKey, true);

    return m_canDevice->writeFrame(frame);
}
