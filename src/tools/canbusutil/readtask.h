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
