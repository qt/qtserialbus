/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#ifndef QMODBUSRTUSERIALCLIENT_H
#define QMODBUSRTUSERIALCLIENT_H

#include <QtSerialBus/qmodbusclient.h>

QT_BEGIN_NAMESPACE

class QModbusRtuSerialClientPrivate;

class Q_SERIALBUS_EXPORT QModbusRtuSerialClient : public QModbusClient
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModbusRtuSerialClient)

public:
    explicit QModbusRtuSerialClient(QObject *parent = nullptr);
    ~QModbusRtuSerialClient();

    int interFrameDelay() const;
    void setInterFrameDelay(int microseconds);

    int turnaroundDelay() const;
    void setTurnaroundDelay(int turnaroundDelay);

protected:
    QModbusRtuSerialClient(QModbusRtuSerialClientPrivate &dd, QObject *parent = nullptr);

    void close() override;
    bool open() override;
};

#if QT_DEPRECATED_SINCE(6, 2)
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wattributes")
using QModbusRtuSerialMaster
    Q_DECL_DEPRECATED_X("Please port your application to QModbusRtuSerialClient.") = QModbusRtuSerialClient;
#endif
QT_WARNING_POP
QT_END_NAMESPACE

#endif // QMODBUSRTUSERIALCLIENT_H
