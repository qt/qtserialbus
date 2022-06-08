// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "writeregistermodel.h"

enum { NumColumn = 0, CoilsColumn = 1, HoldingColumn = 2, ColumnCount = 3, RowCount = 10 };

WriteRegisterModel::WriteRegisterModel(QObject *parent)
    : QAbstractTableModel(parent),
      m_coils(RowCount, false), m_holdingRegisters(RowCount, 0u)
{
}

int WriteRegisterModel::rowCount(const QModelIndex &/*parent*/) const
{
    return RowCount;
}

int WriteRegisterModel::columnCount(const QModelIndex &/*parent*/) const
{
    return ColumnCount;
}

QVariant WriteRegisterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= RowCount || index.column() >= ColumnCount)
        return QVariant();

    Q_ASSERT(m_coils.count() == RowCount);
    Q_ASSERT(m_holdingRegisters.count() == RowCount);

    if (index.column() == NumColumn && role == Qt::DisplayRole)
        return QString::number(index.row());

    if (index.column() == CoilsColumn && role == Qt::CheckStateRole) // coils
        return m_coils.at(index.row()) ? Qt::Checked : Qt::Unchecked;

    if (index.column() == HoldingColumn && role == Qt::DisplayRole) // holding registers
        return QString("0x%1").arg(QString::number(m_holdingRegisters.at(index.row()), 16));

    return QVariant();

}

QVariant WriteRegisterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case NumColumn:
            return QStringLiteral("#");
        case CoilsColumn:
            return QStringLiteral("Coils  ");
        case HoldingColumn:
            return QStringLiteral("Holding Registers");
        default:
            break;
        }
    }
    return QVariant();
}

bool WriteRegisterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() ||  index.row() >= RowCount || index.column() >= ColumnCount)
        return false;

    Q_ASSERT(m_coils.count() == RowCount);
    Q_ASSERT(m_holdingRegisters.count() == RowCount);

    if (index.column() == CoilsColumn && role == Qt::CheckStateRole) { // coils
        auto s = static_cast<Qt::CheckState>(value.toUInt());
        s == Qt::Checked ? m_coils.setBit(index.row()) : m_coils.clearBit(index.row());
        emit dataChanged(index, index);
        return true;
    }

    if (index.column() == HoldingColumn && role == Qt::EditRole) { // holding registers
        bool result = false;
        quint16 newValue = value.toString().toUShort(&result, 16);
        if (result)
            m_holdingRegisters[index.row()] = newValue;

        emit dataChanged(index, index);
        return result;
    }

    return false;
}

Qt::ItemFlags WriteRegisterModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= RowCount || index.column() >= ColumnCount)
        return QAbstractTableModel::flags(index);

    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if ((index.row() < m_address) || (index.row() >= (m_address + m_number)))
        flags &= ~Qt::ItemIsEnabled;

    if (index.column() == CoilsColumn) // coils
        return flags | Qt::ItemIsUserCheckable;
    if (index.column() == HoldingColumn) // holding registers
        return flags | Qt::ItemIsEditable;

    return flags;
}

void WriteRegisterModel::setStartAddress(int address)
{
    m_address = address;
    emit updateViewport();
}

void WriteRegisterModel::setNumberOfValues(const QString &number)
{
    m_number = number.toInt();
    emit updateViewport();
}
