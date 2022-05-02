/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

#ifndef QMODBUSDEVICE_P_H
#define QMODBUSDEVICE_P_H

#include <QtCore/qvariant.h>
#include <QtSerialBus/qmodbusdevice.h>
#if QT_CONFIG(modbus_serialport)
#include <QtSerialPort/qserialport.h>
#endif

#include <private/qobject_p.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class QModbusDevicePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QModbusDevice)

public:
    QModbusDevice::State state = QModbusDevice::UnconnectedState;
    QModbusDevice::Error error = QModbusDevice::NoError;
    QString errorString;

#if QT_CONFIG(modbus_serialport)
    QString m_comPort;
    QSerialPort::DataBits m_dataBits = QSerialPort::Data8;
    QSerialPort::Parity m_parity = QSerialPort::EvenParity;
    QSerialPort::StopBits m_stopBits = QSerialPort::OneStop;
    QSerialPort::BaudRate m_baudRate = QSerialPort::Baud19200;

    /*!
        According to the Modbus specification, in RTU mode message frames
        are separated by a silent interval of at least 3.5 character times.
        Calculate the timeout if we are less than 19200 baud, use a fixed
        timeout for everything equal or greater than 19200 baud.
        If the user set the timeout to be longer than the calculated one,
        we'll keep the user defined.
    */
    void calculateInterFrameDelay()
    {
        // The spec recommends a timeout value of 1.750 msec. Without such
        // precise single-shot timers use a approximated value of 1.750 msec.
        int delayMilliSeconds = RecommendedDelay;
        if (m_baudRate < 19200) {
            // Example: 9600 baud, 11 bit per packet -> 872 char/sec so:
            // 1000 ms / 872 char = 1.147 ms/char * 3.5 character = 4.0145 ms
            // Always round up because the spec requests at least 3.5 char.
            delayMilliSeconds = qCeil(3500. / (qreal(m_baudRate) / 11.));
        }
        m_interFrameDelayMilliseconds = qMax(m_interFrameDelayMilliseconds, delayMilliSeconds);
    }
    static constexpr int RecommendedDelay = 2; // A approximated value of 1.750 msec.
    int m_interFrameDelayMilliseconds = RecommendedDelay;
#endif

    int m_networkPort = 502;
    QString m_networkAddress = QStringLiteral("127.0.0.1");

    virtual QIODevice *device() const { return nullptr; }
};

QT_END_NAMESPACE

#endif // QMODBUSDEVICE_P_H
