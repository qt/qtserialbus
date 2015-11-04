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
#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>

enum ModbusConnection {
    Serial,
    Tcp
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , lastRequest(Q_NULLPTR)
    , modbusDevice(Q_NULLPTR)
{
    ui->setupUi(this);
    ui->readTable->addItem(tr("Coils"), QModbusDataUnit::Coils);
    ui->readTable->addItem(tr("Discrete Inputs"), QModbusDataUnit::DiscreteInputs);
    ui->readTable->addItem(tr("Input Registers"), QModbusDataUnit::InputRegisters);
    ui->readTable->addItem(tr("Holding Registers"), QModbusDataUnit::HoldingRegisters);
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

    ModbusConnection type = static_cast<ModbusConnection> (index);
    if (type == Serial) {
        modbusDevice = new QModbusRtuSerialMaster(this);
    } else if (type == Tcp) {
        modbusDevice = new QModbusTcpClient(this);
    }

    if (!modbusDevice) {
        ui->connectButton->setDisabled(true);
        if (type == Serial)
            ui->errorLabel->setText(tr("Could not create Modbus master."));
        else
            ui->errorLabel->setText(tr("Could not create Modbus client."));
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
        modbusDevice->setPortName(ui->portEdit->text());
        if (!modbusDevice->connectDevice())
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
    if (!modbusDevice || modbusDevice->state() != QModbusDevice::ConnectedState)
        return;

    const QModbusDataUnit::RegisterType registerType =
        static_cast<QModbusDataUnit::RegisterType> (ui->readTable->currentData().toInt());
    QModbusDataUnit dataRequest(registerType);
    dataRequest.setValueCount(ui->readSize->currentText().toInt());
    dataRequest.setStartAddress(ui->readAddress->text().toInt());

    ui->readValue->clear();

    QModbusReply *reply = modbusDevice->sendReadRequest(dataRequest,
                                                        ui->readSlave->text().toInt());
    // broadcast replies return immediately
    if (reply && reply->isFinished()) {
        delete reply;
        return;
    }

    if (reply)
        connect(reply, &QModbusReply::finished, this, &MainWindow::readReady);
    else
        ui->errorLabel->setText(tr("Read error: ") + modbusDevice->errorString());
}

void MainWindow::readReady()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusReply::NoError) {
        const QModbusDataUnit unit = reply->result();
        for (uint i = 0; i < unit.valueCount(); i++) {
            const QString entry = tr("Address: %1, Value: %2").arg(unit.startAddress())
                                     .arg(QString::number(unit.value(i),
                                          unit.registerType() <= QModbusDataUnit::Coils ? 10 : 16));
            ui->readValue->addItem(entry);
        }
    } else {
        ui->errorLabel->setText(tr("Read response error: %1 (code: 0x%2)").
                                        arg(reply->errorText()).arg(reply->error(), -1, 16));
    }

    reply->deleteLater();
}

void MainWindow::on_writeButton_clicked()
{
    if (!modbusDevice || modbusDevice->state() != QModbusDevice::ConnectedState)
        return;

    QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;
    if (ui->writeTable->currentText() == tr("Coils"))
        table = QModbusDataUnit::Coils;

    QModbusDataUnit unit(table, ui->writeAddress->text().toInt(), 1u);
    unit.setValue(0, ui->writeValue->text().toInt(0, 16));

    // TODO extend to test write of single coil and holding register
    // Write Multiple coils and registers as well as R/W MultipleRegisters is missing
    QModbusReply *reply = modbusDevice->sendWriteRequest(unit, ui->writeSlave->text().toInt());

    // broadcast replies return immediately
    if (reply && reply->isFinished()) {
        delete reply;
        return;
    }

    if (reply)
        connect(reply, &QModbusReply::finished, this, &MainWindow::writeReady);
    else
        ui->errorLabel->setText(tr("Write error: ") + modbusDevice->errorString());
}

void MainWindow::writeReady()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() != QModbusReply::NoError) {
        ui->errorLabel->setText(tr("Write response error: %1 (code: 0x%2)").
                                        arg(reply->errorText()).arg(reply->error(), -1, 16));
    }

    reply->deleteLater();
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
