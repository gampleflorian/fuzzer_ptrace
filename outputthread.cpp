#include "outputthread.h"
#include "target.h"
#include <QDebug>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdbool.h>

OutputThread::OutputThread(QObject *parent) :
	QThread(parent)
{
}

void OutputThread::process()
{
	std::string buf;
	while(T::arget().read_from_child(buf)) {
		if(buf.empty() == false) {
			emit text_signal(buf.c_str());
		}
		buf.clear();
	}

	emit finished();
}
