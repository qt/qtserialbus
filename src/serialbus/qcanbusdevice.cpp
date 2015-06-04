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

#include "qcanbusdevice.h"
#include "qcanbusdevice_p.h"

#include "qcanframe.h"
#include <QtCore/qdebug.h>
#include <QtCore/qdatastream.h>

#define SOCKET_CAN_MTU 72

/*!
 *  \class QCanBusDevice
 *  \inmodule QtSerialBus
 *  \inherits QIODevice
 *
 *  \brief The QCanBusDevice is the interface class for CAN Bus.
 *  Working CAN backend must be given in object creation.
 *  QCanBusDevice takes ownership of QSerialBusBackend given in object creation.
 *  Frames written and read must be serialized/deserialized with QDataStream.
 *  writeFrame() serializes QCanFrame properly before writing.
 */
QCanBusDevice::QCanBusDevice(QPointer<QSerialBusBackend> backend, QObject *parent) :
    QSerialBusDevice(backend, parent)
{
    busBackend = backend;
    connect(busBackend.data(), &QSerialBusBackend::readyRead, this, &QCanBusDevice::readyRead);
}

/*
 * For socketCAN following configurations can be done
 * QPair<key,value>
 * key: Loopback value: True/False
 * key: ReceiveOwnMessages value: True/False
 * key: ErrorMask value: int
 * key: CanFilter value: QList<QVariant filter>,
 * QVariant filter is QHash<QString, QVariant>,
 * QHash has 2 keys: "FilterId" and "CanMask" both values are int
 */
void QCanBusDevice::setConfiguration(QPair<QString, QVariant> conf)
{
    busBackend->setConfiguration(conf);
}

/*!
 * \brief Writes and serializes CAN frame to CAN bus. Both standard 8 byte frame
 * and 64 byte Flexible Data-Rate(FD) frame are supported
 * \param id Frame identifier. Standard 11 bits or 29 bits with extended frame format
 * \param data Data payload. Standard max 8 bytes. FD max 64 bytes
 * \param dataLength Bytes used in payload
 * \param EFF Extended frame format. Increases identifier size to 29 bits
 * \param RTR Remote transmission request. Mark frame as remote request frame.
 */
void QCanBusDevice::writeFrame(const QCanFrame &frame)
{
    Q_D(QCanBusDevice);
    write(d->writeFrame(frame));
}

/*!
 * \brief Reads one CAN frame worth of information from backend
 */
QCanFrame QCanBusDevice::readFrame()
{
    //TODO: when additional can backends are implemented,
    //some kind of frame size chooser must be added
    return deserialize(read(SOCKET_CAN_MTU));
}

/*!
 * \brief Deserializes received data to QCanFrame format.
 * \param data Serialized data.
 * \param frame Frame to put all the data in.
 */
QCanFrame QCanBusDevice::deserialize(const QByteArray &data)
{
    Q_D(QCanBusDevice);
    return d->deserialize(data);
}

/*!
 * \brief Sets same version of QDataStream for QCanBusDevice and backend
 * \param v enum QDataStream::Version
 */
void QCanBusDevice::setDataStreamVersion(int version)
{
    busBackend->setDataStreamVersion(version);
}

/*!
 * \brief Returns version of QDataStream in use by QCanBusDevice and backend
 * \return enum QDataStream::Version
 */
int QCanBusDevice::dataStreamVersion()
{
    return busBackend->dataStreamVersion();
}

QByteArray QCanBusDevicePrivate::writeFrame(const QCanFrame &frame)
{
    return serialize(frame);
}

QCanFrame QCanBusDevicePrivate::deserialize(const QByteArray &data)
{
    Q_Q(QCanBusDevice);
    QCanFrame frame;
    if (data.isEmpty())
        return frame;

    QDataStream stream(data);
    stream.setVersion(q->dataStreamVersion());
    QByteArray payload;
    qint32 id;
    qint64 sec;
    qint64 usec;

    stream >> id
           >> payload
           >> sec
           >> usec;

    QCanFrame::TimeStamp stamp;
    stamp.setSeconds(sec);
    stamp.setMicroSeconds(usec);

    frame.setFrameId(id);
    frame.setPayload(payload);
    frame.setTimeStamp(stamp);
    return frame;
}

QByteArray QCanBusDevicePrivate::serialize(const QCanFrame &frame)
{
    Q_Q(QCanBusDevice);

    QByteArray data;
    QDataStream stream(&data, QIODevice::ReadWrite);
    stream.setVersion(q->dataStreamVersion());
    stream << frame.frameId()
           << frame.payload();
    return data;
}
