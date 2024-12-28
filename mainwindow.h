#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "tag.h"
#include "fuzzer.h"
#include "tabmodel.h"
#include "codemodel.h"
#include "vrangemodel.h"
#include "rdel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(Fuzzer* t, QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void startFuzz();
    void error(QString err);
    void text_slot(QString err);

private:
    Ui::MainWindow *ui;
    Fuzzer* fuzzer;
    TagModel* tagmodel;
    CodeModel* codemodel;
    VRangeModel* vrangemodel;
    RDel* rDelegate;
};

#endif // MAINWINDOW_H
