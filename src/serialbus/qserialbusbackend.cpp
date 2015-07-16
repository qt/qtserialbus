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
/*!
    \class QSerialBusBackend
    \inmodule QtSerialBus
    \since 5.6

    \brief The QSerialBusBackend is an abstract class for serial bus backends.

    The QSerialBusBackend class is implemented in the serial bus plugins and provides a common
    API for all backends.
 */

/*!
    \fn bool QSerialBusBackend::open(QIODevice::OpenMode openMode)

    Opens a connection to the backend in the mode specified by \a openMode.  Returns \c true if it
    was successful; otherwise returns \c false.
 */

/*!
    \fn QSerialBusBackend::close()

    Closes the connection to the backend.
 */

/*!
    \fn QSerialBusBackend::setConfigurationParameter(const QString &key, const QVariant &value)

    Sets the value of the configuration parameter of \a key as \a value. Keys and values depend on
    the backend.
 */

/*!
    \fn QVariant QSerialBusBackend::configurationParameter(const QString &key) const

    Returns the current value of the configuration parameter \a key.
 */

/*!
    \fn QVector<QString> QSerialBusBackend::configurationKeys() const

    Returns the list of keys used in the backend.
 */

/*!
    \fn void QSerialBusBackend::error(QString errorString, int errorCode)

    This signal is emitted when the backend encounters an error. \a errorString is a human-readable
    description of the device error that occurred. This should be passed to
    \l QIODevice::setErrorString(). \a errorCode can carry, for example, an error enum value that
    will be interpreted by the appropriate class derived from \l QSerialBusDevice.
*/
