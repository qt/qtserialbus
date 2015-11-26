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

#include "qmodbuspdu.h"

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

namespace Private {

enum struct Type {
    Request,
    Response
};

static int minimumDataSize(QModbusPdu::FunctionCode code, Type type)
{
    Q_ASSERT(!(code & 0x80));
    switch (code) {
    case QModbusPdu::ReadCoils:
    case QModbusPdu::ReadDiscreteInputs:
        return type == Type::Request ? 4 : 2;
    case QModbusPdu::WriteSingleCoil:
    case QModbusPdu::WriteSingleRegister:
        return 4;
    case QModbusPdu::ReadHoldingRegisters:
    case QModbusPdu::ReadInputRegisters:
        return type == Type::Request ? 4 : 3;
    case QModbusPdu::ReadExceptionStatus:
        return type == Type::Request ? 0 : 1;
    case QModbusPdu::Diagnostics:
        return 4;
    case QModbusPdu::GetCommEventCounter:
        return type == Type::Request ? 0 : 4;
    case QModbusPdu::GetCommEventLog:
        return type == Type::Request ? 0 : 8;
    case QModbusPdu::WriteMultipleCoils:
        return type == Type::Request ? 6 : 4;
    case QModbusPdu::WriteMultipleRegisters:
        return type == Type::Request ? 7 : 4;
    case QModbusPdu::ReportServerId:
        return type == Type::Request ? 0 : 3;
    case QModbusPdu::ReadFileRecord:
        return type == Type::Request ? 8 : 5;
    case QModbusPdu::WriteFileRecord:
        return 10;
    case QModbusPdu::MaskWriteRegister:
        return 6;
    case QModbusPdu::ReadWriteMultipleRegisters:
        return type == Type::Request ? 11 : 3;
    case QModbusPdu::ReadFifoQueue:
        return type == Type::Request ? 2 : 6;
    case QModbusPdu::EncapsulatedInterfaceTransport:
        return 2;
    case QModbusPdu::Invalid:
    case QModbusPdu::UndefinedFunctionCode:
        return -1;
    }
    return -1;
}

}   // namespace Private

/*!
    \class QModbusPdu
    \inmodule QtSerialBus
    \since 5.6

    \brief QModbusPdu is a abstract container class containing the function code and
    payload that is stored inside a Modbus ADU.

    The class provides access to the raw Modbus protocol packets as defined by
    the  Modbus Application Protocol Specification 1.1b.
*/

/*!
    \enum QModbusPdu::ExceptionCode

    This enum describes all the possible error conditions as defined by Modbus Exception Codes.
    They are set by the server after checking the appropriate error conditions in the reply to a
    request and must be decoded by the client to operate on the exception code.

    \value IllegalFunction                      Function code is not supported by device.
    \value IllegalDataAddress                   The received data address in the query is not an
                                                allowable address for the Modbus server.
    \value IllegalDataValue                     The contained value in the request data field is
                                                not an allowable value for the Modbus server.
    \value ServerDeviceFailure                  An irrecoverable error occurred while the server
                                                was attempting to perform the requested action.
    \value Acknowledge                          Specialized use in conjunction with programming
                                                commands.
    \value ServerDeviceBusy                     The server is engaged in processing a long duration
                                                program command.
    \value NegativeAcknowledge                  The server cannot perform the program function
                                                received in the query. This code is returned for an
                                                unsuccessful programming request. The client should
                                                request diagnostic or error information from the
                                                server.
    \value MemoryParityError                    Indicates that the extended file area failed to
                                                pass a consistency check. Used in conjunction with
                                                function codes 20 and 21. The exception code does
                                                not refer to any parity settings of the
                                                transmission line but only to the servers' internal
                                                memory of file records.
    \value GatewayPathUnavailable               Indicates that the gateway was unable to allocate
                                                an internal communication path from the input port
                                                to the output port for processing the request.
    \value GatewayTargetDeviceFailedToRespond   Indicates that no response was obtained from the
                                                target device behind a gateway.
                                                Usually this means the target device is not online
                                                on the network.
    \value ExtendedException                    This is an extended exception as per Modbus
                                                specification. Generally this code is used to
                                                describe an exception that is otherwise further
                                                described.
*/

/*!
    \enum QModbusPdu::FunctionCode

    Defines the function code and the implicit type of action required by the server. Not all
    Modbus devices can handle the same set of function codes.

    \value Invalid                          Set by the default constructor, do not use.
    \value ReadCoils                        Requests the status of one or more coils from a device.
    \value ReadDiscreteInputs               Requests the status of one or more input registers from
                                            a device.
    \value ReadHoldingRegisters             Requests the status of one or more holding register
                                            values from a device.
    \value ReadInputRegisters               Requests the status of one or more input register
                                            values from a device.
    \value WriteSingleCoil                  Requests to write a single coil on a device.
    \value WriteSingleRegister              Requests to write a single holding register on a device.
    \value ReadExceptionStatus              Requests the status of the eight Exception Status
                                            outputs on a device.
    \value Diagnostics                      Used to provide a series of tests for checking the
                                            client server communication system, or checking internal
    \value GetCommEventCounter              Requests a status word and an event count from the
                                            device's communication event counter.
    \value GetCommEventLog                  Requests a status word, event count, message count,
                                            and a field of event bytes from a device.
    \value WriteMultipleCoils               Requests to write one or more coils on a device.
    \value WriteMultipleRegisters           Requests to write one or more holding registers on a
                                            device.
    \value ReportServerId                   Requests the description of the type, the current
                                            status, and other information specific to a device.
    \value ReadFileRecord                   Requests a file record read.
    \value WriteFileRecord                  Requests a file record write.
    \value MaskWriteRegister                Requests to modify the contents of a specified holding
                                            register using a combination of an AND or OR mask, and
                                            the register's current contents.
    \value ReadWriteMultipleRegisters       Requests the status of one or more holding register and
                                            at the same time to write one or more holding registers
                                            on a device.
    \value ReadFifoQueue                    Requests to read the contents of a First-In-First-Out
                                            (FIFO) queue of register in a remote device.
    \value EncapsulatedInterfaceTransport   Please refer to Annex A of the Modbus specification.
    \value UndefinedFunctionCode            Do not use.
*/

/*!
    \fn QModbusPdu::QModbusPdu()

    Constructs an invalid QModbusPdu.
*/

/*!
    \fn QModbusPdu::~QModbusPdu()

    Destroys a QModbusPdu.
*/

/*!
    \fn QModbusPdu::QModbusPdu(const QModbusPdu &other)

    Constructs a QModbusPdu that is a copy of \a other.

*/

/*!
    \fn QModbusPdu& QModbusPdu::operator=(const QModbusPdu& other)

    Makes a copy of the \a other and assigns it to this QModbusPdu object.
*/

/*!
    \fn QModbusPdu::QModbusPdu(FunctionCode code, const QByteArray &data)

    Constructs a QModbusPdu with function code set to \a code and payload set to \a data.
    The data is expected to be stored in big-endian byte order already.
*/

/*!
    \internal
    \fn QModbusPdu::QModbusPdu(FunctionCode code, Args... data)

    Constructs a QModbusPdu with function code set to \a code and payload set to \a data.
    The data is converted and stored in big-endian byte order.

    \note Usage should be limited the POD types only. This is because \c QDataStream stream
        operators will not only append raw data, but also e.g. size, count etc. for complex types.
*/

/*!
    \fn bool QModbusPdu::isValid() const

    Returns true if the PDU is valid; otherwise false.

    A PDU is considered valid if the message code is in the range of 1 to 255 decimal and the
    PDU's compound size (function code + data) does not exceed 253 bytes. A default constructed
    PDU is invalid.
*/

/*!
    \fn bool QModbusPdu::isException() const

    Returns true if the PDU contains an exception code; otherwise false.
*/

/*!
    \fn QModbusPdu::ExceptionCode QModbusPdu::exceptionCode() const

    Returns the response's exception code.
*/

/*!
    \fn qint16 QModbusPdu::size() const

    Returns the PDU's full size, including function code and data size.
*/

/*!
    \fn qint16 QModbusPdu::dataSize() const

    Returns the PDU's data size, excluding the function code.
*/

/*!
    \fn FunctionCode QModbusPdu::functionCode() const

    Returns the PDU's function code.
*/

/*!
    \fn void QModbusPdu::setFunctionCode(FunctionCode code)

    Sets the PDU's function code to \a code.
*/

/*!
    \fn QByteArray QModbusPdu::data() const

    Returns the PDU's payload, excluding the function code.
    The payload is stored in big-endian byte order.
*/

/*!
    \fn void QModbusPdu::setData(const QByteArray &data)

    Sets the PDU's function payload to \a data. The data is expected to be stored in big-endian
    byte order already.
*/

/*!
    \fn void QModbusPdu::decodeData(Args data) const

    Converts the payload into host endianness and reads it into \a data. Data can be a variable
    length argument list.

    \code
        QModbusResponsePdu response(QModbusPdu::ReportServerId);
        response.encodeData(quint8(0x02), quint8(0x01), quint8(0xff));
        quint8 count, id, run;
        response.decodeData(&count, &id, &run);
    \endcode

    \note Usage should be limited the POD types only. This is because \c QDataStream stream
    operators will not only read raw data, but also e.g. size, count etc. for complex types.
*/

/*!
    \fn void QModbusPdu::encodeData(Args data)

    Sets the payload to \a data. The data is converted and stored in big-endian byte order.

    \code
        QModbusRequestPdu request(QModbusPdu::ReadCoils);
        // starting address and quantity of coils
        request.encodeData(quint16(0x0c), quint16(0x0a));
    \endcode

    \note Usage should be limited the POD types only. This is because \c QDataStream stream
        operators will not only append raw data, but also e.g. size, count etc. for complex types.
*/

/*!
    Writes the Modbus \a pdu to the \a debug stream.
*/
QDebug operator<<(QDebug debug, const QModbusPdu &pdu)
{
    QDebugStateSaver _(debug);
    debug.nospace().noquote() << "0x" << QByteArray(1, pdu.functionCode()).toHex() << pdu.data()
        .toHex();
    return debug;
}

/*!
    \relates QModbusPdu

    Writes a \a pdu to the \a stream and returns a reference to the stream.
*/
QDataStream &operator<<(QDataStream &stream, const QModbusPdu &pdu)
{
    stream << static_cast<quint8> (pdu.functionCode());
    if (!pdu.data().isEmpty())
        stream.writeRawData(pdu.data().constData(), pdu.data().size());

    return stream;
}

/*!
    \class QModbusRequest
    \inmodule QtSerialBus
    \since 5.6

    \brief QModbusRequest is a container class containing the function code and payload that is
    stored inside a Modbus ADU.

    A Modbus request usually consists of a single byte describing the \c FunctionCode and N bytes
    of payload

    A typical Modbus request can looks like this:
    \code
        QModbusRequest request(QModbusRequest::WriteMultipleCoils,
            QByteArray::fromHex("0013000a02cd01"));
    \endcode
    \note When using the constructor taking the \c QByteArray, please make sure to convert the
        containing data to big-endian byte order before creating the request.

    The same request can be created like this, if the values are know at compile time:
    \code
        quint16 startAddress = 19, numberOfCoils = 10;
        quint8 payloadInBytes = 2, outputHigh = 0xcd, outputLow = 0x01;
        QModbusRequest request(QModbusRequest::WriteMultipleCoils, startAddress, numberOfCoils,
            payloadInBytes, outputHigh, outputLow);
    \endcode
*/

/*!
    \fn QModbusRequest::QModbusRequest()

    Constructs an invalid QModbusRequest.
*/

/*!
    \fn QModbusRequest::QModbusRequest(const QModbusPdu &pdu)

    Constructs a copy of \a pdu.
*/

/*!
    \fn QModbusRequest::QModbusRequest(FunctionCode code, Args... data)

    Constructs a QModbusRequest with function code set to \a code and payload set to \a data.
    The data is converted and stored in big-endian byte order.

    \note Usage should be limited the POD types only. This is because \c QDataStream stream
        operators will not only append raw data, but also e.g. size, count etc. for complex types.
*/

/*!
    \fn QModbusRequest::QModbusRequest(FunctionCode code, const QByteArray &data = QByteArray())

    Constructs a QModbusResponse with function code set to \a code and payload set to \a data.
    The data is expected to be stored in big-endian byte order already.
*/

/*!
    Returns the minimum data size for a request, based on the function \a code.
    \note The function returns \c {-1} if the size could not be properly calculated.
*/
int QModbusRequest::minimumDataSize(FunctionCode code)
{
    return Private::minimumDataSize(code, Private::Type::Request);
}

/*!
    Calculates the expected data size for a request, based on the function \a code.
    The \a data byte array is expected to be the part directly following the function
    code. Returns the full size of the PDU's data part; \c {-1} if the size could not be
    properly calculated.
*/
int QModbusRequest::calculateDataSize(FunctionCode code, const QByteArray &data)
{
    int size = -1;
    int minimum = Private::minimumDataSize(code, Private::Type::Request);
    if (minimum < 0)
        return size;

    switch (code) {
    case QModbusPdu::WriteMultipleCoils:
        minimum -= 1; // first payload payload byte
        if (data.size() >= minimum)
            size = minimum + data[minimum - 1] /*byte count*/;
        break;
    case QModbusPdu::WriteMultipleRegisters:
    case QModbusPdu::ReadWriteMultipleRegisters:
        minimum -= 2; // first 2 payload payload bytes
        if (data.size() >= minimum)
            size = minimum + data[minimum - 1] /*byte count*/;
        break;
    case QModbusPdu::ReadFileRecord:
    case QModbusPdu::WriteFileRecord:
        if (data.size() >= 1)
            size = 1 /*byte count*/ + data[0] /*actual bytes*/;
        break;
    case QModbusPdu::Diagnostics:
    case QModbusPdu::EncapsulatedInterfaceTransport:
        // The following part makes sure we pass all checks in the request processing functions
        // and not handled function codes get passed on the processPrivateRequest() function.
        size = data.size();
        break;
    default:
        size = minimum;
        break;
    }
    return size;
}

/*!
    \relates QModbusRequest

    Reads a \a pdu from the \a stream and returns a reference to the stream.

    \note The function might fail to properly stream PDU's with function code
    \l QModbusPdu::Diagnostics or \l QModbusPdu::EncapsulatedInterfaceTransport
    because of the missing size indicator inside the PDU. In particular this may
    happen when the PDU is embedded into a stream that doesn't end with the
    diagnostic/encapsulated request itself.
*/
QDataStream &operator>>(QDataStream &stream, QModbusRequest &pdu)
{
    quint8 code;
    stream >> code;
    pdu.setFunctionCode(static_cast<QModbusPdu::FunctionCode> (code));

    int size = Private::minimumDataSize(pdu.functionCode(), Private::Type::Request);
    if (size < 0)
        pdu.setFunctionCode(QModbusRequest::Invalid);
    if (size <= 0)
        return stream;

    QByteArray data(size, Qt::Uninitialized);
    if (code == QModbusPdu::Diagnostics || code == QModbusPdu::EncapsulatedInterfaceTransport) {
        data.resize(stream.device()->size() - 1); // One byte for the Function code.
        if (data.size() > 252) { // The maximum PDU data size is 252 bytes.
            pdu.setFunctionCode(QModbusRequest::Invalid);
            return stream;
        }
    }
    stream.device()->peek(data.data(), data.size());
    size = QModbusRequest::calculateDataSize(pdu.functionCode(), data);

    if (size >= 0) {
        data.resize(size);
        stream.readRawData(data.data(), data.size());
        pdu.setData(data);
    } else {
        pdu.setFunctionCode(QModbusRequest::Invalid);
    }

    return stream;
}

/*!
    \class QModbusResponse
    \inmodule QtSerialBus
    \since 5.6

    \brief QModbusResponse is a container class containing the function code and payload that is
        stored inside a Modbus ADU.

    A typical Modbus response can looks like this:
    \code
        QModbusResponse response(QModbusResponse::ReadCoils, QByteArray::fromHex("02cd01"));
    \endcode
    \note When using the constructor taking the \c QByteArray, please make sure to convert the
        containing data to big-endian byte order before creating the request.

    The same response can be created like this, if the values are know at compile time:
    \code
        quint8 payloadInBytes = 2, outputHigh = 0xcd, outputLow = 0x01;
        QModbusResponse response(QModbusResponse::ReadCoils, payloadInBytes, outputHigh, outputLow);
    \endcode
*/

/*!
    \fn QModbusResponse::QModbusResponse()

    Constructs an invalid QModbusResponse.
*/

/*!
    \fn QModbusResponse::QModbusResponse(const QModbusPdu &pdu)

    Constructs a copy of \a pdu.
*/

/*!
    \fn QModbusResponse::QModbusResponse(FunctionCode code, Args... data)

    Constructs a QModbusResponse with function code set to \a code and payload set to \a data.
    The data is converted and stored in big-endian byte order.

    \note Usage should be limited the POD types only. This is because \c QDataStream stream
        operators will not only append raw data, but also e.g. size, count etc. for complex types.
*/

/*!
    \fn QModbusResponse::QModbusResponse(FunctionCode code, const QByteArray &data = QByteArray())

    Constructs a QModbusResponse with function code set to \a code and payload set to \a data.
    The data is expected to be stored in big-endian byte order already.
*/

/*!
    Returns the minimum data size for a response, based on the function \a code.
    \note The function returns \c {-1} if the size could not be properly calculated.
*/
int QModbusResponse::minimumDataSize(FunctionCode code)
{
    if (code & quint8(0x80))
        return 1;
    return Private::minimumDataSize(code, Private::Type::Response);
}

/*!
    Calculates the expected data size for a response, based on the function \a code.
    The \a data byte array is expected to be the part directly following the function
    code. Returns the full size of the PDU's data part; \c {-1} if the size could not be
    properly calculated.
*/
int QModbusResponse::calculateDataSize(FunctionCode code, const QByteArray &data)
{
    int size = -1;
    if (QModbusResponse::minimumDataSize(code) < 0)
        return size;

    switch (code) {
    case QModbusResponse::ReadCoils:
    case QModbusResponse::ReadDiscreteInputs:
    case QModbusResponse::ReadHoldingRegisters:
    case QModbusResponse::ReadInputRegisters:
    case QModbusResponse::GetCommEventLog:
    case QModbusResponse::ReadFileRecord:
    case QModbusResponse::WriteFileRecord:
    case QModbusResponse::ReadWriteMultipleRegisters:
    case QModbusResponse::ReportServerId:
        if (data.size() >= 1)
            size = 1 /*byte count*/ + data[0] /*actual bytes*/;
        break;
    case QModbusResponse::ReadFifoQueue: {
        if (data.size() >= 2) {
            quint16 tmp;
            QDataStream rawSize(data);
            rawSize >> tmp;
            size = tmp + 2; // 2 bytes size info
        }
    }   break;
    case QModbusPdu::Diagnostics:
    case QModbusPdu::EncapsulatedInterfaceTransport:
        size = data.size(); // Make sure we pass all checks in the response processing function.
        break;
    default:
        size = QModbusResponse::minimumDataSize(code);
        break;
    }
    return size;
}

/*!
    \relates QModbusResponse

    Reads a \a pdu from the \a stream and returns a reference to the stream.

    \note The function might fail to properly stream PDU's with function code
    \l QModbusPdu::Diagnostics or \l QModbusPdu::EncapsulatedInterfaceTransport
    because of the missing size indicator inside the PDU. In particular this may
    happen when the PDU is embedded into a stream that doesn't end with the
    diagnostic/encapsulated request itself.
*/
QDataStream &operator>>(QDataStream &stream, QModbusResponse &pdu)
{
    quint8 code;
    stream >> code;
    pdu.setFunctionCode(static_cast<QModbusPdu::FunctionCode> (code));

    int size = QModbusResponse::minimumDataSize(pdu.functionCode());
    if (size < 0)
        pdu.setFunctionCode(QModbusResponse::Invalid);
    if (size <= 0)
        return stream;

    QByteArray data(size, Qt::Uninitialized);
    if (code == QModbusPdu::Diagnostics || code == QModbusPdu::EncapsulatedInterfaceTransport) {
        data.resize(stream.device()->size() - 1); // One byte for the Function code.
        if (data.size() > 252) { // The maximum PDU data size is 252 bytes.
            pdu.setFunctionCode(QModbusRequest::Invalid);
            return stream;
        }
    }
    stream.device()->peek(data.data(), data.size());
    size = QModbusResponse::calculateDataSize(pdu.functionCode(), data);

    if (size >= 0) {
        data.resize(size);
        stream.readRawData(data.data(), data.size());
        pdu.setData(data);
    } else {
        pdu.setFunctionCode(QModbusResponse::Invalid);
    }

    return stream;
}

/*!
    \class QModbusExceptionResponse
    \inmodule QtSerialBus
    \since 5.6

    \brief QModbusExceptionResponse is a container class containing the function and error code
        inside a Modbus ADU.

    A typical QModbusExceptionResponse response can looks like this:
    \code
        QModbusExceptionResponse exception(QModbusExceptionResponse::ReportServerId,
            QModbusExceptionResponse::ServerDeviceFailure);
    \endcode
*/

/*!
    \fn QModbusExceptionResponse::QModbusExceptionResponse()

    Constructs an invalid QModbusExceptionResponse.
*/

/*!
    \fn QModbusExceptionResponse::QModbusExceptionResponse(const QModbusPdu &pdu)

    Constructs a copy of \a pdu.
*/

/*!
    \fn QModbusExceptionResponse::QModbusExceptionResponse(FunctionCode code, ExceptionCode ec)

    Constructs a QModbusExceptionResponse with function code set to \a code and exception error
    code set to \a ec.
*/

/*!
    \fn void QModbusExceptionResponse::setFunctionCode(FunctionCode c)

    Sets the response's function code to \a c.
*/

/*!
    \fn void QModbusExceptionResponse::setExceptionCode(ExceptionCode ec)

    Sets the response's exception code to \a ec.
*/

QT_END_NAMESPACE
