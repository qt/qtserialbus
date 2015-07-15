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

#include <QtSerialBus/QSerialBusDevice>
#include <QtSerialBus/QCanFrame>
#include <QtSerialBus/QSerialBusBackend>

#include <QtTest/QtTest>
#include <QSignalSpy>

class tst_Backend : public QSerialBusBackend
{
    Q_OBJECT
public:
    qint64 read(char *buffer, qint64)
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        QByteArray payload;
        payload.append('A');
        qint32 id = 22;
        qint32 sec = 22;
        qint32 usec = 22;
        stream << id;
        stream << payload;
        stream << sec;
        stream << usec;
        memcpy(buffer, data.constData(), 72);
        return 72;
    };
    bool open(QIODevice::OpenMode) { return true; };
    void close() {};
    void setConfigurationParameter(const QString &key, const QVariant &param)
    {
        value = param;
        keys.append(key);
    };
    QVariant configurationParameter(const QString&) const { return value; } ;
    QVector<QString> configurationKeys() const { return keys; };
    qint64 write(const char*, qint64) { emit written(); return 72; };
    void setDataStreamVersion(int v){ version = v; };
    int dataStreamVersion() const { return version; };
    qint64 bytesAvailable() const { return 72; };

    qint64 availableFrames() const { return 0; }
    QCanFrame nextFrame() { return QCanFrame(); }
    bool writeFrame(const QCanFrame &/*data*/) { return true; }


signals:
    void written();

private:
    int version;
    QVariant value;
    QVector<QString> keys;
};

class tst_QSerialBusDevice : public QObject
{
    Q_OBJECT
public:
    explicit tst_QSerialBusDevice();

private slots:
    void initTestCase();
    void open();
    void readWrite();
    void close();
    void cleanupTestCase();

private:
    QPointer<QSerialBusDevice> device;
    QPointer<QSerialBusBackend> backend;
};

tst_QSerialBusDevice::tst_QSerialBusDevice() :
    device(0),
    backend(0)
{
}

void tst_QSerialBusDevice::initTestCase()
{
    backend = new tst_Backend();
    device = new QSerialBusDevice(backend);
    QVERIFY(device);
    QVERIFY(backend);
}

void tst_QSerialBusDevice::open()
{
    device->open(QIODevice::WriteOnly);
    QCOMPARE(device->openMode(), QIODevice::WriteOnly);
    device->open(QIODevice::ReadWrite);
    QCOMPARE(device->openMode(), QIODevice::ReadWrite);
}

void tst_QSerialBusDevice::readWrite()
{
    QSignalSpy spy(backend, SIGNAL(written()));
    device->open(QIODevice::ReadWrite);
    device->write("test");
    device->read(72);
    QCOMPARE(spy.count(), 1);
}

void tst_QSerialBusDevice::close()
{
    device->close();
    QCOMPARE(device->openMode(), QIODevice::NotOpen);
}

void tst_QSerialBusDevice::cleanupTestCase()
{
    delete device;
    QVERIFY(backend.isNull());
}

QTEST_MAIN(tst_QSerialBusDevice)

#include "tst_qserialbusdevice.moc"
