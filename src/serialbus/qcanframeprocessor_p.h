// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QCANFRAMEPROCESSOR_P_H
#define QCANFRAMEPROCESSOR_P_H

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

#include "private/qtserialbusexports_p.h"
#include "qcanframeprocessor.h"
#include "qcanmessagedescription.h"
#include "qcanuniqueiddescription.h"

#include <QtCore/QHash>
#include <QtCore/QSharedData>

#include <utility>

QT_BEGIN_NAMESPACE

class Q_SERIALBUS_PRIVATE_EXPORT QCanFrameProcessorPrivate : public QSharedData
{
public:
    void resetErrors();
    void setError(QCanFrameProcessor::Error err, const QString &desc);
    void addWarning(const QString &warning);
    QVariant decodeSignal(const QCanBusFrame &frame, const QCanSignalDescription &signalDesc);
    QVariant parseData(const unsigned char *data, const QCanSignalDescription &signalDesc);
    void encodeSignal(unsigned char *data, const QVariant &value,
                      const QCanSignalDescription &signalDesc);
    std::optional<QtCanBus::UniqueId> extractUniqueId(const QCanBusFrame &frame) const;
    bool fillUniqueId(unsigned char *data, quint16 sizeInBits, QtCanBus::UniqueId uniqueId);

    inline bool isShared() const { return ref.loadRelaxed() != 1; }

    static QCanFrameProcessorPrivate *get(const QCanFrameProcessor &processor);

    QCanFrameProcessor::Error error = QCanFrameProcessor::Error::NoError;
    QString errorDescription;
    QStringList warnings;
    QHash<QtCanBus::UniqueId, QCanMessageDescription> messages;
    QCanUniqueIdDescription uidDescription;
};

QT_END_NAMESPACE

#endif // QCANFRAMEPROCESSOR_P_H
