/****************************************************************************
**
** Copyright (C) 2021 Andre Hartmann <aha_1980@gmx.de>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the QtSerialBus module.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
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

#include "canbusdeviceinfobox.h"
#include "ui_canbusdeviceinfobox.h"

#include <QCanBusDeviceInfo>

CanBusDeviceInfoBox::CanBusDeviceInfoBox(QWidget *parent) :
    QGroupBox(parent),
    m_ui(new Ui::CanBusDeviceInfoBox)
{
    m_ui->setupUi(this);

    auto setReadOnlyAndCompact = [](QCheckBox *box) {
        box->setAttribute(Qt::WA_TransparentForMouseEvents);
        box->setFocusPolicy(Qt::NoFocus);
        box->setStyleSheet("margin-top:0; margin-bottom:0;");
    };
    setReadOnlyAndCompact(m_ui->isVirtual);
    setReadOnlyAndCompact(m_ui->isFlexibleDataRateCapable);
}

CanBusDeviceInfoBox::~CanBusDeviceInfoBox()
{
    delete m_ui;
}

void CanBusDeviceInfoBox::clear()
{
    m_ui->pluginLabel->clear();
    m_ui->nameLabel->clear();
    m_ui->descriptionLabel->clear();
    m_ui->serialNumberLabel->clear();
    m_ui->aliasLabel->clear();
    m_ui->channelLabel->clear();
    m_ui->isVirtual->setChecked(false);
    m_ui->isFlexibleDataRateCapable->setChecked(false);
}

void CanBusDeviceInfoBox::setDeviceInfo(const QCanBusDeviceInfo &info)
{
    m_ui->pluginLabel->setText(tr("Plugin: %1").arg(info.plugin()));
    m_ui->nameLabel->setText(tr("Name: %1").arg(info.name()));
    m_ui->descriptionLabel->setText(info.description());
    QString serialNumber = info.serialNumber();
    if (serialNumber.isEmpty())
        serialNumber = tr("n/a");
    m_ui->serialNumberLabel->setText(tr("Serial: %1").arg(serialNumber));
    QString alias = info.alias();
    if (alias.isEmpty())
        alias = tr("n/a");
    m_ui->aliasLabel->setText(tr("Alias: %1").arg(alias));
    m_ui->channelLabel->setText(tr("Channel: %1").arg(info.channel()));
    m_ui->isVirtual->setChecked(info.isVirtual());
    m_ui->isFlexibleDataRateCapable->setChecked(info.hasFlexibleDataRate());
}
