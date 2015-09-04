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

#include <QtTest/QtTest>
#include <QtSerialBus/QModBusReply>
#include <QtCore/qpointer.h>

class dummyReply : public QModBusReply
{
friend class tst_QModBusReply;

protected:
    void setFinished() Q_DECL_OVERRIDE { finish = true; }
    void setError(QModBusReply::RequestError errorCode, const QString &errorString) Q_DECL_OVERRIDE
    {
        errorType = errorCode;
        errorText = errorString;
    }
};

class tst_QModBusReply : public QObject
{
    Q_OBJECT
public:
    explicit tst_QModBusReply();

private slots:
    void error();
    void errorString();
    void finished();
    void result();

private:
    QPointer<dummyReply> reply;
};

tst_QModBusReply::tst_QModBusReply()
{
    reply = new dummyReply();
}

void tst_QModBusReply::error()
{
    QCOMPARE(reply->error(), QModBusReply::NoError);
}

void tst_QModBusReply::errorString()
{
    QString error("error string");
    reply->setError(QModBusReply::IllegalFunction, error);
    QCOMPARE(reply->errorString(), error);
}

void tst_QModBusReply::finished()
{
    QVERIFY(!reply->isFinished());
    QVERIFY(reply->isRunning());

    reply->setFinished();

    QVERIFY(reply->isFinished());
    QVERIFY(!reply->isRunning());
}

void tst_QModBusReply::result()
{
    QList<QModBusDataUnit> units = reply->result();
    QVERIFY(units.isEmpty());
}

QTEST_MAIN(tst_QModBusReply)

#include "tst_qmodbusreply.moc"
