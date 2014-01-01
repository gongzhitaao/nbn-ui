#include "nninfomodel.h"

NNInfoModel::NNInfoModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

int NNInfoModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

int NNInfoModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant NNInfoModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        return QString("Row%1\n Column%2")
                        .arg(index.row() + 1)
                        .arg(index.column() + 1);
    }
    return QVariant();
}
