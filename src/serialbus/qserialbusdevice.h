/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
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

#ifndef QSERIALBUSDEVICE_H
#define QSERIALBUSDEVICE_H

#include <QtSerialBus/qserialbusglobal.h>
#include <QtCore/qiodevice.h>

QT_BEGIN_NAMESPACE

class QSerialBusBackend;

//TODO: close()/open()/connect() functions missing
//TODO: error reporting missing
//TODO: state reporting missing
class Q_SERIALBUS_EXPORT QSerialBusDevice : public QIODevice
{
    Q_OBJECT

public:
    explicit QSerialBusDevice(QPointer<QSerialBusBackend> backend, QObject *parent = 0);
protected:
    virtual qint64 readData(char *data, qint64 maxSize) Q_DECL_OVERRIDE;
    virtual qint64 writeData(const char *data, qint64 maxSize) Q_DECL_OVERRIDE;

private:
    QPointer<QSerialBusBackend> busBackend;
};

QT_END_NAMESPACE

#endif // QSERIALBUSDEVICE_H
