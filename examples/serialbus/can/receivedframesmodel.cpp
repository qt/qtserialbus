/****************************************************************************
**
** Copyright (C) 2021 Evgeny Shtanov <shtanov_evgenii@mail.ru>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the QtSerialBus module.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "receivedframesmodel.h"

#include <QSize>

#include <iterator>

constexpr int ColumnAlignment[] = {
    Qt::AlignRight | Qt::AlignVCenter,
    Qt::AlignRight | Qt::AlignVCenter,
    Qt::AlignCenter,
    Qt::AlignRight | Qt::AlignVCenter,
    Qt::AlignRight | Qt::AlignVCenter,
    Qt::AlignLeft | Qt::AlignVCenter
};

ReceivedFramesModel::ReceivedFramesModel(QObject *parent) : QAbstractTableModel(parent)
{
}

bool ReceivedFramesModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);

    QList<QStringList>::iterator i_start = m_framesQueue.begin() + row;
    QList<QStringList>::iterator i_end = i_start + count;
    m_framesQueue.erase(i_start, i_end);

    endRemoveRows();

    return true;
}

QVariant ReceivedFramesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role == Qt::DisplayRole) && (orientation == Qt::Horizontal)) {
        switch (section) {
        case Number:
            return tr("#");
        case Timestamp:
            return tr("Timestamp");
        case Flags:
            return tr("Flags");
        case CanID:
            return tr("CAN-ID");
        case DLC:
            return tr("DLC");
        case Data:
            return tr("Data");
        }
    }

    if ((role == Qt::SizeHintRole) && (orientation == Qt::Horizontal)) {
        switch (section) {
        case Number:
            return QSize(80, 25);
        case Timestamp:
            return QSize(130, 25);
        case Flags:
            return QSize(25, 25);
        case CanID:
            return QSize(50, 25);
        case DLC:
            return QSize(25, 25);
        case Data:
            return QSize(200, 25);
        }
    }

    return {};
}

QVariant ReceivedFramesModel::data(const QModelIndex &index, int role) const
{
    if (m_framesQueue.empty())
        return {};

    const int row = index.row();
    const int column = index.column();

    switch (role) {
    case Qt::TextAlignmentRole:
        return ColumnAlignment[index.column()];
    case Qt::DisplayRole:
        return m_framesQueue.at(row).at(column);
    case ClipboardTextRole:
        if (index.column() == DLC)
            return QString("[%1]").arg(m_framesQueue.at(row).at(column));
        else
            return m_framesQueue.at(row).at(column);
    default:
        return {};
    }
}

int ReceivedFramesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_framesQueue.size();
}

int ReceivedFramesModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : Count;
}

void ReceivedFramesModel::appendFrames(const QList<QStringList> &slvector)
{
    m_framesAccumulator.append(slvector);
}

bool ReceivedFramesModel::needUpdate() const
{
    return !m_framesAccumulator.empty();
}

void ReceivedFramesModel::update()
{
    if (m_framesAccumulator.empty())
        return;

    if (m_queueLimit)
        appendFramesRingBuffer(m_framesAccumulator);
    else
        appendFramesUnlimited(m_framesAccumulator);

    m_framesAccumulator.clear();
}

void ReceivedFramesModel::appendFramesRingBuffer(const QList<QStringList> &slvector)
{
    if (m_queueLimit <= (rowCount() + slvector.size())) {
        if (slvector.size() < m_queueLimit)
            removeRows(0, rowCount() + slvector.size() - m_queueLimit + 1);
        else
            clear();
    }

    beginInsertRows(QModelIndex(), rowCount(), rowCount() + slvector.size() - 1);

    if (slvector.size() < m_queueLimit)
        m_framesQueue.append(slvector);
    else
        m_framesQueue.append(slvector.mid(slvector.size() - m_queueLimit));

    endInsertRows();
}

void ReceivedFramesModel::appendFrame(const QStringList &slist)
{
    appendFrames({slist});
}

void ReceivedFramesModel::appendFramesUnlimited(const QList<QStringList> &slvector)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + slvector.size() - 1);

    m_framesQueue.append(slvector);

    endInsertRows();
}

void ReceivedFramesModel::clear()
{
    if (m_framesQueue.count()) {
        beginResetModel();

        m_framesQueue.clear();

        endResetModel();
    }
}

void ReceivedFramesModel::setQueueLimit(int limit)
{
    m_queueLimit = limit;

    if (limit && m_framesQueue.size() > limit)
        removeRows(0, m_framesQueue.size() - limit);
}
