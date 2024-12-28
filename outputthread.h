#ifndef OUTPUTTHREAD_H
#define OUTPUTTHREAD_H

#include <QThread>
#include <QTextBrowser>

class OutputThread : public QThread
{
    Q_OBJECT
public:
    explicit OutputThread(QObject *parent = 0);


private:
signals:
    void error(QString err);
    void text_signal(QString);
    void finished();

    
public slots:
    void process();
    
};

#endif // OUTPUTTHREAD_H
