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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCanBusDevice>

#include <QMainWindow>
#include <QPointer>

#include <linux/can.h>

class QSerialBusBackend;
class QSerialBus;
class QBusDummyDevice;
class QSerialBusDevice;
class QCanFrame;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private Q_SLOTS:

    void checkDummyMessages();
    void checkMessages();
    void on_sendButton_clicked();
    void on_connectButton_clicked();
    void receiveError(QCanBusDevice::CanBusError);

private:
    void init();
    void interpretError(QString&, const QCanFrame&);

    QPointer<QSerialBus> b;
    QPointer<QBusDummyDevice> dummyDevice;
    QPointer<QCanBusDevice> canDevice;
    Ui::MainWindow *ui;
    QList< QPointer<QSerialBusBackend> > backends;
    QList<QSerialBusDevice *> devices;
    qint64 currentDevice;
};

#endif // MAINWINDOW_H
