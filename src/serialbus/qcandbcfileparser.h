// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QCANDBCFILEPARSER_H
#define QCANDBCFILEPARSER_H

#include <QtCore/QList>

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

    QCanDbcFileParser();
    ~QCanDbcFileParser();

    bool parse(const QString &fileName);
    bool parse(const QStringList &fileNames);

    QList<QCanMessageDescription> messageDescriptions() const;

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
