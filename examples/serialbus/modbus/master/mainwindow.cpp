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
#include "settingsdialog.h"
#include "writeregistermodel.h"

#include <QtSerialBus/qmodbustcpclient.h>
#include <QtSerialBus/qmodbusrtuserialmaster.h>
#include <QtCore/qbytearray.h>
#include <QtWidgets/qstatusbar.h>

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

    m_settingsDialog = new SettingsDialog(this);

    initActions();

    writeModel = new WriteRegisterModel(this);
    ui->writeValueTable->setModel(writeModel);
    ui->writeValueTable->resizeColumnToContents(0);
    ui->writeValueTable->horizontalHeader()->setStretchLastSection(true);
    ui->writeValueTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->readTable->addItem(tr("Coils"), QModbusDataUnit::Coils);
    ui->readTable->addItem(tr("Discrete Inputs"), QModbusDataUnit::DiscreteInputs);
    ui->readTable->addItem(tr("Input Registers"), QModbusDataUnit::InputRegisters);
    ui->readTable->addItem(tr("Holding Registers"), QModbusDataUnit::HoldingRegisters);

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

void MainWindow::initActions()
{
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionExit->setEnabled(true);
    ui->actionOptions->setEnabled(true);

    connect(ui->actionConnect, &QAction::triggered,
            this, &MainWindow::on_connectButton_clicked);
    connect(ui->actionDisconnect, &QAction::triggered,
            this, &MainWindow::on_connectButton_clicked);

    connect(ui->actionExit, &QAction::triggered, this, &QMainWindow::close);
    connect(ui->actionOptions, &QAction::triggered, m_settingsDialog, &QDialog::show);
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
        if (ui->portEdit->text().isEmpty())
            ui->portEdit->setText(QLatin1Literal("127.0.0.1:502"));
    }

    connect(modbusDevice, &QModbusClient::errorOccurred,
            [this](QModbusDevice::ModbusError){
        statusBar()->showMessage(modbusDevice->errorString(), 5000);
    });

    if (!modbusDevice) {
        ui->connectButton->setDisabled(true);
        if (type == Serial)
            statusBar()->showMessage(tr("Could not create Modbus master."), 5000);
        else
            statusBar()->showMessage(tr("Could not create Modbus client."), 5000);
    } else {
        connect(modbusDevice, &QModbusClient::stateChanged,
                this, &MainWindow::onStateChanged);
    }
}

void MainWindow::on_connectButton_clicked()
{
    if (!modbusDevice)
        return;

    statusBar()->clearMessage();
    if (modbusDevice->state() != QModbusDevice::ConnectedState) {
        modbusDevice->setPortName(ui->portEdit->text());
        modbusDevice->setTimeout(m_settingsDialog->settings().responseTime);
        if (!modbusDevice->connectDevice()) {
            statusBar()->showMessage(tr("Connect failed: ") + modbusDevice->errorString(), 5000);
        } else {
            ui->actionConnect->setEnabled(false);
            ui->actionDisconnect->setEnabled(true);
        }
    } else {
        modbusDevice->disconnectDevice();
        ui->actionConnect->setEnabled(true);
        ui->actionDisconnect->setEnabled(false);
    }
}

void MainWindow::onStateChanged(int state)
{
    bool connected = (state != QModbusDevice::UnconnectedState);
    ui->actionConnect->setEnabled(!connected);
    ui->actionDisconnect->setEnabled(connected);

    if (state == QModbusDevice::UnconnectedState)
        ui->connectButton->setText(tr("Connect"));
    else if (state == QModbusDevice::ConnectedState)
        ui->connectButton->setText(tr("Disconnect"));
}

void MainWindow::on_readButton_clicked()
{
    if (!modbusDevice)
        return;

    const QModbusDataUnit::RegisterType registerType =
        static_cast<QModbusDataUnit::RegisterType> (ui->readTable->currentData().toInt());
    QModbusDataUnit dataRequest(registerType);

    int numberOfEntries = ui->readSize->currentText().toInt();
    int startAddress = ui->readAddress->text().toInt();

    // do not go beyond 10 entries
    numberOfEntries = qMin(numberOfEntries, 10 - startAddress);
    Q_ASSERT(startAddress >= 0 && startAddress < 10);

    dataRequest.setValueCount(numberOfEntries);
    dataRequest.setStartAddress(startAddress);

    ui->readValue->clear();
    statusBar()->clearMessage();

    QModbusReply *reply = modbusDevice->sendReadRequest(dataRequest,
                                                        ui->readServer->text().toInt());
    // broadcast replies return immediately
    if (reply && reply->isFinished()) {
        delete reply;
        return;
    }

    if (reply)
        connect(reply, &QModbusReply::finished, this, &MainWindow::readReady);
    else
        statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
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
    } else if (reply->error() == QModbusReply::ProtocolError) {
        statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)").
                                    arg(reply->errorText()).
                                    arg(reply->rawResult().exceptionCode(), -1, 16), 5000);
    } else {
        statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                                    arg(reply->errorText()).
                                    arg(reply->error(), -1, 16), 5000);
    }

    reply->deleteLater();
}

void MainWindow::on_writeButton_clicked()
{
    if (!modbusDevice)
        return;

    QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;
    if (ui->writeTable->currentText() == tr("Coils"))
        table = QModbusDataUnit::Coils;

    int numberOfEntries = ui->writeSize->currentText().toInt();
    int startAddress = ui->writeAddress->text().toInt();
    // do not go beyond 10 entries
    numberOfEntries = qMin(numberOfEntries, 10 - startAddress);

    Q_ASSERT(startAddress >= 0 && startAddress < 10);
    QModbusDataUnit writeUnit(table, startAddress, numberOfEntries);
    for (int i = startAddress; i < (startAddress + numberOfEntries); i++)
    {
        if (table == QModbusDataUnit::Coils)
            writeUnit.setValue(i - startAddress, writeModel->m_coils[i]);
        else
            writeUnit.setValue(i - startAddress, writeModel->m_holdingRegisters[i]);
    }

    statusBar()->clearMessage();

    // TODO test for R/W MultipleRegisters is missing
    QModbusReply *reply = modbusDevice->sendWriteRequest(writeUnit, ui->writeServer->text().toInt());

    // broadcast replies return immediately
    if (reply && reply->isFinished()) {
        delete reply;
        return;
    }

    if (reply)
        connect(reply, &QModbusReply::finished, this, &MainWindow::writeReady);
    else
       statusBar()->showMessage(tr("Write error: ") + modbusDevice->errorString(), 5000);
}

void MainWindow::writeReady()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusReply::ProtocolError) {
        statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)").
                                    arg(reply->errorText()).
                                    arg(reply->rawResult().exceptionCode(), -1, 16), 5000);
    } else if (reply->error() != QModbusReply::NoError) {
        statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                                    arg(reply->errorText()).
                                    arg(reply->error(), -1, 16), 5000);
    }

    reply->deleteLater();
}
