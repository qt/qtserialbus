/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
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

#ifndef QMODBUSERVER_P_H
#define QMODBUSERVER_P_H

#include "qmodbusserver.h"
#include "qmodbusdevice_p.h"
#include "qmodbusdataunit.h"

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class QModbusServerPrivate : public QModbusDevicePrivate
{
    Q_DECLARE_PUBLIC(QModbusServer)

public:
    QModbusServerPrivate() Q_DECL_EQ_DEFAULT;

    bool setMap(const QModbusDataUnitMap &map);

    QModbusResponse processRequest(const QModbusPdu &request);
    QModbusResponse processReadCoilsRequest(const QModbusRequest &request);
    QModbusResponse processReadDiscreteInputsRequest(const QModbusRequest &request);
    QModbusResponse processReadHoldingRegistersRequest(const QModbusRequest &request);
    QModbusResponse processReadInputRegistersRequest(const QModbusRequest &request);
    QModbusResponse processWriteSingleCoilRequest(const QModbusRequest &request);
    QModbusResponse processWriteSingleRegisterRequest(const QModbusRequest &request);
    QModbusResponse processDiagnostics(const QModbusRequest &request);
    QModbusResponse processWriteMultipleCoilsRequest(const QModbusRequest &request);
    QModbusResponse processWriteMultipleRegistersRequest(const QModbusRequest &request);
    QModbusResponse processReadWriteMultipleRegistersRequest(const QModbusRequest &request);

private:
    bool restartCommunicationsOption(bool clearLog);
    void resetCommunicationCounters();
    void storeEvent(quint8 eventByte);

    // Device specific fields to be moved later
    QModbusDataUnit m_discreteInputs;
    QModbusDataUnit m_coils;
    QModbusDataUnit m_inputRegisters;
    QModbusDataUnit m_holdingRegisters;

    quint16 m_diagnosticRegister = 0;

    int m_slaveAddress = 0;

    // Modbus Communication fields to stay
    bool m_listenOnly = false; //TODO not enforced yet
    bool m_continueOnError = true; //TODO hook into server implementations
    bool m_deviceBusy = false; //TODO required?
    uchar m_asciiInputDelimiter; // TODO not taken into account yet
    quint16 m_commEventCounter = 0;
    QByteArray m_commEventLog;

    quint16 m_busMessageCounter = 0;
    quint16 m_crcErrorCounter = 0;
    quint16 m_exceptionErrorCounter = 0;
    quint16 m_serverMessageCounter = 0;
    quint16 m_serverNoResponseCounter = 0;
    quint16 m_serverNAKCounter = 0;
    quint16 m_serverDeviceBusyCounter = 0;
    quint16 m_serverCharacterOverrunCounter = 0;
};

QT_END_NAMESPACE

#endif // QMODBUSERVER_P_H

