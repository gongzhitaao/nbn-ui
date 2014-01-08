#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMutex>

namespace Ui {
class MainWindow;
}

class NBN;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void errorReady(int);

private slots:
    void on_parsingFinished();

    void on_errorReady(int);

    void on_trainingFinished();

    void on_comboBox_Algorithm_currentIndexChanged(int index);

    void on_pushButton_Configuration_clicked();

    void on_actionParameter_triggered();

    void on_actionConfiguration_triggered();

    void on_actionExit_triggered();

    void on_pushButton_Data_clicked();

    void on_pushButton_Train_clicked();

private:
    bool configuration(const QString &fileName);
    void training();

    Ui::MainWindow *ui;

    QString lastConfPath_;
    QString lastDataPath_;
    QString dataFile_;

    NBN *nbn_;

    int totalRun_;
    int maxIteration_;
    int maxError_;
    QVector<double> errors_;
    QMutex mutex_;
    int count_;
};

#endif // MAINWINDOW_H
