#include "parserthread.h"

#include <QFile>
#include <QMap>
#include <QRegExp>
#include <QTextStream>
#include <QVector>

#include <QDebug>

#include "nbn.h"

struct Model {
    NBN::nbn_activation_func_enum func;
    double gain, der;
};

void ParserThread::run()
{
    QVector<int> topology, outputs;
    QVector<double> gains, weights;
    QVector<NBN::nbn_activation_func_enum> activations;

    QVector<QString> types;
    QMap<QString, Model> models;

    QFile file(fileName_);
    file.open(QFile::ReadOnly);
    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty() || line.startsWith("//")) continue;

        if (line.startsWith('n')) {
            QStringList list = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);

            qDebug() << list << endl;

            // neuron id
            topology.push_back(list[1].toInt());

            // neuron type
            types.push_back(list[2]);

            // neuron inputs
            for (int i = 3; i < list.size(); ++i)
                topology.push_back(list[i].toInt());

        } else if (line.startsWith('w')) {
            QStringList list = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
            qDebug() << list << endl;
            for (int i = 1; i < list.size(); ++i)
                weights.push_back(list[i].toDouble());
        } else if (line.startsWith(".model")) {
            QStringList list = line.split(QRegExp(",|\\s+"), QString::SkipEmptyParts);
            qDebug() << list << endl;
            Model model;
            for (int i = 2; i < list.size(); ++i) {
                if (list[i].startsWith("fun")) {
                    if (list[i].endsWith("lin")) model.func = NBN::NBN_LINEAR;
                    else if (list[i].endsWith("uni")) model.func = NBN::NBN_SIGMOID;
                    else model.func = NBN::NBN_SIGMOID_SYMMETRIC;
                } else if (list[i].startsWith("gain")) {
                    QRegExp rx("(\\d+\\.?|\\d*\\.\\d+)");
                    rx.indexIn(list[i]);
                    model.gain = rx.cap(1).toDouble();
                } else {
                    QRegExp rx("(\\d+\\.?|\\d*\\.\\d+)");
                    rx.indexIn(list[i]);
                    model.der = rx.cap(1).toDouble();
                }
                models[list[1]] = model;
            }
        }
    }

    for (int i = 0; i < types.size(); ++i) {
        Model model = models[types[i]];
        gains.push_back(model.gain);
        activations.push_back(model.func);
    }

    nbn_->set_topology(topology.toStdVector(), std::vector<int>());
    nbn_->set_gains(gains.toStdVector());
    nbn_->set_activations(activations.toStdVector());
}
