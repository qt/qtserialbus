// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef READTASK_H
#define READTASK_H

#include <QObject>
#include <QtSerialBus>
#include <QCanBusFrame>

class ReadTask : public QObject
{
    Q_OBJECT
public:
    explicit ReadTask(QTextStream &m_output, QObject *parent = nullptr);
    void setShowTimeStamp(bool showStamp);
    bool isShowFlags() const;
    void setShowFlags(bool isShowFlags);

public slots:
    void handleFrames();
    void handleError(QCanBusDevice::CanBusError /*error*/);

private:
    QTextStream &m_output;
    bool m_showTimeStamp = false;
    bool m_showFlags = false;
};

#endif // READTASK_H
