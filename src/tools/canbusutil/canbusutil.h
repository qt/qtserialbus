/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the QtSerialBus module.
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

#ifndef CANBUSUTIL_H
#define CANBUSUTIL_H

#include "readtask.h"

#include <QObject>
#include <QScopedPointer>

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
    bool parseDataField(quint32 &id, QString &payload);
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
    QScopedPointer<QCanBusDevice> m_canDevice;
    ReadTask *m_readTask = nullptr;
    using ConfigurationParameter = QHash<QCanBusDevice::ConfigurationKey, QVariant>;
    ConfigurationParameter m_configurationParameter;
};

#endif // CANBUSUTIL_H
