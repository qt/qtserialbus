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
#ifndef QMODBUSPDU_H
#define QMODBUSPDU_H

#include <QtSerialBus/qserialbusglobal.h>

#include <QtCore/qdatastream.h>
#include <QtCore/qmetatype.h>

QT_BEGIN_NAMESPACE

class QModbusPdu
{
public:
    enum ExceptionCode {
        IllegalFunction = 0x01,
        IllegalDataAddress = 0x02,
        IllegalDataValue = 0x03,
        ServerDeviceFailure = 0x04,
        Acknowledge = 0x05,
        ServerDeviceBusy = 0x06,
        MemoryParityError = 0x08,
        GatewayPathUnavailable = 0x0A,
        GatewayTargetDeviceFailedToRespond = 0x0B
    };
    Q_ENUMS(ExceptionCode)

    enum FunctionCode {
        Invalid = 0x00,
        ReadCoils = 0x01,
        ReadDiscreteInputs = 0x02,
        ReadHoldingRegisters = 0x03,
        ReadInputRegisters = 0x04,
        WriteSingleCoil = 0x05,
        WriteSingleRegister = 0x06,
        ReadExceptionStatus = 0x07,
        Diagnostics = 0x08,
        GetCommEventCounter = 0x0B,
        GetCommEventLog = 0x0C,
        WriteMultipleCoils = 0x0F,
        WriteMultipleRegisters = 0x10,
        ReportServerId = 0x11,
        ReadFileRecord = 0x14,
        WriteFileRecord = 0x15,
        MaskWriteRegister = 0x16,
        ReadWriteMultipleRegisters = 0x17,
        ReadFifoQueue = 0x18,
        EncapsulatedInterfaceTransport = 0x2B,
        UndefinedFunctionCode = 0x100
    };
    Q_ENUMS(FunctionCode)

    QModbusPdu() Q_DECL_EQ_DEFAULT;
    virtual ~QModbusPdu() {}

    bool isValid() const {
        return (m_code >= ReadCoils && m_code < UndefinedFunctionCode)
                && (m_data.size() < 253);
    }
    bool isException() const { return m_code & quint8(0x80); }

    qint16 size() const { return dataSize() + 1; }
    qint16 dataSize() const { return m_data.size(); }

    FunctionCode functionCode() const { return m_code; }
    virtual void setFunctionCode(FunctionCode code) { m_code = code; }

    QByteArray data() const { return m_data; }
    void setData(const QByteArray &newData) { m_data = newData; }

    template <typename ... Args> void encodeData(Args ... newData) {
        encode(std::forward<Args>(newData)...);
    }
    template <typename ... Args> void decodeData(Args && ... newData) const {
        decode(std::forward<Args>(newData)...);
    }

protected:
    template <typename ... Args>
    QModbusPdu(FunctionCode code, Args ... newData)
        : m_code(code)
    {
        encode(std::forward<Args>(newData)...);
    }
    QModbusPdu(FunctionCode code, const QByteArray &newData)
        : m_code(code)
        , m_data(newData)
    {}

    QModbusPdu(const QModbusPdu &) Q_DECL_EQ_DEFAULT;
    QModbusPdu &operator=(const QModbusPdu &) Q_DECL_EQ_DEFAULT;

private:
    template <typename T> void encode(QDataStream *stream, const T &t) {
        static_assert(std::is_pod<T>::value, "Only POD types supported.");
        (*stream) << t;
    }
    template <typename T> void decode(QDataStream *stream, T &t) const {
        static_assert(std::is_pod<T>::value, "Only POD types supported.");
        (*stream) >> *t;
    }

    template<typename ... Args> void encode(Args ... newData) {
        if (sizeof...(Args)) {
            QDataStream stream(&m_data, QIODevice::WriteOnly);
            char tmp[1024] = { (encode(&stream, newData), void(), '0')... };
            Q_UNUSED(tmp)
        }
    }
    template<typename ... Args> void decode(Args ... newData) const {
        Q_CONSTEXPR quint32 argCount = sizeof...(Args);
        if (argCount > 0 && !m_data.isEmpty()) {
            QDataStream stream(m_data);
            char tmp[1024] = { (decode(&stream, newData), void(), '0')... };
            Q_UNUSED(tmp)
        }
    }

private:
    FunctionCode m_code = Invalid;
    QByteArray m_data;
};

class QModbusRequest : public QModbusPdu
{
public:
    QModbusRequest() Q_DECL_EQ_DEFAULT;
    QModbusRequest(const QModbusPdu &pdu)
        : QModbusPdu(pdu)
    {}
    template <typename ... Args>
    QModbusRequest(FunctionCode code, Args ... newData)
        : QModbusPdu(code, newData...)
    {}
    QModbusRequest(FunctionCode code, const QByteArray &newData = QByteArray())
        : QModbusPdu(code, newData)
    {}
};

class QModbusResponse : public QModbusPdu
{
public:
    QModbusResponse() Q_DECL_EQ_DEFAULT;
    QModbusResponse(const QModbusPdu &pdu)
        : QModbusPdu(pdu)
    {}
    template <typename ... Args>
    QModbusResponse(FunctionCode code, Args ... newData)
        : QModbusPdu(code, newData...)
    {}
    QModbusResponse(FunctionCode code, const QByteArray &newData = QByteArray())
        : QModbusPdu(code, newData)
    {}
};

class QModbusExceptionResponse : public QModbusResponse
{
public:
    QModbusExceptionResponse() Q_DECL_EQ_DEFAULT;
    QModbusExceptionResponse(const QModbusPdu &pdu)
        : QModbusResponse(pdu)
    {}
    QModbusExceptionResponse(FunctionCode fc, ExceptionCode ec)
        : QModbusResponse(FunctionCode(quint8(fc) | quint8(0x80)), static_cast<quint8> (ec))
    {}

    void setFunctionCode(FunctionCode c) {
        QModbusPdu::setFunctionCode(FunctionCode(quint8(c) | quint8(0x80)));
    }
    void setExeceptionCode(ExceptionCode ec) { QModbusPdu::encodeData(quint8(ec)); }
};

Q_SERIALBUS_EXPORT QDataStream &operator<<(QDataStream &stream, const QModbusPdu &pdu);
Q_SERIALBUS_EXPORT QDataStream &operator>>(QDataStream &stream, QModbusPdu &pdu);

Q_DECLARE_TYPEINFO(QModbusPdu, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(QModbusPdu::ExceptionCode, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QModbusPdu::FunctionCode, Q_PRIMITIVE_TYPE);

Q_DECLARE_TYPEINFO(QModbusRequest, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(QModbusResponse, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(QModbusExceptionResponse, Q_MOVABLE_TYPE);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QModbusPdu::ExceptionCode)
Q_DECLARE_METATYPE(QModbusPdu::FunctionCode)

#endif // QMODBUSPDU_H
