#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fstream>
#include <vector>
#include <sstream>

#include <QProgressBar>
#include <QtConcurrent/QtConcurrent>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFlags>
#include <QFutureWatcher>

#include "nbn.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    lastConfPath_(QDir::currentPath()),
    lastDataPath_(QDir::currentPath())
{
    ui->setupUi(this);

    nbn_ = new NBN();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete nbn_;
}

void MainWindow::on_comboBox_Algorithm_currentIndexChanged(int index)
{
    ui->stackedWidget_AlgorithmParam->setCurrentIndex(index);
}

void MainWindow::on_pushButton_Configuration_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
                this, tr("Open Configuration File"), lastConfPath_,
                "Conf files (*.in);;Text files (*.txt);;All files (*.*)");

    if (!fileName.isNull() && !fileName.isEmpty()) {
        lastConfPath_ = QFileInfo(fileName).path();

        QFutureWatcher<bool> *watcher = new QFutureWatcher<bool>;
        QObject::connect(watcher, SIGNAL(finished()), this, SLOT(on_parsingFinished()));

        QFuture<bool> parser = QtConcurrent::run(this, &MainWindow::configuration, fileName);
        watcher->setFuture(parser);
    }
}

void MainWindow::on_actionParameter_triggered()
{
}

void MainWindow::on_actionConfiguration_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
                this, tr("Open Configuration File"), lastConfPath_,
                "Conf files (*.in);;Text files (*.txt);;All files (*.*)");

    if (!fileName.isNull() && !fileName.isEmpty())
        lastConfPath_ = QFileInfo(fileName).path();

    QFutureWatcher<bool> *watcher = new QFutureWatcher<bool>;
    QObject::connect(watcher, SIGNAL(finished()), this, SLOT(on_parsingFinished()));

    QFuture<bool> parser = QtConcurrent::run(this, &MainWindow::configuration, fileName);
    watcher->setFuture(parser);
}

void MainWindow::on_parsingFinished()
{
    ui->pushButton_Data->setEnabled(true);
    ui->pushButton_Train->setEnabled(true);
}

bool MainWindow::configuration(const QString &fileName)
{
    QVector<int> topology, outputs;
    QVector<double> gains, weights;
    QVector<NBN::nbn_activation_func_enum> activations;

    QVector<QString> types;

    QFile file(fileName);
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

            for (int i = 2; i < list.size(); ++i) {
                if (list[i].startsWith("fun")) {
                    if (list[i].endsWith("lin")) activations.push_back(NBN::NBN_LINEAR);
                    else if (list[i].endsWith("uni")) activations.push_back(NBN::NBN_SIGMOID);
                    else activations.push_back(NBN::NBN_SIGMOID_SYMMETRIC);
                } else if (list[i].startsWith("gain")) {
                    QRegExp rx("(\\d+\\.?|\\d*\\.\\d+)");
                    rx.indexIn(list[i]);
                    gains.push_back(rx.cap(1).toDouble());
                } else {
                    QRegExp rx("(\\d+\\.?|\\d*\\.\\d+)");
                    rx.indexIn(list[i]);
//                    model.der = rx.cap(1).toDouble();
                }
            }
        } else if (line.startsWith("datafile")) {
            QFileInfo info(file);
            dataFile_ = info.absoluteDir().filePath(line.right(line.length() - line.lastIndexOf('=') - 1));
            qDebug() << dataFile_ << endl;
        }
    }

    nbn_->set_topology(topology.toStdVector());
    nbn_->set_gains(gains.toStdVector());
    nbn_->set_activations(activations.toStdVector());

    return true;
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_pushButton_Data_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
                this, tr("Open Configuration File"), lastDataPath_,
                "Text files (*.txt);;All files (*.*)");

    if (!fileName.isNull() && !fileName.isEmpty()) {
        lastDataPath_ = QFileInfo(fileName).path();
        dataFile_ = fileName;
    }
}

void MainWindow::on_pushButton_Train_clicked()
{
    totalRun_ = ui->lineEdit_totalRuns->text().toInt();
    maxIteration_ = ui->lineEdit_maxIteration->text().toInt();
    maxError_ = ui->lineEdit_maxError->text().toDouble();
    count_ = 0;

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>;
    QObject::connect(watcher, SIGNAL(finished()), this, SLOT(on_trainingFinished()));

    QFuture<void> training = QtConcurrent::run(this, &MainWindow::training);
    watcher->setFuture(training);

    QProgressBar *progressBar = new QProgressBar(ui->statusBar);
    progressBar->setAlignment(Qt::AlignRight);
    progressBar->setMaximumSize(180, 19);
    progressBar->setFormat("%v / %m");
    ui->statusBar->addPermanentWidget(progressBar);
    progressBar->setMaximum(totalRun_);
    progressBar->setValue(0);

}

void MainWindow::on_trainingFinished()
{
    QProgressBar *progressBar =
            ui->statusBar->findChild<QProgressBar *>(QString(), Qt::FindDirectChildrenOnly);
    ui->statusBar->removeWidget(progressBar);
}

void MainWindow::training()
{
    std::vector<double> inputs, desired_outputs;
    {
        const int num_input = nbn_->get_num_input();
        const int num_output = nbn_->get_num_output();
        double x;
        std::ifstream fin(dataFile_.toStdString());
        while (true) {
            int i;
            for (i = 0; i < num_input && fin >> x; ++i)
                inputs.push_back(x);

            if (i < num_input) break;

            for (i = 0; i < num_output && fin >> x; ++i)
                desired_outputs.push_back(x);
        }
    }

    for (int runs = 0; runs < totalRun_; ++runs) {
        nbn_->init_default();
        nbn_->train(inputs, desired_outputs, maxIteration_, maxError_);
        mutex_.lock();
        errors_ = QVector<double>::fromStdVector(nbn_->get_error());
        mutex_.unlock();
        QMetaObject::invokeMethod(this, "on_errorReady", Qt::QueuedConnection,
                                  Q_ARG(int, runs + 1));
    }
}

void MainWindow::on_errorReady(int runs)
{
    QVector<double> x;
    double errmin = errors_[0], errmax = errors_[0];
    for (int i = 0; i < errors_.size(); ++i) {
        x.push_back(i);
        if (errors_[i] < errmin) errmin = errors_[i];
        if (errors_[i] > errmax) errmax = errors_[i];
    }
    mutex_.lock();
    ui->customPlot_ErrorPlot->addGraph();
    ui->customPlot_ErrorPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->customPlot_ErrorPlot->yAxis->setNumberFormat("b");
    ui->customPlot_ErrorPlot->yAxis->setNumberPrecision(0);
    ui->customPlot_ErrorPlot->xAxis->setRange(0, errors_.size());
    ui->customPlot_ErrorPlot->yAxis->setRange(errmin, errmax);
    ui->customPlot_ErrorPlot->graph()->setData(x, errors_);
    ui->customPlot_ErrorPlot->replot();
    qDebug() << errors_ << endl;
    QProgressBar *progressBar =
            ui->statusBar->findChild<QProgressBar *>(QString(), Qt::FindDirectChildrenOnly);
    progressBar->setValue(runs);
    mutex_.unlock();
}
