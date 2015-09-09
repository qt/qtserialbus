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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , modBusDevice(Q_NULLPTR)
    , serialPort(Q_NULLPTR)
    , lastRequest(Q_NULLPTR)
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
    ui->writeAddress->setInputMask(QStringLiteral("9"));
    on_writeTable_currentIndexChanged(ui->writeTable->currentText());

    QPointer<QModBus> modBus = QModBus::instance();

    plugins = modBus->plugins();
    for (int i = 0; i < plugins.size(); i++)
        ui->pluginBox->insertItem(i, QString::fromLatin1(plugins.at(i)));
    ui->pluginBox->setCurrentText(QStringLiteral("libmodbus"));
}

void MainWindow::connectDevice(int pluginIndex)
{
    QPointer<QModBus> modBus = QModBus::instance();

    if (!serialPort.isNull())
        serialPort.clear();
    if (!modBusDevice.isNull())
        modBusDevice.clear();

    serialPort = new QSerialPort(ui->portEdit->text());
    modBusDevice = modBus->createMaster(plugins.at(pluginIndex));

    if (modBusDevice.isNull())
        return;

    modBusDevice->setDevice(serialPort, QModBusDevice::RemoteTerminalUnit);
    modBusDevice->setPortName(ui->portEdit->text());

    connect(modBusDevice.data(), &QModBusMaster::stateChanged, this, &MainWindow::onMasterStateChanged);

    if (!modBusDevice->connectDevice()) {
        qWarning() << tr("Connect failed: ") << modBusDevice->errorString();
        return;
    }
}

void MainWindow::onMasterStateChanged(int state)
{
    if (state == QModBusDevice::UnconnectedState) {
        ui->pushButton->setText(tr("Connect"));
        ui->connectedLabel->setText(tr("Connected to: "));
    } else if (state == QModBusDevice::ConnectedState) {
        ui->pushButton->setText(tr("Disconnect"));
        ui->connectedLabel->setText(tr("Connected to: ") + serialPort->portName());
    }
}

void MainWindow::on_pushButton_clicked()
{
    if (modBusDevice && modBusDevice->state() == QModBusDevice::ConnectedState) {
        modBusDevice->disconnectDevice();
        modBusDevice.clear();
    } else {
        connectDevice(ui->pluginBox->currentIndex());
    }
}

void MainWindow::on_readButton_clicked()
{
    lastRequest.clear();
    QList<QModBusDataUnit> units;
    QModBusDevice::ModBusTable table;
    if (ui->readTable->currentText() == tr("Discrete Inputs"))
        table = QModBusDevice::DiscreteInputs;
    else if (ui->readTable->currentText() == tr("Coils"))
        table = QModBusDevice::Coils;
    else if (ui->readTable->currentText() == tr("Input Registers"))
        table = QModBusDevice::InputRegisters;
    else if (ui->readTable->currentText() == tr("Holding Registers"))
        table = QModBusDevice::HoldingRegisters;
    else
        return;

    for (int i = 0; i < ui->readSize->currentText().toInt(); i++)
        units.append(QModBusDataUnit(table, ui->readAddress->text().toInt() + i));

    lastRequest = modBusDevice->read(units, ui->readSlave->text().toInt());
    connect(lastRequest.data(), &QModBusReply::finished, this, &MainWindow::readReady);
}

void MainWindow::readReady()
{
    QList<QModBusDataUnit> units = lastRequest->result();
    for (int i = 0; i < units.size(); i++) {
        const QString entry = tr("Address: ") + QString::number(units.at(i).address())
            + tr(" Value: ") + QString::number(units.at(i).value());
        ui->readValue->addItem(entry);
    }
    lastRequest.clear();
}

void MainWindow::on_writeButton_clicked()
{
    lastRequest.clear();

    QModBusDevice::ModBusTable table;
    if (ui->writeTable->currentText() == tr("Coils"))
        table = QModBusDevice::Coils;
    else
        table = QModBusDevice::HoldingRegisters;

    const QModBusDataUnit unit(table, ui->writeAddress->text().toInt(), ui->writeValue->text()
        .toInt(0,16));
    lastRequest = modBusDevice->write(unit, ui->readSlave->text().toInt());
    if (lastRequest.isNull())
        qWarning() << modBusDevice->errorString();

    connect(lastRequest.data(), &QModBusReply::finished, this, &MainWindow::writeReady);
}

void MainWindow::writeReady()
{
    lastRequest.clear();
}

void MainWindow::on_writeTable_currentIndexChanged(const QString &text)
{
    if (text == tr("Coils")) {
        ui->writeValue->setInputMask(QStringLiteral("B"));
    } else if (text == tr("Holding Registers")) {
        ui->writeValue->setInputMask(QStringLiteral("HHHH"));
    }
}
