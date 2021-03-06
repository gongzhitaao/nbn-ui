#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fstream>
#include <limits>
#include <sstream>
#include <vector>

#include <QDateTime>
#include <QProgressBar>
#include <QtConcurrent/QtConcurrent>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFlags>

#include "core/nbn.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Using logarithm for Y axis
    ui->customPlot_ErrorPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->customPlot_ErrorPlot->yAxis->setNumberFormat("eb");
    ui->customPlot_ErrorPlot->yAxis->setNumberPrecision(0);

    lastConfPath_ = QDir::currentPath();
    lastDataPath_ = QDir::currentPath();
    dataFile_ = QString("");

    nbn_ = new NBN();
    totalRun_ = 0;
    maxIteration_ = 0;
    maxError_ = 0.0;
    failcount_ = 0;
    averageTime_ = 0.0;

    training_ = false;
    canceled_ = false;
    close_ = false;

    progressBar_ = new QProgressBar(ui->statusBar);
    progressBar_->setHidden(true);
    progressBar_->setAlignment(Qt::AlignRight);
    progressBar_->setMaximumSize(180, 19);
    progressBar_->setFormat("%v / %m");
    ui->statusBar->addPermanentWidget(progressBar_);
    progressBar_->setMaximum(totalRun_);
    progressBar_->setValue(0);

    ui->lineEdit_totalRuns->setText(QString::number(100));
    ui->lineEdit_maxError->setText(QString::number(0.001));
    maxIterationSave_ = {100000, 300};

    ui->comboBox_Algorithm->setCurrentIndex(1);

    ui->lineEdit_learningConstant->setText(QString::number(nbn_->get_learning_const()));
    ui->lineEdit_momentum->setText(QString::number(nbn_->get_momentum()));
    ui->lineEdit_mu->setText(QString::number(nbn_->get_mu()));
    ui->lineEdit_factor->setText(QString::number(nbn_->get_scale_up()));
    ui->lineEdit_muMax->setText(QString::number(nbn_->get_mu_max()));
    ui->lineEdit_muMin->setText(QString::number(nbn_->get_mu_min()));

    watcher_ = new QFutureWatcher<bool>;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete nbn_;
    delete progressBar_;
    delete watcher_;
}

void MainWindow::on_comboBox_Algorithm_currentIndexChanged(int index)
{
    ui->stackedWidget_AlgorithmParam->setCurrentIndex(index);
    ui->lineEdit_maxIteration->setText(QString::number(maxIterationSave_[index]));
    nbn_->set_algorithm((NBN::nbn_train_enum)index);
}

void MainWindow::on_pushButton_Configuration_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
                this, tr("Open Configuration File"), lastConfPath_,
                "Conf files (*.in);;Text files (*.txt);;All files (*.*)");

    if (fileName.isNull() || fileName.isEmpty()) return;

    lastConfPath_ = QFileInfo(fileName).path();

    QObject::connect(watcher_, SIGNAL(finished()), this, SLOT(on_my_parsingFinished()));
    QFuture<bool> parser = QtConcurrent::run(this, &MainWindow::configuration, fileName);
    watcher_->setFuture(parser);
}

void MainWindow::on_my_parsingFinished()
{
    if (watcher_->future().result()) {
        ui->pushButton_Train->setEnabled(true);

        std::vector<int> vec = nbn_->get_layer_size();
        QString arch = QString::number(vec[0]);
        for (size_t i = 1; i < vec.size(); ++i)
            arch += QString(" - %1").arg(vec[i]);

        QString msg = QString("Number of neurons: <b>%1</b><br/>"
                              "Number of input: <b>%2</b><br/>"
                              "Number of output: <b>%3</b><br/>"
                              "Architecture: <b>%4</b><br/>"
                              "Data file: <b>%5</b><br/>")
                .arg(nbn_->get_num_neuron())
                .arg(nbn_->get_num_input())
                .arg(nbn_->get_num_output())
                .arg(arch)
                .arg(dataFile_);
        ui->textBrowser_message->append(msg);
    } else {
        // TODO: show some message indicating parsing error.
    }
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (training_) {
        event->ignore();
        close_ = true;
        mutex_cancel_.lock();
        canceled_ = true;
        mutex_cancel_.unlock();
    }
}

void MainWindow::on_pushButton_Train_clicked()
{
    if (training_) {
        ui->pushButton_Train->setText("Stopping...");
        mutex_cancel_.lock();
        canceled_ = true;
        mutex_cancel_.unlock();
    } else {
        training_ = true;
        canceled_ = false;

        switch (ui->comboBox_Algorithm->currentIndex()) {
        case 0:
            nbn_->set_learning_const(ui->lineEdit_learningConstant->text().toDouble());
            nbn_->set_momentum(ui->lineEdit_momentum->text().toDouble());
            break;
        case 1:
            nbn_->set_mu(ui->lineEdit_mu->text().toDouble());
            nbn_->set_mu_max(ui->lineEdit_muMax->text().toDouble());
            nbn_->set_mu_min(ui->lineEdit_muMin->text().toDouble());
            double factor = ui->lineEdit_factor->text().toDouble();
            double scaleup, scaledown;
            if (factor > 1.0) {
                scaleup = factor;
                scaledown = 1.0 / factor;
            } else {
                scaledown = factor;
                scaleup = 1.0 / factor;
            }
            nbn_->set_scale_up(scaleup);
            nbn_->set_scale_down(scaledown);
        }

        totalRun_ = ui->lineEdit_totalRuns->text().toInt();
        maxIteration_ = ui->lineEdit_maxIteration->text().toInt();
        maxError_ = ui->lineEdit_maxError->text().toDouble();
        delayedPlot_ = ui->checkBox_delayedPlotting->isChecked();
        failcount_ = 0;
        averageTime_ = 0.0;

        maxIterationSave_[ui->comboBox_Algorithm->currentIndex()] = maxIteration_;

        ui->groupBox_trainingParameter->setEnabled(false);
        ui->groupBox_algorithm->setEnabled(false);

        ui->pushButton_Configuration->setEnabled(false);
        ui->pushButton_ClearPlot->setEnabled(false);
        ui->pushButton_Validate->setEnabled(false);

        ui->customPlot_ErrorPlot->xAxis->setRange(0, maxIteration_);

        errorcur_ = std::numeric_limits<double>::max();

        QFutureWatcher<void> *watcher = new QFutureWatcher<void>;
        QObject::connect(watcher, SIGNAL(finished()), this, SLOT(on_my_trainingFinished()));
        QFuture<void> training = QtConcurrent::run(this, &MainWindow::training);
        watcher->setFuture(training);

        progressBar_->setMaximum(totalRun_);
        progressBar_->setValue(0);
        progressBar_->setHidden(false);

        ui->pushButton_Train->setText("Cancel");
    }
}

void MainWindow::on_my_trainingFinished()
{
    progressBar_->setHidden(true);

    if (delayedPlot_)
        ui->customPlot_ErrorPlot->replot();

    if (failcount_ < totalRun_) {
        // At lease one training is successful.
        nbn_->set_weights(weights_.toStdVector());
        ui->pushButton_Validate->setEnabled(true);
    }

    ui->progressBar_successRate->setMaximum(totalRun_);

    training_ = false;
    ui->pushButton_ClearPlot->setEnabled(true);
    ui->pushButton_Configuration->setEnabled(true);
    ui->pushButton_Train->setText("Train");
    ui->groupBox_trainingParameter->setEnabled(true);
    ui->groupBox_algorithm->setEnabled(true);
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

        mutex_cancel_.lock();
        if (canceled_) {
            QMetaObject::invokeMethod(this, "on_my_canceled", Qt::QueuedConnection,
                                      Q_ARG(int, runs));
            mutex_cancel_.unlock();
            break;
        }
        mutex_cancel_.unlock();

        nbn_->random_weights();
        bool success = nbn_->train(inputs, desired_outputs, maxIteration_, maxError_);

        averageTime_ = (averageTime_ * runs + nbn_->elapsed()) / (runs + 1);

        // This mutex will be released in on_errorReady() when the plotting of
        // current errors is finished.  This may incur some delay, but most of
        // time, it's unnoticable.
        mutex_error_.lock();

        if (!success) ++failcount_;

        errors_ = QVector<double>::fromStdVector(nbn_->get_error());

        if (errors_.back() < errorcur_) {
            errorcur_ = errors_.back();
            weights_ = QVector<double>::fromStdVector(nbn_->get_weights());
        }

        QMetaObject::invokeMethod(this, "on_my_errorReady", Qt::QueuedConnection,
                                  Q_ARG(int, runs + 1));
    }
}

void MainWindow::on_my_errorReady(int runs)
{
    QVector<double> x;
    for (int i = 0; i < errors_.size(); ++i)
        x.push_back(i);

    ui->customPlot_ErrorPlot->addGraph();
    ui->customPlot_ErrorPlot->graph()->setData(x, errors_);
    if (!delayedPlot_) {
        ui->customPlot_ErrorPlot->rescaleAxes();
        ui->customPlot_ErrorPlot->replot();
    }
    progressBar_->setValue(runs);

    ui->progressBar_successRate->setValue(runs - failcount_);
    ui->label_currentError->setText(QString("%1").arg(errors_.back(), 0, 'g', 5));
    ui->label_averageTime->setText(QString("%1").arg(averageTime_, 0, 'g', 5));

    // Current errors are finished, release the mutex.
    mutex_error_.unlock();
}

void MainWindow::on_my_canceled(int runs)
{
    totalRun_ = runs;
    if (close_) this->close();
}

void MainWindow::on_pushButton_ClearPlot_clicked()
{
    ui->customPlot_ErrorPlot->clearGraphs();
    ui->customPlot_ErrorPlot->replot();
    ui->pushButton_ClearPlot->setEnabled(false);
}
