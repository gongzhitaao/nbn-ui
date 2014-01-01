#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fstream>
#include <vector>
#include <sstream>

#include <QProgressBar>

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>

#include "parserthread.h"
#include "trainthread.h"
#include "nninfomodel.h"

#include "nbn.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    errors_("ErrorData"),
    ui(new Ui::MainWindow),
    lastConfPath_(QDir::currentPath()),
    lastDataPath_(QDir::currentPath())
{
    ui->setupUi(this);

    QProgressBar *progressBar = new QProgressBar(ui->statusBar);
    progressBar->setAlignment(Qt::AlignRight);
    progressBar->setMaximumSize(180, 19);
    progressBar->setFormat("%v / %m");
    ui->statusBar->addPermanentWidget(progressBar);
    progressBar->setValue(0);

    ui->customPlot_ErrorPlot->addGraph();

    info_ = new NNInfoModel;
    ui->tableView_Information->setModel(info_);

    nbn_ = new NBN();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete info_;
    delete nbn_;
}

void MainWindow::on_comboBox_Algorithm_currentIndexChanged(int index)
{
    ui->stackedWidget_AlgorithmParam->setCurrentIndex(index);
}

void MainWindow::on_pushButton_Configuration_clicked()
{
    configuration();
}

void MainWindow::on_actionParameter_triggered()
{
}

void MainWindow::on_actionConfiguration_triggered()
{
    configuration();
}

void MainWindow::on_parsing_finished()
{
    ui->pushButton_Data->setEnabled(true);
    ui->pushButton_Train->setEnabled(true);
}

void MainWindow::on_training_udpated()
{
    std::vector<double> auxvec;
    errors_.lock();
    double *ptr = (double *)errors_.data();
    auxvec.insert(auxvec.end(), ptr, ptr + errors_.size());

    QVector<double> yval = QVector<double>::fromStdVector(auxvec);
    QVector<double> xval;
    for (int i = 0; i < yval.size(); ++i)
        xval.append(i);
    ui->customPlot_ErrorPlot->graph()->addData(xval, yval);
    ui->customPlot_ErrorPlot->replot();

    errors_.unlock();
    errors_.detach();
}

void MainWindow::configuration()
{
    QString fileName = QFileDialog::getOpenFileName(
                this, tr("Open Configuration File"), lastConfPath_,
                "Text files (*.txt);;All files (*.*)");

    if (!fileName.isNull() && !fileName.isEmpty())
        lastConfPath_ = QFileInfo(fileName).path();

    ParserThread *t = new ParserThread(nbn_, fileName);
    QObject::connect(t, SIGNAL(finished()), this, SLOT(on_parsing_finished()));
    t->start();
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
    TrainThread *t = new TrainThread(nbn_, ui->lineEdit_runs->text().toInt(),
                                     ui->lineEdit_maxIteration->text().toInt(),
                                     ui->lineEdit_maxError->text().toDouble(),
                                     dataFile_);
    QObject::connect(t, SIGNAL(update()), this, SLOT(on_training_updated()));
    t->start();
}
