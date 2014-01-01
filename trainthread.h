#ifndef TRAINTHREAD_H
#define TRAINTHREAD_H

#include <QThread>
#include <QSharedMemory>

class NBN;

class TrainThread : public QThread
{
    Q_OBJECT
public:
    explicit TrainThread(NBN *nbn, int num_run, int max_iteration, double max_error,
                         QString data_file, QObject *parent = 0);

signals:
    void update();

private:
    void run();

    NBN *nbn_;
    int num_run_;
    int max_iteration_;
    double max_error_;
    QString data_file_;
    QSharedMemory errors_;
};

#endif // TRAINTHREAD_H
