// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QCANFRAMEPROCESSOR_H
#define QCANFRAMEPROCESSOR_H

#include <QtCore/QVariantMap>

#include <QtSerialBus/qcancommondefinitions.h>
#include <QtSerialBus/qtserialbusglobal.h>

#include <memory>

QT_BEGIN_NAMESPACE

class QCanBusFrame;
class QCanMessageDescription;
class QCanUniqueIdDescription;
class QCanFrameProcessorPrivate;

class Q_SERIALBUS_EXPORT QCanFrameProcessor
{
public:
    enum class Error : quint8 {
        NoError = 0,
        InvalidFrame,
        UnsupportedFrameFormat,
        DecodingError,
        EncodingError,
    };

    struct ParseResult {
        QtCanBus::UniqueId uniqueId = 0;
        QVariantMap signalValues;
    };

    QCanFrameProcessor();
    ~QCanFrameProcessor();

    QCanBusFrame prepareFrame(QtCanBus::UniqueId uniqueId, const QVariantMap &signalValues);
    ParseResult parseFrame(const QCanBusFrame &frame);

    Error error() const;
    QString errorString() const;
    QStringList warnings() const;

    QList<QCanMessageDescription> messageDescriptions() const;
    void addMessageDescriptions(const QList<QCanMessageDescription> &descriptions);
    void setMessageDescriptions(const QList<QCanMessageDescription> &descriptions);
    void clearMessageDescriptions();

    QCanUniqueIdDescription uniqueIdDescription() const;
    void setUniqueIdDescription(const QCanUniqueIdDescription &description);

private:
    std::unique_ptr<QCanFrameProcessorPrivate> d;
    friend class QCanFrameProcessorPrivate;

    Q_DISABLE_COPY_MOVE(QCanFrameProcessor)
};

QT_END_NAMESPACE

#endif // QCANFRAMEPROCESSOR_H
