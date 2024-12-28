#include <QApplication>
#include "mainwindow.h"
//#include "fuzzer.h"
//#include "target.h"
#include "fuzzer.h"
#include <stdio.h>
#include <QtCore/QCoreApplication>
#include <QxtBasicFileLoggerEngine>
#include <QxtLogger>


void setupLogger()
{
    QxtBasicFileLoggerEngine *dbg  = new QxtBasicFileLoggerEngine("debug.log");
    QxtBasicFileLoggerEngine *info = new QxtBasicFileLoggerEngine("info.log");

    // qxtLog takes ownership of dbg - no need to manage its memory
    qxtLog->addLoggerEngine("dbg", dbg);
    qxtLog->addLoggerEngine("app", info);

    qxtLog->disableAllLogLevels();

    qxtLog->enableLogLevels("dbg", QxtLogger::AllLevels);
    qxtLog->enableLogLevels("app",  QxtLogger::InfoLevel | QxtLogger::WarningLevel | QxtLogger::ErrorLevel | QxtLogger::CriticalLevel | QxtLogger::FatalLevel | QxtLogger::WriteLevel );
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    setupLogger();

    qxtLog->info("starting fuzzer");

    Fuzzer* fuzzer = new Fuzzer(argc-1, &argv[1]);

    qxtLog->info("fuzzer done");

    MainWindow w(fuzzer);

    qxtLog->info("mainwindow initialized");
    w.show();

    return a.exec();
}
