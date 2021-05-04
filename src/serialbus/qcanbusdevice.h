/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Copyright (C) 2021 Andre Hartmann <aha_1980@gmx.de>
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

#ifndef QCANBUSDEVICE_H
#define QCANBUSDEVICE_H

#include <QtCore/qobject.h>
#include <QtSerialBus/qcanbusframe.h>
#include <QtSerialBus/qcanbusdeviceinfo.h>

#include <functional>

QT_BEGIN_NAMESPACE

class QCanBusDevicePrivate;

class Q_SERIALBUS_EXPORT QCanBusDevice : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QCanBusDevice)
    Q_DISABLE_COPY(QCanBusDevice)

public:
    enum CanBusError {
        NoError,
        ReadError,
        WriteError,
        ConnectionError,
        ConfigurationError,
        UnknownError,
        OperationError,
        TimeoutError
    };
    Q_ENUM(CanBusError)

    enum CanBusDeviceState {
        UnconnectedState,
        ConnectingState,
        ConnectedState,
        ClosingState
    };
    Q_ENUM(CanBusDeviceState)

    enum class CanBusStatus {
        Unknown,
        Good,
        Warning,
        Error,
        BusOff
    };
    Q_ENUM(CanBusStatus)

    enum ConfigurationKey {
        RawFilterKey = 0,
        ErrorFilterKey,
        LoopbackKey,
        ReceiveOwnKey,
        BitRateKey,
        CanFdKey,
        DataBitRateKey,
        ProtocolKey,
        UserKey = 30
    };
    Q_ENUM(ConfigurationKey)

    struct Filter
    {
        friend constexpr bool operator==(const Filter &a, const Filter &b) noexcept
        {
            return a.frameId == b.frameId && a.frameIdMask == b.frameIdMask
                    && a.type == b.type && a.format == b.format;
        }

        friend constexpr bool operator!=(const Filter &a, const Filter &b) noexcept
        {
            return !operator==(a, b);
        }

        enum FormatFilter {
            MatchBaseFormat = 0x0001,
            MatchExtendedFormat = 0x0002,
            MatchBaseAndExtendedFormat = 0x0003,
        };
        Q_DECLARE_FLAGS(FormatFilters, FormatFilter)

        QCanBusFrame::FrameId frameId = 0;
        QCanBusFrame::FrameId frameIdMask = 0;
        QCanBusFrame::FrameType type = QCanBusFrame::InvalidFrame;
        FormatFilter format = MatchBaseAndExtendedFormat;
    };

    explicit QCanBusDevice(QObject *parent = nullptr);

    virtual void setConfigurationParameter(ConfigurationKey key, const QVariant &value);
    QVariant configurationParameter(ConfigurationKey key) const;
    QList<ConfigurationKey> configurationKeys() const;

    virtual bool writeFrame(const QCanBusFrame &frame) = 0;
    QCanBusFrame readFrame();
    QList<QCanBusFrame> readAllFrames();
    qint64 framesAvailable() const;
    qint64 framesToWrite() const;

    void resetController();
    bool hasBusStatus() const;
    QCanBusDevice::CanBusStatus busStatus() const;

    enum Direction {
        Input = 1,
        Output = 2,
        AllDirections = Input | Output
    };
    Q_DECLARE_FLAGS(Directions, Direction)
    void clear(Directions direction = Direction::AllDirections);

    virtual bool waitForFramesWritten(int msecs);
    virtual bool waitForFramesReceived(int msecs);

    bool connectDevice();
    void disconnectDevice();

    CanBusDeviceState state() const;

    CanBusError error() const;
    QString errorString() const;

    virtual QString interpretErrorFrame(const QCanBusFrame &errorFrame) = 0;

Q_SIGNALS:
    void errorOccurred(QCanBusDevice::CanBusError);
    void framesReceived();
    void framesWritten(qint64 framesCount);
    void stateChanged(QCanBusDevice::CanBusDeviceState state);

protected:
    void setState(QCanBusDevice::CanBusDeviceState newState);
    void setError(const QString &errorText, QCanBusDevice::CanBusError);
    void clearError();

    void enqueueReceivedFrames(const QList<QCanBusFrame> &newFrames);

    void enqueueOutgoingFrame(const QCanBusFrame &newFrame);
    QCanBusFrame dequeueOutgoingFrame();
    bool hasOutgoingFrames() const;

    virtual bool open() = 0;
    virtual void close() = 0;

    void setResetControllerFunction(std::function<void()> resetter);
    void setCanBusStatusGetter(std::function<CanBusStatus()> busStatusGetter);

    static QCanBusDeviceInfo createDeviceInfo(const QString &name,
                                              bool isVirtual = false,
                                              bool isFlexibleDataRateCapable = false);
    static QCanBusDeviceInfo createDeviceInfo(const QString &name, const QString &serialNumber,
                                              const QString &description, int channel,
                                              bool isVirtual, bool isFlexibleDataRateCapable);
    static QCanBusDeviceInfo createDeviceInfo(const QString &name, const QString &serialNumber,
                                              const QString &description,
                                              const QString &alias, int channel,
                                              bool isVirtual, bool isFlexibleDataRateCapable);

    virtual QCanBusDeviceInfo deviceInfo() const;
};

Q_DECLARE_TYPEINFO(QCanBusDevice::CanBusError, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QCanBusDevice::CanBusDeviceState, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QCanBusDevice::ConfigurationKey, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QCanBusDevice::Filter, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QCanBusDevice::Filter::FormatFilter, Q_PRIMITIVE_TYPE);

Q_DECLARE_OPERATORS_FOR_FLAGS(QCanBusDevice::Filter::FormatFilters)
Q_DECLARE_OPERATORS_FOR_FLAGS(QCanBusDevice::Directions)

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QCanBusDevice::Filter::FormatFilter)
Q_DECLARE_METATYPE(QList<QCanBusDevice::Filter>)

#endif // QCANBUSDEVICE_H
