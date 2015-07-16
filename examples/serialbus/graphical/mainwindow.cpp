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
#include <QCanFrame>

#include <QtCore/qbytearray.h>
#include <QtCore/qvariant.h>
#include <QtCore/qdebug.h>

#include <iostream>

#include <linux/can.h>
#include <linux/can/error.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    currentDevice(0)
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
    canBus = QCanBus::instance();
    if (!canBus)
        return;

    const QList<QByteArray> plugins = canBus->plugins();
    for (int i = 0; i < plugins.size(); i++)
    {
        ui->comboBox->insertItem(i, plugins.at(i));

        if (plugins.at(i) == QStringLiteral("can")) {
            //TODO find way to dynamically detect vcan1
            //TODO potentially instanciate every backend in the plugin - not just first
            const QString type = canBus->availableBackends(plugins.at(i)).first();
            canDevice = canBus->createDevice(plugins.at(i),
                                             type,
                                             QStringLiteral("vcan1"));
            connect(canDevice.data(), &QCanBusDevice::errorOccurred, this, &MainWindow::receiveError);
            if (!canDevice)
                return;
            if (!canDevice->connectDevice())
                return;
            /*QList<QVariant> var;
            QHash<QString, QVariant> hash;
            hash.insert(QStringLiteral("FilterId"), 1);
            hash.insert(QStringLiteral("CanMask"), CAN_EFF_MASK);
            var.append(hash);
            canDevice->setConfigurationParameter(QStringLiteral("CanFilter"), var);*/ //NOTE: Filtering example
            canDevice->setConfigurationParameter(QStringLiteral("ReceiveOwnMessages"), QVariant(1));
            connect(canDevice.data(), &QCanBusDevice::frameReceived,
                    this, &MainWindow::checkMessages);
        } else if (plugins.at(i) == QStringLiteral("dummy")) {
            dummyDevice = canBus->createDevice(plugins.at(i), QString(), QString());
            if (!dummyDevice)
                return;
            dummyDevice->connectDevice();
            connect(dummyDevice.data(), &QCanBusDevice::frameReceived,
                    this, &MainWindow::checkMessages);
        }
    }
    on_connectButton_clicked(); //initialize plugin selection
}

void MainWindow::receiveError(QCanBusDevice::CanBusError error)
{
    switch (error) {
    case QCanBusDevice::ReadError:
    case QCanBusDevice::WriteError:
    case QCanBusDevice::ConnectionError:
    case QCanBusDevice::ConfigurationError:
        qWarning() << canDevice->errorString();
    default:
        break;
    }
}

void MainWindow::checkMessages()
{
    QCanBusDevice *device = qobject_cast<QCanBusDevice *>(sender());

    // for now filter the correct device out
    if (ui->comboBox->itemText(currentDevice) == QStringLiteral("can")
            && device != canDevice)
        device = 0;

    if (ui->comboBox->itemText(currentDevice) == QStringLiteral("dummy")
            && device != dummyDevice)
        device = 0;

    if (!device)
        return;

    const QCanFrame frame = device->readFrame();

    if (frame.payload().isEmpty())
        return;

    qint32 id = frame.frameId();

    if (!frame.hasExtendedFrameFormat() && id > 2047) // 11 bits
        id = 2047;

    QString view;
    if (frame.frameType() == QCanFrame::ErrorFrame) {
        interpretError(view, frame);
    } else {
        view += QLatin1String("Id: ");
        view += QString::number(id, 16);
        view += QLatin1String(" bytes: ");
        view += QString::number(frame.payload().size(), 10);
        view += QLatin1String(" data: ");
        view += frame.payload().data();
    }

    if (frame.frameType() == QCanFrame::RemoteRequestFrame) {
        ui->requestList->addItem(view);
    } else if (frame.frameType() == QCanFrame::ErrorFrame) {
        ui->errorList->addItem(view);
    } else {
        ui->listWidget->addItem(view);
    }
}

void MainWindow::on_sendButton_clicked()
{
    if (!canDevice || !dummyDevice) {
        init();
        if (!canDevice || !dummyDevice)
            return;
    }

    QByteArray writings = ui->lineEdit->displayText().toUtf8();
    ui->lineEdit->clear();

    QCanFrame frame;
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
        frame.setFrameType(QCanFrame::RemoteRequestFrame);
    else if (ui->ERR->checkState())
        frame.setFrameType(QCanFrame::ErrorFrame);
    else
        frame.setFrameType(QCanFrame::DataFrame);

    if (ui->comboBox->itemText(currentDevice) == QStringLiteral("can")) {
        canDevice->writeFrame(frame);
    } else if (ui->comboBox->itemText(currentDevice) == QStringLiteral("dummy")) {
        dummyDevice->writeFrame(frame);
    }
}

void MainWindow::on_connectButton_clicked()
{
    currentDevice = ui->comboBox->currentIndex();
    ui->connectedToLabel->setText("Connected to : " + ui->comboBox->currentText());
    checkMessages();
}

void MainWindow::interpretError(QString &view, const QCanFrame &frame)
{
    switch (frame.frameId()) {
    case CAN_ERR_TX_TIMEOUT:
        view += QStringLiteral("TX timeout\n");
        break;
    case CAN_ERR_LOSTARB:
        view += QStringLiteral("lost arbitration\n");
        break;
    case CAN_ERR_CRTL:
        view += QStringLiteral("controller problems\n");
        break;
    case CAN_ERR_PROT:
        view += QStringLiteral("protocol violations\n");
        break;
    case CAN_ERR_TRX:
        view += QStringLiteral("transceiver status\n");
        break;
    case CAN_ERR_ACK:
        view += QStringLiteral("received no ACK on transmission\n");
        break;
    case CAN_ERR_BUSOFF:
        view += QStringLiteral("bus off\n");
        break;
    case CAN_ERR_BUSERROR:
        view += QStringLiteral("bus error\n");
        break;
    case CAN_ERR_RESTARTED:
        view += QStringLiteral("controller restarted\n");
        break;
    default:
        break;
    }

    const QByteArray payload = frame.payload();
    view += QStringLiteral("arbitration lost in bit: ");
    view += QString::number(payload.at(0), 16);
    view += QStringLiteral("\n");

    switch (payload.at(1)) {
    case CAN_ERR_CRTL_RX_OVERFLOW:
        view += QStringLiteral("RX buffer overflow\n");
        break;
    case CAN_ERR_CRTL_TX_OVERFLOW:
        view += QStringLiteral("TX buffer overflow\n");
        break;
    case CAN_ERR_CRTL_RX_WARNING:
        view += QStringLiteral("reached warning level for RX errors\n");
        break;
    case CAN_ERR_CRTL_TX_WARNING:
        view += QStringLiteral("reached warning level for TX errors\n");
        break;
    case CAN_ERR_CRTL_RX_PASSIVE:
        view += QStringLiteral("reached error passive status RX\n");
        break;
    case CAN_ERR_CRTL_TX_PASSIVE:
        view += QStringLiteral("reached error passive status TX\n");
        break;
    default:
        break;
    }

    switch (payload.at(2)) { //type
    case CAN_ERR_PROT_BIT:
        view += QStringLiteral("single bit error\n");
        break;
    case CAN_ERR_PROT_FORM:
        view += QStringLiteral("frame format error\n");
        break;
    case CAN_ERR_PROT_STUFF:
        view += QStringLiteral("bit stuffing error\n");
        break;
    case CAN_ERR_PROT_BIT0:
        view += QStringLiteral("unable to send dominant bit\n");
        break;
    case CAN_ERR_PROT_BIT1:
        view += QStringLiteral("unable to send recessive bit\n");
        break;
    case CAN_ERR_PROT_OVERLOAD:
        view += QStringLiteral("bus overload\n");
        break;
    case CAN_ERR_PROT_ACTIVE:
        view += QStringLiteral("active error announcement\n");
        break;
    case CAN_ERR_PROT_TX:
        view += QStringLiteral("error occurred on transmission\n");
        break;
    default:
        break;
    }

    switch (payload.at(3)) { //location
    case CAN_ERR_PROT_LOC_SOF:
        view += QStringLiteral("start of frame\n");
        break;
    case CAN_ERR_PROT_LOC_ID28_21:
        view += QStringLiteral("ID bits 28 - 21 (SFF: 10 - 3)\n");
        break;
    case CAN_ERR_PROT_LOC_ID20_18:
        view += QStringLiteral("ID bits 20 - 18 (SFF: 2 - 0 )\n");
        break;
    case CAN_ERR_PROT_LOC_SRTR:
        view += QStringLiteral("substitute RTR (SFF: RTR)\n");
        break;
    case CAN_ERR_PROT_LOC_IDE:
        view += QStringLiteral("identifier extension\n");
        break;
    case CAN_ERR_PROT_LOC_ID17_13:
        view += QStringLiteral("ID bits 17-13\n");
        break;
    case CAN_ERR_PROT_LOC_ID12_05:
        view += QStringLiteral("ID bits 12-5\n");
        break;
    case CAN_ERR_PROT_LOC_ID04_00:
        view += QStringLiteral("ID bits 4-0\n");
        break;
    case CAN_ERR_PROT_LOC_RTR:
        view += QStringLiteral("RTR\n");
        break;
    case CAN_ERR_PROT_LOC_RES1:
        view += QStringLiteral("reserved bit 1\n");
        break;
    case CAN_ERR_PROT_LOC_RES0:
        view += QStringLiteral("reserved bit 0\n");
        break;
    case CAN_ERR_PROT_LOC_DLC:
        view += QStringLiteral("data length code\n");
        break;
    case CAN_ERR_PROT_LOC_DATA:
        view += QStringLiteral("data section\n");
        break;
    case CAN_ERR_PROT_LOC_CRC_SEQ:
        view += QStringLiteral("CRC sequence\n");
        break;
    case CAN_ERR_PROT_LOC_CRC_DEL:
        view += QStringLiteral("CRC delimiter\n");
        break;
    case CAN_ERR_PROT_LOC_ACK:
        view += QStringLiteral("ACK slot\n");
        break;
    case CAN_ERR_PROT_LOC_ACK_DEL:
        view += QStringLiteral("ACK delimiter\n");
        break;
    case CAN_ERR_PROT_LOC_EOF:
        view += QStringLiteral("end of frame\n");
        break;
    case CAN_ERR_PROT_LOC_INTERM:
        view += QStringLiteral("Intermission\n");
        break;
    default:
        break;
    }

    switch (payload.at(4)) {
    case CAN_ERR_TRX_CANH_NO_WIRE:
        view += QStringLiteral("CAN-transceiver CANH no wire\n");
        break;
    case CAN_ERR_TRX_CANH_SHORT_TO_BAT:
        view += QStringLiteral("CAN-transceiver CANH short to bat\n");
        break;
    case CAN_ERR_TRX_CANH_SHORT_TO_VCC:
        view += QStringLiteral("CAN-transceiver CANH short to vcc\n");
        break;
    case CAN_ERR_TRX_CANH_SHORT_TO_GND:
        view += QStringLiteral("CAN-transceiver CANH short to ground\n");
        break;
    case CAN_ERR_TRX_CANL_NO_WIRE:
        view += QStringLiteral("CAN-transceiver CANL no wire\n");
        break;
    case CAN_ERR_TRX_CANL_SHORT_TO_BAT:
        view += QStringLiteral("CAN-transceiver CANL short to bat\n");
        break;
    case CAN_ERR_TRX_CANL_SHORT_TO_VCC:
        view += QStringLiteral("CAN-transceiver CANL short to vcc\n");
        break;
    case CAN_ERR_TRX_CANL_SHORT_TO_GND:
        view += QStringLiteral("CAN-transceiver CANL short to ground\n");
        break;
    case CAN_ERR_TRX_CANL_SHORT_TO_CANH:
        view += QStringLiteral("CAN-transceiver CANL short to CANH\n");
        break;
    default:
        break;
    }
}
