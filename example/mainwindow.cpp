/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
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
    b = QSerialBus::instance();
    if (!b)
        return;

    QList<QByteArray> plugins = b->plugins();
    for (int i = 0; i < plugins.size(); i++)
    {
        backends.insert(i, b->createBackend(plugins.at(i), QStringLiteral("SocketCAN"), QStringLiteral("vcan1")));
        ui->comboBox->insertItem(i, plugins.at(i));

        if (plugins.at(i) == QStringLiteral("can")) {
            canDevice = new QCanBusDevice(backends.at(i), this);
            if (!canDevice)
                return;
            canDevice->open(QIODevice::ReadWrite);
            /*QList<QVariant> var;
            QHash<QString, QVariant> hash;
            hash.insert("FilterId", 1);
            hash.insert("CanMask", CAN_EFF_MASK);
            var.append(hash);
            canDevice->setConfiguration(QPair<QString, QVariant>("CanFilter", var));*/ //NOTE: Filtering example
            canDevice->setConfigurationParameter(QStringLiteral("ReceiveOwnMessages"), QVariant(1));
            connect(canDevice.data(), &QCanBusDevice::readyRead, this, &MainWindow::checkMessages);
        } else if (plugins.at(i) == QStringLiteral("dummy")) {
            dummyDevice = new QBusDummyDevice(backends.at(i), this);
            if (!dummyDevice)
                return;
            dummyDevice->open(QIODevice::ReadWrite);
            connect(dummyDevice.data(), &QBusDummyDevice::readyRead, this, &MainWindow::checkDummyMessages);
        }
    }
    on_connectButton_clicked(); //initialize plugin selection
}

void MainWindow::checkDummyMessages()
{
    if (!dummyDevice) {
        init();
        if (!dummyDevice)
            return;
    }
    if (ui->comboBox->itemText(currentDevice) == QStringLiteral("dummy")) {
        const QByteArray result = dummyDevice->readAll();
        if (!result.isEmpty())
            ui->listWidget->addItem(result);
    }
}

void MainWindow::checkMessages()
{
    if (ui->comboBox->itemText(currentDevice) == QStringLiteral("can")) {
        QCanFrame frame = canDevice->readFrame();

        if (frame.payload().isEmpty())
            return;

        bool EFF = false;
        bool RTR = false;
        bool ERR = false;

        qint32 id = frame.frameId();
        qint8 dataLength = frame.payload().size();

        if (!dataLength)
            return;

        EFF = id & CAN_EFF_FLAG;
        RTR = id & CAN_RTR_FLAG;
        ERR = id & CAN_ERR_FLAG;

        //removes possible flags from id
        id &= ~(CAN_EFF_FLAG|CAN_RTR_FLAG|CAN_ERR_FLAG);

        if (!EFF && id > 2047) //11 bits
            id = 2047;

        QString view;
        if (ERR) {
            frame.setFrameId(id);
            interpretError(view, frame);
        } else {
            view += QLatin1String("Id: ");
            view += QString::number(id, 16);
            view += QLatin1String(" bytes: ");
            view += QString::number(dataLength, 10);
            view += QLatin1String(" data: ");
            view += frame.payload().data();
        }

        if (RTR) {
            ui->requestList->addItem(view);
        } else if (ERR) {
            ui->errorList->addItem(view);
        } else {
            ui->listWidget->addItem(view);
        }
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
    if (ui->comboBox->itemText(currentDevice) == QStringLiteral("can")) {
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

        if (ui->EFF->checkState())
            id |= CAN_EFF_FLAG;
        if (ui->RTR->checkState())
            id |= CAN_RTR_FLAG;
        if (ui->ERR->checkState())
            id |= CAN_ERR_FLAG;
        frame.setFrameId(id);

        canDevice->writeFrame(frame);
    } else if (ui->comboBox->itemText(currentDevice) == QStringLiteral("dummy")) {
        dummyDevice->write(writings);
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
