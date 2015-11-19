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

#include <QtSerialBus/private/qmodbuscommevent_p.h>
#include <QtSerialBus/private/qmodbusdevice_p.h>
#include <QtSerialBus/private/qmodbus_symbols_p.h>
#include <QtSerialBus/qmodbusdataunit.h>
#include <QtSerialBus/qmodbusserver.h>

#include <QtCore/qcontiguouscache.h>
#include <QtCore/qvector.h>

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

    QModbusResponse processReadExceptionStatusRequest(const QModbusRequest &request);
    QModbusResponse processDiagnosticsRequest(const QModbusRequest &request);
    QModbusResponse processGetCommEventCounterRequest(const QModbusRequest &request);
    QModbusResponse processGetCommEventLogRequest(const QModbusRequest &request);
    QModbusResponse processWriteMultipleCoilsRequest(const QModbusRequest &request);
    QModbusResponse processWriteMultipleRegistersRequest(const QModbusRequest &request);
    QModbusResponse processReportServerIdRequest(const QModbusRequest &request);
    QModbusResponse processMaskWriteRegisterRequest(const QModbusRequest &request);
    QModbusResponse processReadWriteMultipleRegistersRequest(const QModbusRequest &request);
    QModbusResponse processReadFifoQueueRequest(const QModbusRequest &request);

    bool isListenOnly() const { return m_forceListenOnlyMode; }
    void storeModbusCommEvent(const QModbusCommEvent &eventByte);

    // Device specific fields to be moved later
    QModbusDataUnitMap m_modbusDataUnitMap;

    int m_slaveAddress = 0;

    quint16 m_deviceBusy = 0x0000;
    uchar m_asciiInputDelimiter; // TODO: Expose through value()|setValue() and use.
    QContiguousCache<quint8> m_commEventLog;

    QVector<quint16> m_counters;
    bool m_forceListenOnlyMode = false; // TODO: Expose through value()|setValue() .
    quint16 m_diagnosticRegister = 0x0000;
    quint16 m_exceptionStatusOffset = 0x0000;
    QHash<int, QVariant> m_userOptions;
    quint8 m_serverIdentifier = 0x0a;
    quint8 m_runIndicatorStatus = 0xff;
    QByteArray m_additionalData = "Qt Modbus Server";
};

QT_END_NAMESPACE

#endif // QMODBUSERVER_P_H
