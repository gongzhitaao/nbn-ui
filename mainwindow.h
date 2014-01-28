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
    void on_my_parsingFinished();

    void on_my_errorReady(int);

    void on_my_trainingFinished();

    void on_my_canceled(int);

    void on_comboBox_Algorithm_currentIndexChanged(int index);

    void on_pushButton_Configuration_clicked();

    void on_actionExit_triggered();

    void on_pushButton_Train_clicked();

    void on_pushButton_ClearPlot_clicked();

protected:
    void closeEvent(QCloseEvent *);

private:
    bool configuration(const QString &fileName);
    void training();

    Ui::MainWindow *ui;
    QProgressBar *progressBar_;

    QString lastConfPath_;
    QString lastDataPath_;
    QString dataFile_;

    NBN *nbn_;

    int totalRun_;
    int maxIteration_;
    double maxError_;
    int failcount_;

    QVector<int> maxIterationSave_;

    QMutex mutex_error_;
    QVector<double> errors_;
    double errorcur_;

    QMutex mutex_cancel_;
    bool training_, canceled_;

    bool delayedPlot_;
    bool close_;

    QVector<double> weights_;

    double averageTime_;
};

#endif // MAINWINDOW_H
