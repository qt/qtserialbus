/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the QtSerialBus module.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtSerialBus>
#include <QCanBusFrame>

#include <QtCore/qbytearray.h>
#include <QtCore/qvariant.h>
#include <QtCore/qdebug.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    deviceName(QStringLiteral("")),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::init()
{
    QPointer<QCanBus> canBus = QCanBus::instance();

    plugins = canBus->plugins();
    for (int i = 0; i < plugins.size(); i++)
        ui->pluginBox->insertItem(i, plugins.at(i));
    connectDevice(ui->pluginBox->currentIndex());
}

void MainWindow::connectDevice(int pluginIndex)
{
    QPointer<QCanBus> canBus = QCanBus::instance();

    canDevice = canBus->createDevice(plugins.at(pluginIndex), deviceName);

    if (canDevice.isNull())
        return;
    connect(canDevice.data(), &QCanBusDevice::errorOccurred, this, &MainWindow::receiveError);
    if (!canDevice->connectDevice()) {
        canDevice.clear();
        return;
    }
    canDevice->setConfigurationParameter(QStringLiteral("ReceiveOwnMessages"), QVariant(1));
    canDevice->setConfigurationParameter(QStringLiteral("ErrorMask"), QVariant(0x1FFFFFFFU));
    connect(canDevice.data(), &QCanBusDevice::frameReceived, this, &MainWindow::checkMessages);

    ui->deviceLabel->setText("Connected to: " + deviceName);

    checkMessages();
}

void MainWindow::receiveError(QCanBusDevice::CanBusError error) const
{
    switch (error) {
    case QCanBusDevice::ReadError:
    case QCanBusDevice::WriteError:
    case QCanBusDevice::ConnectionError:
    case QCanBusDevice::ConfigurationError:
    case QCanBusDevice::UnknownError:
        qWarning() << canDevice->errorString();
    default:
        break;
    }
}

void MainWindow::checkMessages()
{
    if (canDevice.isNull())
        return;

    const QCanBusFrame frame = canDevice->readFrame();

    if (frame.payload().isEmpty())
        return;

    const qint8 dataLength = frame.payload().size();

    const qint32 id = frame.frameId();

    QString view;
    if (frame.frameType() == QCanBusFrame::ErrorFrame) {
        interpretError(view, frame);
    } else {
        view += QLatin1String("Id: ");
        view += QString::number(id, 16);
        view += QLatin1String(" bytes: ");
        view += QString::number(dataLength, 10);
        view += QLatin1String(" data: ");
        view += frame.payload().data();
    }

    if (frame.frameType() == QCanBusFrame::RemoteRequestFrame) {
        ui->requestList->addItem(view);
    } else if (frame.frameType() == QCanBusFrame::ErrorFrame) {
        ui->errorList->addItem(view);
    } else {
        ui->listWidget->addItem(view);
    }
}

void MainWindow::on_sendButton_clicked() const
{
    if (canDevice.isNull())
        return;

    QByteArray writings = ui->lineEdit->displayText().toUtf8();
    ui->lineEdit->clear();

    QCanBusFrame frame;
    const int maxPayload = ui->fdBox->checkState() ? 64 : 8;
    int size = writings.size();
    if (size > maxPayload)
        size = maxPayload;
    writings = writings.left(size);
    frame.setPayload(writings);

    qint32 id = ui->idEdit->displayText().toInt();
    ui->idEdit->clear();
    if (!ui->EFF->checkState() && id > 2047) //11 bits
        id = 2047;

    frame.setFrameId(id);
    frame.setExtendedFrameFormat(ui->EFF->checkState());

    if (ui->RTR->checkState())
        frame.setFrameType(QCanBusFrame::RemoteRequestFrame);
    else if (ui->ERR->checkState())
        frame.setFrameType(QCanBusFrame::ErrorFrame);
    else
        frame.setFrameType(QCanBusFrame::DataFrame);

    canDevice->writeFrame(frame);
}

void MainWindow::on_connectButton_clicked()
{
    canDevice.clear();
    deviceName = ui->deviceEdit->text();
    ui->deviceEdit->clear();
    ui->deviceLabel->setText("Connected to:");
    connectDevice(ui->pluginBox->currentIndex());
}

void MainWindow::on_pluginBox_activated(int index)
{
    canDevice.clear();
    ui->deviceLabel->setText("Connected to:");
    connectDevice(index);
}

void MainWindow::interpretError(QString &view, const QCanBusFrame &frame)
{
    if (canDevice.isNull())
        return;

    view = canDevice->interpretErrorFrame(frame);
}
