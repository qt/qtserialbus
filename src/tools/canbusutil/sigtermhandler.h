/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the QtSerialBus module.
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

#ifndef SIGTERMHANDLER_H
#define SIGTERMHANDLER_H

#include <QObject>

class SigTermHandler : public QObject
{
    Q_OBJECT
public:
    static SigTermHandler *instance();
    static void handle(int s);

    virtual ~SigTermHandler();
signals:
    void sigTermSignal();

private:
    explicit SigTermHandler(QObject *parent = nullptr);
};

#endif // SIGTERMHANDLER_H
