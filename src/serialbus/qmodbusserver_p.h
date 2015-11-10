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
#include "qmodbus_symbols_p.h"

#include <QtCore/qvector.h>

#include <QtCore/qcontiguouscache.h>

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
    enum Counter {
        CommEvent = 0x0001,
        BusMessage = Diagnostics::ReturnBusMessageCount,
        BusCommunicationError = Diagnostics::ReturnBusCommunicationErrorCount,
        BusExceptionError = Diagnostics::ReturnBusExceptionErrorCount,
        ServerMessage = Diagnostics::ReturnServerMessageCount,
        ServerNoResponse = Diagnostics::ReturnServerNoResponseCount,
        ServerNAK = Diagnostics::ReturnServerNAKCount,
        ServerBusy = Diagnostics::ReturnServerBusyCount,
        BusCharacterOverrun = Diagnostics::ReturnBusCharacterOverrunCount
    };

    QModbusServerPrivate()
        : m_commEventLog(64)
        , m_counters(20, 0u)
    {
    }

    bool setMap(const QModbusDataUnitMap &map);

    void resetCommunicationCounters() { m_counters.fill(0u, m_counters.size()); }
    void incrementCounter(QModbusServerPrivate::Counter counter) { m_counters[counter]++; }

    QModbusResponse processRequest(const QModbusPdu &request);

    QModbusResponse processReadCoilsRequest(const QModbusRequest &request);
    QModbusResponse processReadDiscreteInputsRequest(const QModbusRequest &request);
    QModbusResponse readBits(const QModbusPdu &request, QModbusDataUnit::RegisterType unitType);

    QModbusResponse processReadHoldingRegistersRequest(const QModbusRequest &request);
    QModbusResponse processReadInputRegistersRequest(const QModbusRequest &request);
    QModbusResponse readBytes(const QModbusPdu &request, QModbusDataUnit::RegisterType unitType);

    QModbusResponse processWriteSingleCoilRequest(const QModbusRequest &request);
    QModbusResponse processWriteSingleRegisterRequest(const QModbusRequest &request);
    QModbusResponse writeSingle(const QModbusPdu &request, QModbusDataUnit::RegisterType unitType);

    QModbusResponse processReadExceptionStatus(const QModbusRequest &request);
    QModbusResponse processDiagnostics(const QModbusRequest &request);
    QModbusResponse processGetCommEventCounter(const QModbusRequest &request);
    QModbusResponse processGetCommEventLog(const QModbusRequest &request);
    QModbusResponse processWriteMultipleCoilsRequest(const QModbusRequest &request);
    QModbusResponse processWriteMultipleRegistersRequest(const QModbusRequest &request);
    QModbusResponse processMaskWriteRegister(const QModbusRequest &request);
    QModbusResponse processReadWriteMultipleRegistersRequest(const QModbusRequest &request);
    QModbusResponse processReadFifoQueue(const QModbusRequest &request);

private:
    bool restartCommunicationsOption(bool clearLog);
    void storeEvent(quint8 eventByte);

    // Device specific fields to be moved later
    QModbusDataUnit m_discreteInputs;
    QModbusDataUnit m_coils;
    QModbusDataUnit m_inputRegisters;
    QModbusDataUnit m_holdingRegisters;

    int m_slaveAddress = 0;

    // Modbus Communication fields to stay
    bool m_continueOnError = true; //TODO hook into server implementations
    quint16 m_deviceBusy = 0x0000; // TODO not taken into account yet
    uchar m_asciiInputDelimiter; // TODO not taken into account yet
    QContiguousCache<quint8> m_commEventLog; // TODO not taken into account yet

    QVector<quint16> m_counters;
    bool m_forceListenOnlyMode = false;
    quint16 m_diagnosticRegister = 0x0000;
    quint16 m_exceptionStatusOffset = 0x0000;
};

QT_END_NAMESPACE

#endif // QMODBUSERVER_P_H
