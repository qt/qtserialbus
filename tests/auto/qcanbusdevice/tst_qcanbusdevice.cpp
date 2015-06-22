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

#include <QtSerialBus/QCanBusDevice>
#include <QtSerialBus/QCanFrame>
#include <QtSerialBus/QSerialBusBackend>

#include <QtTest/QtTest>
#include <QSignalSpy>

class tst_Backend : public QSerialBusBackend
{
    Q_OBJECT
public:
    qint64 read(char *buffer, qint64){
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
    void setConfigurationParameter(const QString&, const QVariant&) {};
    QVariant configurationParameter(const QString&) const { return QVariant(); } ;
    QVector<QString> configurationKeys() const { return QVector<QString>(); };
    qint64 write(const char*, qint64) { emit written(); return 72; };
    void setDataStreamVersion(int v){ version = v; };
    int dataStreamVersion() const { return version; };
    qint64 bytesAvailable() const { return 72; };

signals:
    void written();

private:
    int version;
};

class tst_QCanBusDevice : public QObject
{
    Q_OBJECT
public:
    explicit tst_QCanBusDevice();

private slots:
    void initTestCase();
    void conf();
    void write();
    void read();
    void version();

private:
    QCanBusDevice *device;
    QSerialBusBackend *backend;
};

tst_QCanBusDevice::tst_QCanBusDevice() :
    device(0),
    backend(0)
{
}

void tst_QCanBusDevice::initTestCase()
{
    backend = new tst_Backend();
    device = new QCanBusDevice(backend);
    QVERIFY(device);
    QVERIFY(backend);
}

void tst_QCanBusDevice::conf()
{
    device->setConfigurationParameter(QStringLiteral("test"), 1);
}

void tst_QCanBusDevice::write()
{
    QSignalSpy spy(backend, SIGNAL(written()));
    QCanFrame frame;
    frame.setPayload(QByteArray("testData"));
    device->writeFrame(frame);
    QCOMPARE(spy.count(), 0);

    device->open(QIODevice::WriteOnly);
    device->writeFrame(frame);
    QCOMPARE(spy.count(), 1);
}

void tst_QCanBusDevice::read()
{
    QCanFrame frame1 = device->readFrame();
    device->open(QIODevice::ReadOnly);
    QCanFrame frame2 = device->readFrame();
    QVERIFY(!frame1.frameId());
    QVERIFY(frame2.frameId());
}

void tst_QCanBusDevice::version()
{
    int version = 7;
    device->setDataStreamVersion(version);
    QCOMPARE(version, device->dataStreamVersion());
}

QTEST_MAIN(tst_QCanBusDevice)

#include "tst_qcanbusdevice.moc"
