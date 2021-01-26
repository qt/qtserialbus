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

#include "sigtermhandler.h"

#include <QTextStream>

SigTermHandler *SigTermHandler::instance()
{
    static auto inst = new SigTermHandler();
    return inst;
}

void SigTermHandler::handle(int s)
{
    QTextStream out(stdout);
    out << " Caught signal " << s << Qt::endl;
    emit instance()->sigTermSignal();
}

SigTermHandler::~SigTermHandler() { }

SigTermHandler::SigTermHandler(QObject *parent) : QObject(parent) { }
