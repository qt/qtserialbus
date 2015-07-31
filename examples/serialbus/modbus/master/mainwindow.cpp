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

#include <QSerialPort>
#include <QSerialPortInfo>

#include <QtCore/qbytearray.h>
#include <QtCore/qdebug.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    if (!modBusDevice.isNull())
        modBusDevice->disconnectDevice();
    delete ui;
}

void MainWindow::init()
{
    QPointer<QModBus> modBus = QModBus::instance();

    plugins = modBus->plugins();
    for (int i = 0; i < plugins.size(); i++)
        ui->pluginBox->insertItem(i, plugins.at(i));
}

void MainWindow::connectDevice(int pluginIndex)
{
    QPointer<QModBus> modBus = QModBus::instance();

    if (!serialPort.isNull())
        serialPort.clear();
    if (!modBusDevice.isNull())
        modBusDevice.clear();

    serialPort = new QSerialPort(ui->portEdit->text());
    ui->portEdit->clear();

    modBusDevice = modBus->createMaster(plugins.at(pluginIndex), serialPort);

    if (modBusDevice.isNull())
        return;

    connect(modBusDevice.data(), &QModBusMaster::stateChanged, this, &MainWindow::onMasterStateChanged);

    if (!modBusDevice->connectDevice()) {
        qWarning() << "Connect failed: " << modBusDevice->errorString();
        return;
    }
}

void MainWindow::onMasterStateChanged(int state)
{
    if (state == QModBusDevice::UnconnectedState)
        ui->connectedLabel->setText("Connected to: ");
    else if (state == QModBusDevice::ConnectedState)
        ui->connectedLabel->setText("Connected to: " + serialPort->portName());
}

void MainWindow::on_pushButton_clicked()
{
    connectDevice(ui->pluginBox->currentIndex());
}

void MainWindow::on_pushButton_2_clicked()
{
    lastRequest.clear();

    QModBusDataUnit unit(QModBusDevice::HoldingRegisters, 5);

    lastRequest = modBusDevice->read(unit, 1);
    connect(lastRequest.data(), &QModBusReply::finished, this, &MainWindow::readReady);
}

void MainWindow::readReady()
{
    QList<QModBusDataUnit> units = lastRequest->result();
    for (int i = 0; i < units.size(); i++)
        qDebug() << "Value at address" << units.at(i).address() << "is" << units.at(i).value();
    lastRequest.clear();
}

void MainWindow::on_writeButton_clicked()
{
    lastRequest.clear();

    QModBusDataUnit unit(QModBusDevice::HoldingRegisters, 5, 0xda2f);

    lastRequest = modBusDevice->write(unit);
    connect(lastRequest.data(), &QModBusReply::finished, this, &MainWindow::writeReady);
}

void MainWindow::writeReady()
{
    lastRequest.clear();
}
