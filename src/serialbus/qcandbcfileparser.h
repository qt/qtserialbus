// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QCANDBCFILEPARSER_H
#define QCANDBCFILEPARSER_H

#include <QtCore/QList>

#include <QtSerialBus/qcancommondefinitions.h>
#include <QtSerialBus/qtserialbusglobal.h>

#include <memory>

QT_BEGIN_NAMESPACE

class QCanDbcFileParserPrivate;
class QCanMessageDescription;
class QCanUniqueIdDescription;

class Q_SERIALBUS_EXPORT QCanDbcFileParser
{
public:
    enum class Error : quint8 {
        NoError = 0,
        FileReadError,
        ParseError
    };

    // The DBC protocol uses unsinged_integer to describe the supported values.
    // Do we need to use QVariant instead of quint32? Or qint64 for better BC
    // guarantees?
    using ValueDescriptions = QHash<quint32, QString>;
    using SignalValueDescriptions = QHash<QString, ValueDescriptions>;
    using MessageValueDescriptions = QHash<QtCanBus::UniqueId, SignalValueDescriptions>;

    QCanDbcFileParser();
    ~QCanDbcFileParser();

    bool parse(const QString &fileName);
    bool parse(const QStringList &fileNames);

    QList<QCanMessageDescription> messageDescriptions() const;
    MessageValueDescriptions valueDescriptions() const;

    Error error() const;
    QString errorString() const;
    QStringList warnings() const;

    static QCanUniqueIdDescription uniqueIdDescription();

private:
    std::unique_ptr<QCanDbcFileParserPrivate> d;
    friend class QCanDbcFileParserPrivate;

    Q_DISABLE_COPY_MOVE(QCanDbcFileParser)
};

QT_END_NAMESPACE

#endif // QCANDBCFILEPARSER_H
