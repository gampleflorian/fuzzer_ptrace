#include "tabmodel.h"
#include "access.h"
#include <stdio.h>
#include <QDebug>
#include <QMenu>

std::map<Tag*, std::vector<int> > tag_idx;
TagModel::TagModel(const std::set<Tag*>& t, QObject *parent) :
	QAbstractTableModel(parent), rip_(0)
{
	size_ = 0;
	double nTags = t.size();
	double hstep = 359/nTags;
	double hue = 0;

	for(std::set<Tag*>::iterator it=t.begin();
			it!=t.end(); ++it) {
		QModelIndexList indices;
		for(int i=0; i<(*it)->len(); ++i)
			indices << createIndex(size_+i,0,0);

		tagitem* item = new tagitem(*it, hue, indices);
		indexToTagitem_.insert(std::pair<int, tagitem*>(size_, item));
		tagToTagitem_.insert(std::pair<Tag*, tagitem*>(
					(*it), item));
		hue += hstep;
		size_ += (*it)->len();
	}

	setVRange = new QAction(tr("Set Value Range"), this);
}

int TagModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return size_;
}

int TagModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 3;
}

QVariant TagModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	int col = index.column();
	std::map<int,tagitem*, std::greater<int>>::const_iterator it =
		indexToTagitem_.lower_bound(row);
	if(it==indexToTagitem_.end()) return QVariant();
	switch(role) {
		case Qt::DisplayRole:
			{
				int diff = row - it->first;
				if(col==0) {
					return QString("%1").arg(
							it->second->tag()->loc()+diff,
							8 , 16, QChar('0')).toUpper();
				} else {
					//const unsigned char* c =
					//it->second->tag()->init_val()->val();
					const Val* v = it->second->tag()->val(rip_);
					//if(!c) return QString("0");
					const unsigned char* c =
						v->val();
					if(col==1) {
						return QString("%1").arg(
								c[diff],
								2 , 16, QChar('0')).toUpper();
					} else if(col==2) {
						return QString(QChar(c[diff]));
					}
				}
				break;
			}
		case Qt::ForegroundRole:
			{
				return  *it->second->fgcolor();
				break;
			}
		case Qt::BackgroundRole:
			{
				QBrush bg(*it->second->color());
				return bg;
				break;
			}
	}
	return QVariant();
}


QVariant TagModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal) {
			switch (section)
			{
				case 0:
					return QString("Address");
				case 1:
					return QString("H Value");
				case 2:
					return QString("C Value");
			}
		}
	}
	return QVariant();
}

void TagModel::selectionChangedSlot(const QItemSelection &newSelection,
		const QItemSelection &oldSelection)
{
	currentSelection_.merge(oldSelection, QItemSelectionModel::Deselect);
	currentSelection_.merge(newSelection, QItemSelectionModel::Select);

	std::map<int,tagitem*, std::greater<int>>::const_iterator kt = indexToTagitem_.begin();
	for( ; kt!=indexToTagitem_.end(); ++kt)
		kt->second->lowlight();
	emit dataChanged(createIndex(0, 0, 0),
			createIndex(rowCount(), columnCount(), 0));

	if(currentSelection_.indexes().isEmpty()) return;

	QModelIndex index = currentSelection_.indexes()[0];
	int row = index.row();
	std::map<int,tagitem*, std::greater<int>>::const_iterator it =
		indexToTagitem_.lower_bound(row);
	if(it==indexToTagitem_.end()) return;
	it->second->highlight();
	const Tag* tag = it->second->tag();
	const Trace* tforw = tag->tforw(rip_);
	if(tforw){
		tagitem* item = tagToTagitem_.find(tforw->tag)->second;
		item->highlight();
		emit dataChanged(createIndex(item->indices().first().row(), 0, 0),
				createIndex(item->indices().last().row(), columnCount(), 0));
	}
	const Trace* tbackw = tag->tbackw(rip_);
	if(tbackw){
		tagitem* item = tagToTagitem_.find(tbackw->tag)->second;
		item->highlight();
		emit dataChanged(createIndex(item->indices().first().row(), 0, 0),
				createIndex(item->indices().last().row(), columnCount(), 0));
	}

	//for(auto trace : tag->tforw()) {
	//	tagitem* item = tagToTagitem_.find(trace.second->tag)->second;
	//	item->highlight();
	//	emit dataChanged(createIndex(item->indices().first().row(), 0, 0),
	//			createIndex(item->indices().last().row(), columnCount(), 0));
	//}
	//for(auto trace : tag->tbackw()) {
	//	tagitem* item = tagToTagitem_.find(trace.second->tag)->second;
	//	item->highlight();
	//	emit dataChanged(createIndex(item->indices().first().row(), 0, 0),
	//			createIndex(item->indices().last().row(), columnCount(), 0));
	//}
}

void TagModel::CodeSelectionChangedSlot( addr_t start, addr_t stop)
{
	std::map<int,tagitem*, std::greater<int>>::const_iterator kt = indexToTagitem_.begin();
	for( ; kt!=indexToTagitem_.end(); ++kt)
		kt->second->lowlight();
	emit dataChanged(createIndex(0, 0, 0),
			createIndex(rowCount(), columnCount(), 0));

	if(start > stop) std::swap(start, stop);
	rip_ = stop;
	for(addr_t addr=start; addr<=stop; ++addr) {
		for( auto tagToItem : tagToTagitem_) {
			const Tag* tag = tagToItem.first;
			tagitem* tagi = tagToItem.second;
			if(tag->rip_access().find(addr) != tag->rip_access().end()) {
				tagi->highlight();
			}
		}
	}
}
void TagModel::doubleClicked_slot(QModelIndex index)
{
	int row = index.row();
	std::map<int,tagitem*, std::greater<int>>::const_iterator it =
		indexToTagitem_.lower_bound(row);
	if(it==indexToTagitem_.end()) return;
	Tag* tag = it->second->tag();
	if(tag->type() == TT_PTR)
		return;
	unsigned char* val = new unsigned char[tag->len()];
	memset(val, 0, tag->len());
	emit addVRange_signal(tag,
			new Val(tag->loc(), tag->len()),
			new Val(tag->loc(), tag->len()),
			it->second->color());

}

tagitem::tagitem(Tag* t, double hue, QModelIndexList indices) :
	tag_(t), indices_(indices), hue_(hue), highlight_(false)
{
	color_ = new QColor();
	color_->setHsv(hue_, 32, 255);
	if(tag_->type() == TT_PTR)
		fgcolor_ = new QColor(QColor::fromRgb(128, 128, 128));
	else
		fgcolor_ = new QColor(Qt::black);
}
tagitem::~tagitem()
{
	// todo
}
