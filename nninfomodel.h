#ifndef NNINFOMODEL_H
#define NNINFOMODEL_H

#include <QAbstractTableModel>
#include <QVector>

class NBN;

class NNInfoModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum info_enum {INFO_NUM_NEURON = 0, INFO_NUM_INPUT, INFO_NUM_OUTPUT,
                    INFO_NUM_LAYER, INFO_NUM_WEIGHT, INFO_NUM_CONNECTION,
                    INFO_GAINS, INFO_WEIGHTS, INFO_FUNCIONS, INFO_TOPOLOGY,
                    INFO_ALL};

    explicit NNInfoModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void update(info_enum type, const NBN *nbn) {}

private:
    QVector<QVariant> data_;
};

#endif // NNINFOMODEL_H
