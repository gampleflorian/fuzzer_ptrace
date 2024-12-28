#include "vrangemodel.h"
#include <QDebug>
#include <QBrush>

VRangeModel::VRangeModel(QObject *parent) :
	QAbstractTableModel(parent)
{
	incNames_.append("SKIP");
	incNames_.append("INC VAL");
	incNames_.append("INC LEN");
}

int VRangeModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 4;
}

int VRangeModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return items_.size();
}

QVariant VRangeModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	int col = index.column();
	vrangeitem* item = items_[row];
	switch(role) {
		case Qt::DisplayRole:
			{
				if(col==0) {
					return QString("%1-%2").arg(item->addrFrom(),
							8 , 16, QChar('0')).arg(item->addrTo(),
							8 , 16, QChar('0')).toUpper();
				} else if(col==1){
					const unsigned char* val = item->vrange()->from()->val();
					int len = item->vrange()->from()->len();
					QString ret;
					for(int i=0; i<len; ++i)
						ret += QString("%1 ").arg(val[i], 2, 16, QChar('0'));
					return ret;
				} else if(col==2) {
					const unsigned char* val = item->vrange()->to()->val();
					int len = item->vrange()->to()->len();
					QString ret;
					for(int i=0; i<len; ++i)
						ret += QString("%1 ").arg(val[i], 2, 16, QChar('0'));
					return ret;
				} else if(col==3) {
					return incNames_[item->vrange()->inc()->type()];

				}
				break;
			}
		case Qt::BackgroundRole:
			{
				QBrush bg(*item->color());
				return bg;
			}
	}
	return QVariant();
}
Qt::ItemFlags VRangeModel::flags(const QModelIndex & index) const
{
	Q_UNUSED(index);
	return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}
bool VRangeModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
	if (role == Qt::EditRole)
	{
		int row = index.row();
		int col = index.column();
		vrangeitem* item = items_[row];
		// TODO
		QByteArray ba = value.toByteArray();

		if(ba.length() > item->tag()->len())
			item->tag()->len(ba.length());
		if(col==1) {
			item->vrange()->from(new Val(
						(unsigned char*)ba.data(),
						ba.length()));
			emit dataChanged(createIndex(row, col, 0),
					createIndex(row, col, 0));
		} else if(col==2) {
			item->vrange()->to(new Val(
						(unsigned char*)ba.data(),
						ba.length()));
			emit dataChanged(createIndex(row, col, 0),
					createIndex(row, col, 0));
		} else if (col==3) { // TODO
			if(value.toUInt()==SKIP && item->vrange()->inc()->type()!=SKIP) {
				item->vrange()->inc(new IncSkip(
							item->vrange()->to()->val(),
							item->vrange()->to()->len()
							));
			} else if(value.toUInt()==IVAL && item->vrange()->inc()->type()!=IVAL) {
				unsigned char step = 1;
				item->vrange()->inc(new IncNum(&step, 1));

			} else if(value.toUInt()==ILEN && item->vrange()->inc()->type()!=ILEN) {
				unsigned char step = 1;
				item->vrange()->inc(new IncStrLen(&step, 1));
			}
			emit dataChanged(createIndex(row, col, 0),
					createIndex(row, col, 0));
		}
	}
	return true;
}


QVariant VRangeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal) {
			switch (section)
			{
				case 0:
					return QString("Address");
				case 1:
					return QString("From");
				case 2:
					return QString("To");
				case 3:
					return QString("Increment Type");
			}
		}
	}
	return QVariant();
}

void VRangeModel::getVrange(std::vector<const VRange*>& vrange) const
{
	for(auto item : items_)
		vrange.push_back(item->vrange());
}

void VRangeModel::addVRange_slot(Tag *tag, Val *from, Val *to, const QColor* tagColor)
{
	if(tags_.contains(tag))
		return;
	tags_.insert(tag);
	int row = items_.size();
	beginInsertRows( QModelIndex(), row, row );
	items_.append(new vrangeitem(tag, from, to, tagColor));
	endInsertRows();
}


vrangeitem::vrangeitem(Tag *tag, Val *from, Val *to, const QColor* tagColor) :
	tag_(tag), color_(tagColor)
{
	vrange_ = new VRange(tag, from, to);
	addrFrom_ = tag->loc();
	addrTo_ = tag->loc()+tag->len();
}
