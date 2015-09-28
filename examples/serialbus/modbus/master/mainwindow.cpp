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

#include <QByteArray>
#include <QModbus>
#include <QModbusSerialMaster>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , lastRequest(Q_NULLPTR)
    , modbusDevice(Q_NULLPTR)
{
    ui->setupUi(this);
    ui->readTable->addItem(tr("Coils"), QModbusRegister::Coils);
    ui->readTable->addItem(tr("Discrete Inputs"), QModbusRegister::DiscreteInputs);
    ui->readTable->addItem(tr("Input Registers"), QModbusRegister::InputRegisters);
    ui->readTable->addItem(tr("Holding Registers"), QModbusRegister::HoldingRegisters);
    on_writeTable_currentIndexChanged(ui->writeTable->currentText());

    ui->connectType->setCurrentIndex(0);
    on_connectType_currentIndexChanged(0);
}

MainWindow::~MainWindow()
{
    if (modbusDevice)
        modbusDevice->disconnectDevice();
    delete modbusDevice;

    delete ui;
}

void MainWindow::on_connectType_currentIndexChanged(int index)
{
    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        delete modbusDevice;
        modbusDevice = Q_NULLPTR;
    }

    QModbusDevice::ModBusConnection type = static_cast<QModbusDevice::ModBusConnection> (index);
    if (type == QModbusDevice::Serial) {
        modbusDevice = new QModbusSerialMaster(this);
    } else if (type == QModbusDevice::Tcp) {
        modbusDevice = QModbus::instance()->createMaster("libmodbus",
                           static_cast<QModbusDevice::ModBusConnection> (index));
    }

    if (!modbusDevice) {
        ui->connectButton->setDisabled(true);
        ui->errorLabel->setText(tr("Could not create modbus master."));
    } else {
        connect(modbusDevice, &QModbusClient::stateChanged,
                this, &MainWindow::onStateChanged);
    }
}

void MainWindow::on_connectButton_clicked()
{
    if (!modbusDevice)
        return;

    ui->errorLabel->setText(QString());
    if (modbusDevice->state() != QModbusDevice::ConnectedState) {
        const QString portString = ui->portEdit->text();
        QModbusSerialMaster *serialMaster = qobject_cast<QModbusSerialMaster *>(modbusDevice);
        bool connectResult = false;

        if (serialMaster) {
            connectResult = serialMaster->connectDevice(portString);
        } else {
            modbusDevice->setPortName(ui->portEdit->text());
            connectResult = modbusDevice->connectDevice();
        }
        if (!connectResult)
            ui->errorLabel->setText(tr("Connect failed: ") + modbusDevice->errorString());
    } else {
        modbusDevice->disconnectDevice();
    }
}

void MainWindow::onStateChanged(int state)
{
    if (state == QModbusDevice::UnconnectedState)
        ui->connectButton->setText(tr("Connect"));
    else if (state == QModbusDevice::ConnectedState)
        ui->connectButton->setText(tr("Disconnect"));
}

void MainWindow::on_readButton_clicked()
{
    delete lastRequest;
    if (!modbusDevice || modbusDevice->state() != QModbusDevice::ConnectedState)
        return;

    const QModbusRegister::RegisterType registerType =
        static_cast<QModbusRegister::RegisterType> (ui->readTable->currentData().toInt());
    QModbusDataUnit dataRequest(registerType);
    dataRequest.setValueCount(ui->readSize->currentText().toInt());
    dataRequest.setStartAddress(ui->readAddress->text().toInt());

    ui->readValue->clear();
    lastRequest = modbusDevice->read(dataRequest, ui->readSlave->text().toInt());
    if (lastRequest)
        connect(lastRequest, &QModbusReply::finished, this, &MainWindow::readReady);
    else
        ui->errorLabel->setText(tr("Read error: ") + modbusDevice->errorString());
}

void MainWindow::readReady()
{
    const QList<QModbusDataUnit> units = lastRequest->result();
    for (int i = 0; i < units.size(); i++) {
        const QString entry = tr("Address: ") + QString::number(units.at(i).startAddress())
            + tr(" Value: ") + QString::number(units.at(i).values().at(0),
                units.at(i).registerType() <= QModbusRegister::Coils ? 10 : 16);
        ui->readValue->addItem(entry);
    }
    lastRequest->deleteLater();
    lastRequest = Q_NULLPTR;
}

void MainWindow::on_writeButton_clicked()
{
    delete lastRequest;
    if (!modbusDevice || modbusDevice->state() != QModbusDevice::ConnectedState)
        return;

    QModbusRegister::RegisterType table = QModbusRegister::HoldingRegisters;
    if (ui->writeTable->currentText() == tr("Coils"))
        table = QModbusRegister::Coils;

    lastRequest = modbusDevice->write(QModbusDataUnit(table, ui->writeAddress->text().toInt(),
        ui->writeValue->text().toInt(0, 16)), ui->readSlave->text().toInt());
    if (lastRequest)
        connect(lastRequest, &QModbusReply::finished, this, &MainWindow::writeReady);
    else
        ui->errorLabel->setText(tr("Write error: ") + modbusDevice->errorString());
}

void MainWindow::writeReady()
{
    lastRequest->deleteLater();
    lastRequest = Q_NULLPTR;
}

void MainWindow::on_writeTable_currentIndexChanged(const QString &text)
{
    ui->writeValue->clear();
    if (text == tr("Coils")) {
        ui->writeValue->setValidator(new QIntValidator(0, 1, this));
        ui->writeValue->setPlaceholderText(tr("Binary 0-1."));
    } else if (text == tr("Holding Registers")) {
        ui->writeValue->setValidator(new QRegExpValidator(QRegExp(QStringLiteral("[0-9a-f]{0,4}"),
            Qt::CaseInsensitive), this));
        ui->writeValue->setPlaceholderText(tr("Hexadecimal A-F, a-f, 0-9."));
    }
}
