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

#ifndef CANBUSUTIL_H
#define CANBUSUTIL_H

#include "readtask.h"

#include <QObject>

QT_BEGIN_NAMESPACE

class QCanBusFrame;
class QCoreApplication;
class QTextStream;

QT_END_NAMESPACE

class CanBusUtil : public QObject
{
    Q_OBJECT
public:
    explicit CanBusUtil(QTextStream &output, QCoreApplication &app, QObject *parent = nullptr);

    void setShowTimeStamp(bool showTimeStamp);
    void setShowFlags(bool showFlags);
    void setConfigurationParameter(QCanBusDevice::ConfigurationKey key, const QVariant &value);
    bool start(const QString &pluginName, const QString &deviceName, const QString &data = QString());
    int  printPlugins();
    int  printDevices(const QString &pluginName);

private:
    bool parseDataField(QCanBusFrame::FrameId &id, QString &payload);
    bool setFrameFromPayload(QString payload, QCanBusFrame *frame);
    bool connectCanDevice();
    bool sendData();

private:
    QCanBus *m_canBus = nullptr;
    QTextStream &m_output;
    QCoreApplication &m_app;
    bool m_listening = false;
    QString m_pluginName;
    QString m_deviceName;
    QString m_data;
    std::unique_ptr<QCanBusDevice> m_canDevice;
    ReadTask *m_readTask = nullptr;
    using ConfigurationParameter = QHash<QCanBusDevice::ConfigurationKey, QVariant>;
    ConfigurationParameter m_configurationParameter;
};

#endif // CANBUSUTIL_H
