#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSharedMemory>

namespace Ui {
class MainWindow;
}

class NBN;
class NNInfoModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_parsing_finished();

    void on_training_udpated();

    void on_comboBox_Algorithm_currentIndexChanged(int index);

    void on_pushButton_Configuration_clicked();

    void on_actionParameter_triggered();

    void on_actionConfiguration_triggered();


    void on_actionExit_triggered();

    void on_pushButton_Data_clicked();

    void on_pushButton_Train_clicked();

private:
    void configuration();

    Ui::MainWindow *ui;
    QSharedMemory errors_;

    NNInfoModel *info_;

    QString lastConfPath_;
    QString lastDataPath_;
    QString dataFile_;
    NBN *nbn_;


};

#endif // MAINWINDOW_H
