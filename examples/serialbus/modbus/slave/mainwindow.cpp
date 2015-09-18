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

#include <QModBus>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , modBusDevice(Q_NULLPTR)
{
    ui->setupUi(this);
    setupWidgetContainers();

    ui->connectType->setCurrentIndex(0);
    on_connectType_currentIndexChanged(0);
}

MainWindow::~MainWindow()
{
    if (modBusDevice)
        modBusDevice->disconnectDevice();
    delete modBusDevice;

    delete ui;
}

void MainWindow::on_connectType_currentIndexChanged(int index)
{
    if (modBusDevice) {
        modBusDevice->disconnect();
        delete modBusDevice;
        modBusDevice = Q_NULLPTR;
    }

    modBusDevice = QModBus::instance()->createSlave("libmodbus",
        static_cast<QModBusDevice::ModBusConnection> (index));

    if (!modBusDevice) {
        ui->connectButton->setDisabled(true);
        ui->errorLabel->setText(tr("Could not create modbus slave."));
    } else {
        connect(modBusDevice, &QModBusMaster::stateChanged,
                this, &MainWindow::onStateChanged);
    }
}

void MainWindow::on_connectButton_clicked()
{
    bool intendToConnect = (modBusDevice->state() == QModBusDevice::UnconnectedState);

    if (modBusDevice && intendToConnect) {
        QModBusRegister reg;
        reg.setRegisterSize(QModBusRegister::Coils, 10);
        reg.setRegisterSize(QModBusRegister::DiscreteInputs, 10);
        reg.setRegisterSize(QModBusRegister::InputRegisters, 10);
        reg.setRegisterSize(QModBusRegister::HoldingRegisters, 10);

        modBusDevice->setMap(reg);

        connect(modBusDevice, &QModBusSlave::slaveWritten,
                this, &MainWindow::updateWidgets);
        connect(modBusDevice, &QModBusSlave::stateChanged,
                this, &MainWindow::onStateChanged);
    }

    ui->errorLabel->setText(QString());

    if (intendToConnect) {
        modBusDevice->setPortName(ui->portEdit->text());
        modBusDevice->setSlaveId(ui->slaveEdit->text().toInt());
        if (modBusDevice->connectDevice()) {
            setupDeviceData();
        } else {
            ui->errorLabel->setText(tr("Connect failed: ") + modBusDevice->errorString());
        }
    } else {
        modBusDevice->disconnectDevice();
    }
}

void MainWindow::onStateChanged(int state)
{
    if (state == QModBusDevice::UnconnectedState)
        ui->connectButton->setText(tr("Connect"));
    else if (state == QModBusDevice::ConnectedState)
        ui->connectButton->setText(tr("Disconnect"));
}

void MainWindow::coilChanged(int id)
{
    QAbstractButton *button = coilButtons.button(id);
    bitChanged(id, QModBusRegister::Coils, button->isChecked());
}

void MainWindow::discreteInputChanged(int id)
{
    QAbstractButton *button = discreteButtons.button(id);
    bitChanged(id, QModBusRegister::DiscreteInputs, button->isChecked());
}

void MainWindow::bitChanged(int id, QModBusRegister::RegisterType table, bool value)
{
    if (!modBusDevice || modBusDevice->state() != QModBusDevice::ConnectedState)
        return;

    if (!modBusDevice->setData(table, id, value))
        ui->errorLabel->setText(tr("Could not set data: ") + modBusDevice->errorString());
}

void MainWindow::setRegister(const QString &value)
{
    if (!modBusDevice || modBusDevice->state() != QModBusDevice::ConnectedState)
        return;

    const QString objectName = QObject::sender()->objectName();
    if (registers.contains(objectName)) {
        bool ok = true;
        const int id = QObject::sender()->property("ID").toInt();
        if (objectName.startsWith(QStringLiteral("inReg")))
            ok = modBusDevice->setData(QModBusRegister::InputRegisters, id, value.toInt(&ok, 16));
        else if (objectName.startsWith(QStringLiteral("holdReg")))
            ok = modBusDevice->setData(QModBusRegister::HoldingRegisters, id, value.toInt(&ok, 16));

        if (!ok)
            ui->errorLabel->setText(tr("Could not set register: ") + modBusDevice->errorString());
    }
}

void MainWindow::updateWidgets(QModBusRegister::RegisterType table, int address, int size)
{
    for (int i = 0; i < size; ++i) {
        quint16 value;
        QString text;
        switch (table) {
        case QModBusRegister::Coils:
            modBusDevice->data(QModBusRegister::Coils, address + i, &value);
            coilButtons.button(address + i)->setChecked(value);
            break;
        case QModBusRegister::HoldingRegisters:
            modBusDevice->data(QModBusRegister::HoldingRegisters, address + i, &value);
            registers.value(QStringLiteral("holdReg_%1").arg(address + i))->setText(text
                .setNum(value, 16));
            break;
        default:
            break;
        }
    }
}

// -- private

void MainWindow::setupDeviceData()
{
    if (!modBusDevice || modBusDevice->state() != QModBusDevice::ConnectedState)
        return;

    for (int i = 0; i < coilButtons.buttons().count(); ++i)
        modBusDevice->setData(QModBusRegister::Coils, i, coilButtons.button(i)->isChecked());

    for (int i = 0; i < discreteButtons.buttons().count(); ++i) {
        modBusDevice->setData(QModBusRegister::DiscreteInputs, i,
            discreteButtons.button(i)->isChecked());
    }

    bool ok;
    foreach (QLineEdit *widget, registers) {
        if (widget->objectName().startsWith(QStringLiteral("inReg"))) {
            modBusDevice->setData(QModBusRegister::InputRegisters, widget->property("ID").toInt(),
                widget->text().toInt(&ok, 16));
        } else if (widget->objectName().startsWith(QStringLiteral("holdReg"))) {
            modBusDevice->setData(QModBusRegister::HoldingRegisters, widget->property("ID").toInt(),
                widget->text().toInt(&ok, 16));
        }
    }
}

void MainWindow::setupWidgetContainers()
{
    coilButtons.setExclusive(false);
    discreteButtons.setExclusive(false);

    QRegularExpression regexp(QStringLiteral("coils_(?<ID>\\d+)"));
    const QList<QCheckBox*> coils = findChildren<QCheckBox*>(regexp);
    foreach (QCheckBox *cbx, coils)
        coilButtons.addButton(cbx, regexp.match(cbx->objectName()).captured("ID").toInt());
    connect(&coilButtons, SIGNAL(buttonClicked(int)), this, SLOT(coilChanged(int)));

    regexp.setPattern(QStringLiteral("discs_(?<ID>\\d+)"));
    const QList<QCheckBox*> discs = findChildren<QCheckBox*>(regexp);
    foreach (QCheckBox *cbx, discs)
        discreteButtons.addButton(cbx, regexp.match(cbx->objectName()).captured("ID").toInt());
    connect(&discreteButtons, SIGNAL(buttonClicked(int)), this, SLOT(discreteInputChanged(int)));

    regexp.setPattern(QLatin1String("(in|hold)Reg_(?<ID>\\d+)"));
    const QList<QLineEdit*> qle = findChildren<QLineEdit*>(regexp);
    foreach (QLineEdit *lineEdit, qle) {
        registers.insert(lineEdit->objectName(), lineEdit);
        lineEdit->setProperty("ID", regexp.match(lineEdit->objectName()).captured("ID").toInt());
        lineEdit->setValidator(new QRegExpValidator(QRegExp(QStringLiteral("[0-9a-f]{0,4}"),
            Qt::CaseInsensitive), this));
        connect(lineEdit, &QLineEdit::textChanged, this, &MainWindow::setRegister);
    }
}
