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
#include <QModBus>
#include <QModBusMaster>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , lastRequest(Q_NULLPTR)
    , modBusDevice(Q_NULLPTR)
{
    ui->setupUi(this);
    ui->readTable->addItem(tr("Coils"), QModBusDevice::Coils);
    ui->readTable->addItem(tr("Discrete Inputs"), QModBusDevice::DiscreteInputs);
    ui->readTable->addItem(tr("Input Registers"), QModBusDevice::InputRegisters);
    ui->readTable->addItem(tr("Holding Registers"), QModBusDevice::HoldingRegisters);
    on_writeTable_currentIndexChanged(ui->writeTable->currentText());

    modBusDevice = QModBus::instance()->createMaster("libmodbus");
    if (!modBusDevice) {
        ui->pushButton->setDisabled(true);
        ui->errorLabel->setText(tr("Could not create modbus master."));
    } else {
        connect(modBusDevice, &QModBusMaster::stateChanged, this, &MainWindow::onStateChanged);
    }
}

MainWindow::~MainWindow()
{
    if (modBusDevice)
        modBusDevice->disconnectDevice();
    delete modBusDevice;

    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    if (!modBusDevice)
        return;

    ui->errorLabel->setText(QString());
    if (modBusDevice->state() != QModBusDevice::ConnectedState) {
        modBusDevice->setPortName(ui->portEdit->text());
        if (!modBusDevice->connectDevice())
            ui->errorLabel->setText(tr("Connect failed: ") + modBusDevice->errorString());
    } else {
        modBusDevice->disconnectDevice();
    }
}

void MainWindow::onStateChanged(int state)
{
    if (state == QModBusDevice::UnconnectedState)
        ui->pushButton->setText(tr("Connect"));
    else if (state == QModBusDevice::ConnectedState)
        ui->pushButton->setText(tr("Disconnect"));
}

void MainWindow::on_readButton_clicked()
{
    delete lastRequest;
    if (!modBusDevice || modBusDevice->state() != QModBusDevice::ConnectedState)
        return;

    QList<QModBusDataUnit> units;
    const QModBusDevice::ModBusTable table =
        static_cast<QModBusDevice::ModBusTable> (ui->readTable->currentData().toInt());
    for (int i = 0; i < ui->readSize->currentText().toInt(); ++i)
        units.append(QModBusDataUnit(table, ui->readAddress->text().toInt() + i));

    ui->readValue->clear();
    lastRequest = modBusDevice->read(units, ui->readSlave->text().toInt());
    if (lastRequest)
        connect(lastRequest, &QModBusReply::finished, this, &MainWindow::readReady);
    else
        ui->errorLabel->setText(tr("Read error: ") + modBusDevice->errorString());
}

void MainWindow::readReady()
{
    const QList<QModBusDataUnit> units = lastRequest->result();
    for (int i = 0; i < units.size(); i++) {
        const QString entry = tr("Address: ") + QString::number(units.at(i).address())
            + tr(" Value: ") + QString::number(units.at(i).value(),
                units.at(i).tableType() <= QModBusDevice::Coils ? 10 : 16);
        ui->readValue->addItem(entry);
    }
    lastRequest->deleteLater();
    lastRequest = Q_NULLPTR;
}

void MainWindow::on_writeButton_clicked()
{
    delete lastRequest;
    if (!modBusDevice || modBusDevice->state() != QModBusDevice::ConnectedState)
        return;

    QModBusDevice::ModBusTable table = QModBusDevice::HoldingRegisters;
    if (ui->writeTable->currentText() == tr("Coils"))
        table = QModBusDevice::Coils;

    lastRequest = modBusDevice->write(QModBusDataUnit(table, ui->writeAddress->text().toInt(),
        ui->writeValue->text().toInt(0, 16)), ui->readSlave->text().toInt());
    if (lastRequest)
        connect(lastRequest, &QModBusReply::finished, this, &MainWindow::writeReady);
    else
        ui->errorLabel->setText(tr("Write error: ") + modBusDevice->errorString());
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
