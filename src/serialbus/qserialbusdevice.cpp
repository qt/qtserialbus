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

#include "qserialbusdevice.h"
#include "qserialbusdevice_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QSerialBusDevice
    \inmodule QtSerialBus
    \since 5.6

    \brief The QSerialBusDevice class handles basic communication with serial bus backends.

    QSerialBusDevice is a basic class for communicating with Qt Serial Bus backends.
    QSerialBusDevice is usually inherited to provide a backend-specific API.

    QSerialBusDevice handles reading from and writing to the backend, as well as opening and closing
    it. In addition, QSerialBusDevice reroutes the readyRead() signal from the backend to the user.
 */

//TODO: state reporting missing
//TODO: connected/disconnected signals
/*!
    Constructs a serial bus device with the specified \a parent and the \a backend.

    The serial bus device takes ownership of the backend.
 */
QSerialBusDevice::QSerialBusDevice(QSerialBusBackend *backend, QObject *parent) :
    QIODevice(*new QSerialBusDevicePrivate, parent)
{
    Q_D(QSerialBusDevice);

    Q_ASSERT(backend);
    d->busBackend = backend;
    connect(d->busBackend.data(), &QSerialBusBackend::readyRead, this, &QIODevice::readyRead);
}

/*!
    Destroys the serial bus device.
 */
QSerialBusDevice::~QSerialBusDevice()
{
}

/*!
    Reads at most \a maxSize bytes from the device into \a data. Returns the number of bytes read,
    or \c -1 if an error occurred.
*/

qint64 QSerialBusDevice::readData(char *data, qint64 maxSize)
{
    Q_D(QSerialBusDevice);

    if (!d->busBackend)
        return -1;

    return d->busBackend->read(data, maxSize);
}

/*!
    Writes at most \a maxSize bytes from \a data to the device. Returns the number of bytes read,
    or \c -1 if an error occurred.
*/

qint64 QSerialBusDevice::writeData(const char *data, qint64 maxSize)
{
    Q_D(QSerialBusDevice);

    if (!d->busBackend)
        return -1;

    return d->busBackend->write(data, maxSize);
}

/*!
    Opens the device and sets its OpenMode to \a openMode. Returns \c true if successful;
    otherwise returns \c false.
 */
bool QSerialBusDevice::open(QIODevice::OpenMode openMode)
{
    Q_D(QSerialBusDevice);

    if (!d->busBackend)
        return false;

    if (!d->busBackend->open(openMode))
        return false;

    if (QIODevice::openMode() == QIODevice::OpenModeFlag::NotOpen)
        QIODevice::open(openMode);
    else
        QIODevice::setOpenMode(openMode);

    return true;
}

/*!
    Closes the device and sets its OpenMode to \l QIODevice::NotOpen.
 */
void QSerialBusDevice::close()
{
    Q_D(QSerialBusDevice);

    if (!d->busBackend)
        return;

    d->busBackend->close();
    QIODevice::close();
}

/*!
    \internal
*/
QSerialBusDevice::QSerialBusDevice(QSerialBusBackend *backend, QSerialBusDevicePrivate &dd, QObject *parent)
    : QIODevice(dd, parent)
{
    Q_D(QSerialBusDevice);
    d->busBackend = backend;
    connect(d->busBackend.data(), &QSerialBusBackend::readyRead, this, &QIODevice::readyRead);
}

/*!
    Returns the backend in use.
 */
QSerialBusBackend *QSerialBusDevice::backend() const
{
    Q_D(const QSerialBusDevice);
    return d->busBackend;
}

QT_END_NAMESPACE
