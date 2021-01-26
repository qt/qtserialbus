/****************************************************************************
**
** Copyright (C) 2017 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2021 The Qt Company Ltd.
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
****************************************************************************/

#ifndef TINYCANBACKEND_P_H
#define TINYCANBACKEND_P_H

#include "tinycanbackend.h"
#include "tinycan_symbols_p.h"

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class QWinEventNotifier;
class QTimer;

class TinyCanBackendPrivate
{
    Q_DECLARE_PUBLIC(TinyCanBackend)
public:
    TinyCanBackendPrivate(TinyCanBackend *q);
    ~TinyCanBackendPrivate();

    bool open();
    void close();
    bool setConfigurationParameter(int key, const QVariant &value);

    QString systemErrorString(int errorCode);
    void setupChannel(const QString &interfaceName);
    void setupDefaultConfigurations();
    void startWrite();
    void startRead();
    void startupDriver();
    void cleanupDriver();
    void resetController();

    bool setBitRate(int bitrate);

    TinyCanBackend * const q_ptr;

    bool isOpen = false;
    int channelIndex = INDEX_INVALID;
    QTimer *writeNotifier = nullptr;
};

QT_END_NAMESPACE

#endif // TINYCANBACKEND_P_H
