#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "target.h"
#include "outputthread.h"
#include "state.h"
#include <QThread>
#include <QDebug>
#include <algorithm>    // std::swap

MainWindow::MainWindow(Fuzzer* t, QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	fuzzer(t)
{
	ui->setupUi(this);

	// Create tableview for Tags
	tagmodel = new TagModel(t->tracer()->tags(), this);
	ui->tagtableview->setModel(tagmodel);
	ui->tagtableview->verticalHeader()->hide();
	ui->tagtableview->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	ui->tagtableview->setSelectionBehavior(QAbstractItemView::SelectRows);
	connect(ui->tagtableview,
			SIGNAL(doubleClicked(QModelIndex)),
			tagmodel,
			SLOT(doubleClicked_slot(QModelIndex)));
	connect(ui->tagtableview->selectionModel(),
			SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			tagmodel,
			SLOT(selectionChangedSlot(const QItemSelection &, const QItemSelection &)));

	// Create tableview for Code
	codemodel = new CodeModel(T::arget().getCode(), T::arget().getCi(), this);
	ui->codetableview->setModel(codemodel);
	ui->codetableview->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	ui->codetableview->verticalHeader()->hide();
	ui->codetableview->setSelectionBehavior(QAbstractItemView::SelectRows);
	connect(ui->codetableview->selectionModel(),
			SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			codemodel,
			SLOT(selectionChangedSlot(const QItemSelection &, const QItemSelection &)));
	connect(codemodel,
			SIGNAL(selectionChanged( addr_t ,  addr_t )),
			tagmodel,
			SLOT(CodeSelectionChangedSlot( addr_t , addr_t )));

	// Create tableview for Value Ranges
	vrangemodel = new VRangeModel();
	ui->vrangeTableView->setModel(vrangemodel);
	ui->vrangeTableView->verticalHeader()->hide();
	ui->vrangeTableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	ui->vrangeTableView->setEditTriggers(QAbstractItemView::AllEditTriggers);
	connect(tagmodel,
			SIGNAL(addVRange_signal(Tag*,Val*,Val*, const QColor*)),
			vrangemodel,
			SLOT(addVRange_slot(Tag*,Val*,Val*, const QColor*)));

	rDelegate = new RDel(this);
	ui->vrangeTableView->setItemDelegateForColumn(3,rDelegate);

	std::string buf;
	T::arget().read_from_child(buf);
	buf.clear();
	//}
	//QThread* thread = new QThread;
	//OutputThread* worker = new OutputThread();
	//worker->moveToThread(thread);
	//connect(worker, SIGNAL(text_signal(QString)), this, SLOT(text_slot(QString)));
	//connect(worker, SIGNAL(error(QString)), this, SLOT(error(QString)));
	//connect(thread, SIGNAL(started()), worker, SLOT(process()));
	//connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
	//connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
	//connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
	//thread->start();
	// textbrowser ...
	//ui->textBrowser->setText("Select Code Section and Variables to start Fuzzing ...");
	//ui->pushButton->setEnabled(false);
	}

MainWindow::~MainWindow()
{
	delete tagmodel;
	delete codemodel;
	delete vrangemodel;
	delete ui;
}

void MainWindow::startFuzz()
{
	// get selected code section
	addr_t from = codemodel->getSelRipFrom();
	addr_t to = codemodel->getSelRipTo();
	if(to==0 || from==0) {
		ui->textBrowser->setText("ERROR: Select Code Section.\n");
		return;
	}
	// get selected variables and accoring value ranges
	std::vector<const VRange*> vrange;
	vrangemodel->getVrange(vrange);
	if(vrange.empty()) {
		ui->textBrowser->setText("ERROR: Select Variables.\n");
		return;
	}


	if(from>to) std::swap(from, to);

	ui->textBrowser->clear();
	ui->textBrowser->setText(
			QString("Starting Fuzz from %1 to %2").arg(
				from, 8, 16, QChar('0')).arg(to, 8, 16, QChar('0')));
	T::arget().reset();
	T::arget().runTo(from);
	struct user_regs_struct regs;
	T::arget().safe_ptrace(PTRACE_GETREGS, 0, &regs);
	State state(regs, Memstate(0,0), 
			Memstate(T::arget().sstart(), T::arget().sstart()+2048));
	ui->textBrowserTarget->clear();
	for( auto value : vrange ) {
		do {
			//Val test(value->tag()->loc(),value->tag()->len());
			//fprintf(stderr, "val: %s\n", test.str());
			value->set();
			ui->textBrowser->append(
					QString("Starting target with %1 = %2 ... ").arg(
						value->tag()->loc(), 8, 16, QChar('0')).arg(
						value->str()));
			std::string buf;
			T::arget().read_from_child(buf);
			buf.clear();
			T::arget().runTo(to);
			T::arget().read_from_child(buf);
			ui->textBrowserTarget->append(buf.c_str());
			ui->textBrowserTarget->append("--------------------------");
			buf.clear();
			ui->textBrowser->append("Done");
			ui->textBrowser->append("--------------------------");
			state.restore();
		}while (value->next());
		value->resetFrom();
	}
	//fuzzer->fuzz(from, to, vrange);
}


void MainWindow::error(QString err)
{
	qDebug() << err;
}
void MainWindow::text_slot(QString str)
{
	ui->textBrowserTarget->append(str);
}
