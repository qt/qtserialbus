// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

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
