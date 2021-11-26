/****************************************************************************
**
** Copyright (C) 2017 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
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
    bool setConfigurationParameter(QCanBusDevice::ConfigurationKey key, const QVariant &value);

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
