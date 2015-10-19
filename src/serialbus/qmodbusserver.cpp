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

#include "qmodbusserver.h"
#include "qmodbusserver_p.h"

#include <bitset>

QT_BEGIN_NAMESPACE

/*!
    \class QModbusServer
    \inmodule QtSerialBus
    \since 5.6

    \brief The QModbusServer class is the interface to receive and process Modbus requests.

    Modbus networks can have multiple Modbus servers. Modbus Servers are read/written by a
    Modbus client represented by \l QModbusClient. QModbusServer communicates with a Modbus
    backend, providing users with a convenient API.

 */

/*!
    Constructs a Modbus server with the specified \a parent.
 */
QModbusServer::QModbusServer(QObject *parent) :
    QModbusDevice(*new QModbusServerPrivate, parent)
{
}

/*!
    \internal
*/
QModbusServer::~QModbusServer()
{
}

/*!
    \internal
 */
QModbusServer::QModbusServer(QModbusServerPrivate &dd, QObject *parent) :
    QModbusDevice(dd, parent)
{
}

/*!
    Sets the registered map structure for requests from other ModBus clients to \a map.
    The register values are initialized with zero. Returns \c true on success; otherwise \c false.

    If this function is not called before connecting, a default register with zero
    entries is setup.

    \note Calling this function discards any register value that was previously set.
 */
bool QModbusServer::setMap(const QModbusDataUnitMap &map)
{
    return d_func()->setMap(map);
}

/*!
    Sets the slave id for this modbus server instance.

    \sa slaveId()
*/
void QModbusServer::setSlaveId(int id)
{
    Q_D(QModbusServer);
    d->m_slaveId = id;
}

/*!
    Returns the slave id of this modbus server instance.

    The purpose of the slave id is to identify the modbus server
    that is responsible for a client request.
*/
int QModbusServer::slaveId() const
{
    Q_D(const QModbusServer);

    return d->m_slaveId;
}

/*!
    Reads data stored in the Modbus server. A Modbus server has four tables (\a table) and each
    have a unique \a address field, which is used to read \a data from the desired field.
    See QModbusDataUnit::RegisterType for more information about the different tables.
    Returns \c false if address is outside of the map range or the register type is not even defined.

    \sa QModbusDataUnit::RegisterType, setData()
 */
bool QModbusServer::data(QModbusDataUnit::RegisterType table,
                         quint16 address, quint16 *data) const
{
    Q_D(const QModbusServer);
    if (!data)
        return false;

    const QModbusDataUnit *unit;

    switch (table) {
    case QModbusDataUnit::Invalid:
        return false;
    case QModbusDataUnit::DiscreteInputs:
        unit = &(d->m_discreteInputs);
        break;
    case QModbusDataUnit::Coils:
        unit = &(d->m_coils);
        break;
    case QModbusDataUnit::InputRegisters:
        unit = &(d->m_inputRegisters);
        break;
    case QModbusDataUnit::HoldingRegisters:
        unit = &(d->m_holdingRegisters);
        break;
    }

    if (!unit->isValid()
        || address < unit->startAddress()
        || address >= (unit->startAddress() + unit->valueCount())) {
        return false;
    }

    *data = unit->value(address);

    return true;
}

/*!
    Returns the values in the register range given by \a newData.

    \a newData must provide a valid register type, start address
    and valueCount. The returned \a newData will contain the register values
    associated with the given range.

    If \a newData contains a valid register type but a negative start address
    the entire register map is returned and \a newData appropriately sized.
 */
bool QModbusServer::data(QModbusDataUnit *newData) const
{
    Q_D(const QModbusServer);

    if (!newData)
        return false;

    const QModbusDataUnit *current;

    switch (newData->registerType()) {
    case QModbusDataUnit::Invalid:
        return false;
    case QModbusDataUnit::DiscreteInputs:
        current = &(d->m_discreteInputs);
        break;
    case QModbusDataUnit::Coils:
        current = &(d->m_coils);
        break;
    case QModbusDataUnit::InputRegisters:
        current = &(d->m_inputRegisters);
        break;
    case QModbusDataUnit::HoldingRegisters:
        current = &(d->m_holdingRegisters);
        break;
    }

    if (newData->startAddress() < 0) { //return enire map for given type
        *newData = *current;
        return true;
    }

    //check range start is within internal map range
    int internalRangeEndAddress = current->startAddress() + current->valueCount() - 1;
    if (newData->startAddress() < current->startAddress()
        || newData->startAddress() > internalRangeEndAddress) {
        return false;
    }

    //check range end is within internal map range
    int rangeEndAddress = newData->startAddress() + newData->valueCount() - 1;
    if (rangeEndAddress < current->startAddress()
        || rangeEndAddress > internalRangeEndAddress) {
        return false;
    }

    newData->setValues(current->values().mid(newData->startAddress(),
                                             newData->valueCount()));
    return true;
}

/*!
    Writes data to the Modbus server. A Modbus server has four tables (\a table) and each have a
    unique \a address field, which is used to write \a data to the desired field.
    Returns \c false if address outside of the map range.

    If the call was successful the \l dataWritten() signal is emitted. Note that
    the signal is not emitted when \a data has not changed. Nevertheless this function
    returns \c true in such cases.

    \sa QModbusDataUnit::RegisterType, data(), dataWritten()
 */

bool QModbusServer::setData(QModbusDataUnit::RegisterType table,
                            quint16 address, quint16 data)
{
    Q_D(QModbusServer);
    QModbusDataUnit* unit;

    switch (table) {
    case QModbusDataUnit::Invalid:
        return false;
    case QModbusDataUnit::DiscreteInputs:
        unit = &(d->m_discreteInputs);
        break;
    case QModbusDataUnit::Coils:
        unit = &(d->m_coils);
        break;
    case QModbusDataUnit::InputRegisters:
        unit = &(d->m_inputRegisters);
        break;
    case QModbusDataUnit::HoldingRegisters:
        unit = &(d->m_holdingRegisters);
        break;
    }

    if (!unit->isValid()
        || address < unit->startAddress()
        || address >= (unit->startAddress() + unit->valueCount())) {
        return false;
    }

    if (unit->value(address) != data) {
        unit->setValue(address, data);
        emit dataWritten(table, address, 1);
    }

    return true;
}

/*!
    Writes \a newData to the Modbus server map.
    Returns \c false if the \a newData range is outside of the map range.

    If the call was successful the \l dataWritten() signal is emitted. Note that
    the signal is not emitted when the addressed register has not changed. This
    may happen when \a newData contains exactly the same values as the
    register already. Nevertheless this function returns \c true in such cases.

    \sa data()
 */

bool QModbusServer::setData(const QModbusDataUnit &newData)
{
    Q_D(QModbusServer);

    QModbusDataUnit *current = Q_NULLPTR;

    switch (newData.registerType()) {
    case QModbusDataUnit::Invalid:
        return false;
    case QModbusDataUnit::DiscreteInputs:
        current = &(d->m_discreteInputs);
        break;
    case QModbusDataUnit::Coils:
        current = &(d->m_coils);
        break;
    case QModbusDataUnit::InputRegisters:
        current = &(d->m_inputRegisters);
        break;
    case QModbusDataUnit::HoldingRegisters:
        current = &(d->m_holdingRegisters);
        break;
    }

    if (!current->isValid())
        return false;

    //check range start is within internal map range
    int internalRangeEndAddress = current->startAddress() + current->valueCount() - 1;
    if (newData.startAddress() < current->startAddress()
        || newData.startAddress() > internalRangeEndAddress) {
        return false;
    }

    //check range end is within internal map range
    int rangeEndAddress = newData.startAddress() + newData.valueCount() - 1;
    if (rangeEndAddress < current->startAddress()
        || rangeEndAddress > internalRangeEndAddress) {
        return false;
    }

    bool changeRequired = false;
    for (int i = newData.startAddress();
         i < newData.startAddress() + int(newData.valueCount()); i++) {
        quint16 newValue = newData.value(i - newData.startAddress());
        if (current->value(i) != newValue) {
            current->setValue(i, newValue);
            changeRequired = true;
        }
    }

    if (changeRequired)
        emit dataWritten(newData.registerType(),
                         newData.startAddress(),
                         newData.valueCount());

    return true;
}

/*!
    \fn int QModbusServer::slaveId() const
    Multiple Modbus devices can be connected together on the same physical link.
    Slave id is a unique identifier that each Modbus server must have, and it is used
    to filter out incoming messages.

    Returns slave id.

    \sa setSlaveId()
 */

/*!
    \fn void QModbusServer::setSlaveId(int id)
    Multiple Modbus devices can be connected together on the same physical link.
    So it is important that each server is identified by a unique id.

    Sets \a id as slave id.

    \sa slaveId()
 */

/*!
    \fn void QModbusServer::dataWritten(QModbusDataUnit::RegisterType table, int address, int size)

    This signal is emitted when a Modbus client has written one or more fields of data to the
    Modbus server. The signal contains information about the fields that were written:
    \list
     \li \a register type that was written,
     \li \a address of the first field that was written,
     \li and \a amount of consecutive fields that were written starting from \a address.
    \endlist

    The signal is not emitted when the to-be-written fields have not changed
    due to no change in value.
 */

/*
    Processes a Modbus client \a request and returns a Modbus response.
*/
QModbusResponse QModbusServer::processRequest(const QModbusPdu &request)
{
    return d_func()->processRequest(request);
}

/*
    To be implemented by custom Modbus server implementation. The default implementation returns
    a \c QModbusExceptionResponse with the \a request function code and error code set to illegal
    function.
*/
QModbusResponse QModbusServer::processPrivateModbusRequest(const QModbusPdu &request)
{
    return QModbusExceptionResponse(request.functionCode(),
        QModbusExceptionResponse::IllegalFunction);
}


// -- QModbusServerPrivate

bool QModbusServerPrivate::setMap(const QModbusDataUnitMap &map)
{
    m_discreteInputs = map.value(QModbusDataUnit::DiscreteInputs);
    m_coils = map.value(QModbusDataUnit::Coils);
    m_inputRegisters = map.value(QModbusDataUnit::InputRegisters);
    m_holdingRegisters = map.value(QModbusDataUnit::HoldingRegisters);
    return true;
}

/*
    TODO: implement
*/
QModbusResponse QModbusServerPrivate::processRequest(const QModbusPdu &request)
{
    switch (request.functionCode()) {
    case QModbusRequest::ReadCoils:
        return processReadCoilsRequest(request);
    case QModbusRequest::ReadDiscreteInputs:
        return processReadDiscreteInputsRequest(request);
    case QModbusRequest::ReadHoldingRegisters:
    case QModbusRequest::ReadInputRegisters:
        return processReadInputRegistersRequest(request);
    case QModbusRequest::WriteSingleCoil:
        return processWriteSingleCoilRequest(request);
    case QModbusRequest::WriteSingleRegister:
        return processWriteSingleRegisterRequest(request);
    case QModbusRequest::ReadExceptionStatus:
    case QModbusRequest::Diagnostics:
    case QModbusRequest::GetCommEventCounter:
    case QModbusRequest::GetCommEventLog:
    case QModbusRequest::WriteMultipleCoils:
        return processWriteMultipleCoilsRequest(request);
    case QModbusRequest::WriteMultipleRegisters:
    case QModbusRequest::ReportServerId:
    case QModbusRequest::ReadFileRecord:
    case QModbusRequest::WriteFileRecord:
    case QModbusRequest::MaskWriteRegister:
    case QModbusRequest::ReadWriteMultipleRegisters:
    case QModbusRequest::ReadFifoQueue:
    case QModbusRequest::EncapsulatedInterfaceTransport:
    default:
        break;
    }
    return q_func()->processPrivateModbusRequest(request);
}

QModbusResponse QModbusServerPrivate::processReadCoilsRequest(const QModbusRequest &request)
{
    // request data size corrupt
    if (request.dataSize() != 4) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataValue);
    }

    quint16 address, numberOfCoils;
    request.decodeData(&address, &numberOfCoils);

    if ((numberOfCoils < 0x0001) || (numberOfCoils > 0x07D0)) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataValue);
    }

    // Get the requested range out of the registers.
    QModbusDataUnit coils(QModbusDataUnit::Coils, address, numberOfCoils);
    if (!q_func()->data(&coils)) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataAddress);
    }

    quint8 byteCount = numberOfCoils / 8;
    if ((numberOfCoils % 8) != 0) {
        byteCount += 1;
        // If the range is not a multiple of 8, resize.
        coils.setValueCount(byteCount * 8);
    }

    address = 0; // The data range now starts with zero.
    QVector<quint8> bytes;
    for (int i = 0; i < byteCount; ++i) {
        std::bitset<8> byte;
        // According to the spec: If the returned output quantity is not a multiple of
        // eight, the remaining bits in the final data byte will be padded with zeros.
        for (int currentBit = 0; currentBit < 8; ++currentBit)
            byte[currentBit] = coils.value(address++); // The padding happens inside value().
        bytes.append(static_cast<quint8> (byte.to_ulong()));
    }

    // TODO: Increase message counters when they are implemented
    return QModbusResponse(request.functionCode(), byteCount, bytes);
}

QModbusResponse QModbusServerPrivate::processReadDiscreteInputsRequest(
        const QModbusRequest &request)
{
    // request data size corrupt
    if (request.dataSize() != 4) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataValue);
    }

    quint16 address, numberOfInputs;
    request.decodeData(&address, &numberOfInputs);

    if ((numberOfInputs < 0x0001) || (numberOfInputs > 0x07D0)) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataValue);
    }

    // Get the requested range out of the registers.
    QModbusDataUnit inputs(QModbusDataUnit::DiscreteInputs, address, numberOfInputs);
    if (!q_func()->data(&inputs)) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataAddress);
    }

    quint8 byteCount = numberOfInputs / 8;
    if ((numberOfInputs % 8) != 0) {
        byteCount += 1;
        // If the range is not a multiple of 8, resize.
        inputs.setValueCount(byteCount * 8);
    }

    address = 0; // The data range now starts with zero.
    QVector<quint8> bytes;
    for (int i = 0; i < byteCount; ++i) {
        std::bitset<8> byte;
        // According to the spec: If the returned input quantity is not a multiple of
        // eight, the remaining bits in the final data byte will be padded with zeros.
        for (int currentBit = 0; currentBit < 8; ++currentBit)
            byte[currentBit] = inputs.value(address++); // The padding happens inside value().
        bytes.append(static_cast<quint8> (byte.to_ulong()));
    }

    // TODO: Increase message counters when they are implemented
    return QModbusResponse(request.functionCode(), byteCount, bytes);
}

QModbusResponse QModbusServerPrivate::processReadInputRegistersRequest(
    const QModbusRequest &request)
{
    // request data size corrupt
    if (request.dataSize() != 4) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataValue);
    }

    quint16 address, numberOfRegisters;
    request.decodeData(&address, &numberOfRegisters);

    if ((numberOfRegisters < 0x0001) || (numberOfRegisters > 0x007D)) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataValue);
    }

    // Get the requested range out of the registers.
    QModbusDataUnit registers(QModbusDataUnit::InputRegisters, address, numberOfRegisters);
    if (!q_func()->data(&registers)) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataAddress);
    }

    // TODO: Increase message counters when they are implemented
    return QModbusResponse(request.functionCode(), quint8(numberOfRegisters * 2), registers.values());
}

QModbusResponse QModbusServerPrivate::processWriteSingleCoilRequest(const QModbusRequest &request)
{
    // request data size corrupt
    if (request.dataSize() != 4) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataValue);
    }

    quint16 address, value;
    request.decodeData(&address, &value);

    if ((value != 0x0000) && (value != 0xFF00)) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataValue);
    }

    // Get the requested register.
    QModbusDataUnit coils(QModbusDataUnit::Coils, address, 1u);
    if (!q_func()->data(&coils)) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataAddress);
    }

    // Since we picked the coil at address, data range
    // is now 1 and therefore index needs to be 0.
    coils.setValue(0, value);

    if (!q_func()->setData(coils)) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::ServerDeviceFailure);
    }

    // - TODO: Increase message counters when they are implemented
    return QModbusResponse(request.functionCode(), address, value);
}

QModbusResponse QModbusServerPrivate::processWriteSingleRegisterRequest(const QModbusRequest &request)
{
    if (request.dataSize() != 4) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataValue);
    }

    quint16 address, value;
    request.decodeData(&address, &value);

    // Get the requested register.
    QModbusDataUnit registers(QModbusDataUnit::HoldingRegisters, address, 1u);
    if (!q_func()->data(&registers)) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataAddress);
    }

    // Since we picked the coil at address, data range
    // is now 1 and therefore index needs to be 0.
    registers.setValue(0, value);

    if (!q_func()->setData(registers)) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::ServerDeviceFailure);
    }

    // - TODO: Increase message counters when they are implemented
    return QModbusResponse(request.functionCode(), address, value);
}

QModbusResponse QModbusServerPrivate::processWriteMultipleCoilsRequest(const QModbusRequest &request)
{
    // request data size corrupt: 5 header + 1 minimum data byte required
    if (request.dataSize() < 6) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataValue);
    }

    quint16 address, numberOfCoils;
    quint8 byteCount;
    request.decodeData(&address, &numberOfCoils, &byteCount);

    // byte count does not match number of data bytes following
    if (byteCount != (request.dataSize() - 5 )) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataValue);
    }

    quint8 expectedBytes = numberOfCoils / 8;
    if ((numberOfCoils % 8) != 0)
        expectedBytes += 1;

    if ((numberOfCoils < 0x0001) || (numberOfCoils > 0x07B0) || (expectedBytes != byteCount)) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataValue);
    }

    // Get the requested range out of the registers.
    QModbusDataUnit coils(QModbusDataUnit::Coils, address, numberOfCoils);
    if (!q_func()->data(&coils)) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataAddress);
    }

    QVector<std::bitset<8>> bytes;
    const QByteArray payload = request.data().mid(5);
    for (qint32 i = payload.size() - 1; i >= 0; --i)
        bytes.append(quint8(payload[i]));

    // Since we picked the coils at start address, data
    // range is numberOfCoils and therefore index too.
    quint16 coil = numberOfCoils;
    qint32 currentBit = 8 - ((byteCount * 8) - numberOfCoils);
    foreach (const auto &currentByte, bytes) {
        for (currentBit -= 1; currentBit >= 0; --currentBit)
            coils.setValue(--coil, currentByte[currentBit]);
        currentBit = 8;
    }

    if (!q_func()->setData(coils)) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::ServerDeviceFailure);
    }

    // - TODO: Increase message counters when they are implemented
    return QModbusResponse(request.functionCode(), address, numberOfCoils);
}

QT_END_NAMESPACE
