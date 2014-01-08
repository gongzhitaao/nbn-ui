#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFutureWatcher>
#include <QMainWindow>
#include <QMutex>

namespace Ui {
class MainWindow;
}

class QProgressBar;

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

    void on_actionConfiguration_triggered();

    void on_actionExit_triggered();

    void on_pushButton_Train_clicked();

    void on_pushButton_ClearPlot_clicked();

private:
    bool configuration(const QString &fileName);
    bool training();

    Ui::MainWindow *ui;
    QProgressBar *progressBar_;

    QMutex mutex_;

    QString lastConfPath_;
    QString lastDataPath_;
    QString dataFile_;

    NBN *nbn_;

    int totalRun_;
    int maxIteration_;
    int maxError_;
    int failcount_;

    QVector<double> errors_;
    double errormin_, errormax_, errorcur_;

    QVector<double> weights_;
};

#endif // MAINWINDOW_H
