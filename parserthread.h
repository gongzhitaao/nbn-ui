#ifndef PARSERTHREAD_H
#define PARSERTHREAD_H

#include <QThread>

class NBN;

class ParserThread : public QThread
{
    Q_OBJECT
public:
    explicit ParserThread(NBN *nbn, QString fileName)
        :nbn_(nbn), fileName_(fileName) {}

private:
    void run();

    NBN *nbn_;
    QString fileName_;
};

#endif // PARSERTHREAD_H
