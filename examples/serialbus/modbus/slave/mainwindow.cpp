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

#include <QDebug>
#include <QByteArray>
#include <QtSerialBus>
#include <QSerialPort>
#include <QSerialPortInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , modBusDevice(Q_NULLPTR)
    , serialPort(Q_NULLPTR)
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

void MainWindow::discreteInputChanged(int id)
{
    QAbstractButton *button = ui->discreteButtons->button(id);
    bitChanged(id, QModBusDevice::DiscreteInputs, button->isChecked());
}

void MainWindow::coilChanged(int id)
{
    QAbstractButton *button = ui->coilButtons->button(id);
    bitChanged(id, QModBusDevice::Coils, button->isChecked());
}

void MainWindow::bitChanged(int id, QModBusDevice::ModBusTable table, bool value)
{
    if (!modBusDevice)
        return;

    int mapId = qAbs(id) - 2; //default button group id goes from -2 to downwards

    if (!modBusDevice->setData(table, mapId, value))
        qWarning() << modBusDevice->errorString();
}

void MainWindow::init()
{
    setLineEdits();
    connect(ui->discreteButtons, SIGNAL(buttonClicked(int)), this, SLOT(discreteInputChanged(int)));
    connect(ui->coilButtons, SIGNAL(buttonClicked(int)), this, SLOT(coilChanged(int)));

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
    modBusDevice = modBus->createSlave(plugins.at(pluginIndex));

    if (modBusDevice.isNull())
        return;

    modBusDevice->setDevice(serialPort, QModBusDevice::RemoteTerminalUnit);

    if (!modBusDevice->setMap(QModBusDevice::DiscreteInputs, 10))
        return;
    if (!modBusDevice->setMap(QModBusDevice::Coils, 10))
        return;
    if (!modBusDevice->setMap(QModBusDevice::InputRegisters, 10))
        return;
    if (!modBusDevice->setMap(QModBusDevice::HoldingRegisters, 10))
        return;

    connect(modBusDevice.data(), &QModBusSlave::stateChanged, this, &MainWindow::onSlaveStateChanged);

    if (!modBusDevice->connectDevice()) {
        qWarning() << tr("Connect failed: ") << modBusDevice->errorString();
        return;
    }

    modBusDevice->setSlaveId(ui->slaveEdit->text().toInt());

     //default button group id goes from -2 to downwards
    for (int i = -2; i > (-2 - ui->discreteButtons->buttons().size()); i--) {
        modBusDevice->setData(QModBusDevice::DiscreteInputs, qAbs(i) - 2,
            ui->discreteButtons->button(i)->isChecked());
    }


    for (int i = -2; i > (-2 - ui->coilButtons->buttons().size()); i--) {
        modBusDevice->setData(QModBusDevice::Coils, qAbs(i) - 2,
            ui->coilButtons->button(i)->isChecked());
    }

    bool ok;
    for (int i = 0; i < 10; i++) {
        if (!registerFields.at(i)->text().isEmpty()) {
            modBusDevice->setData(QModBusDevice::InputRegisters, i,
                registerFields.at(i)->text().toInt(&ok, 16));
        }
    }

    for (int i = 10; i < 20; i++) {
        if (!registerFields.at(i)->text().isEmpty()) {
            modBusDevice->setData(QModBusDevice::HoldingRegisters, (i - 10),
                registerFields.at(i)->text().toInt(&ok, 16));
        }
    }

    connect(modBusDevice.data(), &QModBusSlave::slaveWritten, this, &MainWindow::updateWidgets);
}

void MainWindow::updateWidgets(QModBusDevice::ModBusTable table, int address, int size)
{
    for (int i = 0; i < size; i++) {
        quint16 value;
        QString text;
        switch (table) {
        case QModBusDevice::Coils:
            modBusDevice->data(QModBusDevice::Coils, address + i, value);
             //default button group id goes from -2 to downwards
            ui->coilButtons->button(-2 - address - i)->setChecked(value);
            break;
        case QModBusDevice::HoldingRegisters:
            modBusDevice->data(QModBusDevice::HoldingRegisters, address + i, value);
            text.setNum(value, 16);
            registerFields.at(10 + address + i)->setText(text);
            break;
        default:
            break;
        }
    }
}

void MainWindow::onSlaveStateChanged(int state)
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

void MainWindow::setLineEdits()
{
    registerFields.append(ui->lineEdit);
    registerFields.append(ui->lineEdit_2);
    registerFields.append(ui->lineEdit_3);
    registerFields.append(ui->lineEdit_4);
    registerFields.append(ui->lineEdit_5);
    registerFields.append(ui->lineEdit_6);
    registerFields.append(ui->lineEdit_7);
    registerFields.append(ui->lineEdit_8);
    registerFields.append(ui->lineEdit_9);
    registerFields.append(ui->lineEdit_10);
    registerFields.append(ui->lineEdit_11);
    registerFields.append(ui->lineEdit_12);
    registerFields.append(ui->lineEdit_13);
    registerFields.append(ui->lineEdit_14);
    registerFields.append(ui->lineEdit_15);
    registerFields.append(ui->lineEdit_16);
    registerFields.append(ui->lineEdit_17);
    registerFields.append(ui->lineEdit_18);
    registerFields.append(ui->lineEdit_19);
    registerFields.append(ui->lineEdit_20);

    for (int i = 0; i < registerFields.size(); i++) {
        registerFields.at(i)->setProperty("id", i);
        connect(registerFields.at(i), &QLineEdit::textChanged, this, &MainWindow::setRegister);
    }
}

void MainWindow::setRegister(const QString &value)
{
    bool ok;
    int hex = value.toInt(&ok, 16);
    if (!modBusDevice || !ok)
        return;

    int id = QObject::sender()->property("id").toInt();
    if (id < 10) {
        if (!modBusDevice->setData(QModBusDevice::InputRegisters, id, hex))
            qWarning() << modBusDevice->errorString();
    } else {
        if (!modBusDevice->setData(QModBusDevice::HoldingRegisters, (id - 10), hex))
            qWarning() << modBusDevice->errorString();
    }
}
