#ifndef VRANGEVIEW_H
#define VRANGEVIEW_H

#include <QAbstractTableModel>
#include <QList>
#include <QSet>
#include <vector>
#include <QStringList>
#include <QColor>
#include "tag.h"
#include "val.h"
#include "vrange.h"

class vrangeitem {
	public:
		vrangeitem(Tag *tag, Val *from, Val *to, const QColor* tagcolor);
		// TODO destructor
		VRange* vrange() { return vrange_;}
		addr_t addrFrom() const { return addrFrom_;}
		addr_t addrTo() const { return addrTo_;}
		Tag* tag() { return tag_;}
		const QColor* color() const{return color_;}
	private:
		VRange* vrange_;
		Tag* tag_;
		addr_t addrFrom_;
		addr_t addrTo_;
		const QColor* color_;
};

class VRangeModel : public QAbstractTableModel
{
	Q_OBJECT
	public:
		explicit VRangeModel(QObject *parent = 0);
		int rowCount(const QModelIndex &parent = QModelIndex()) const ;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
		bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
		Qt::ItemFlags flags(const QModelIndex & index) const ;
		QVariant headerData(int section, Qt::Orientation orientation, int role) const;

		const QStringList& incNames() const {return incNames_;}
		void getVrange(std::vector<const VRange*>&) const;

	protected:

	signals:
		//void editCompleted(const QString &);

		public slots:
			void addVRange_slot(Tag* tag, Val *from, Val *to, const QColor* tagColor);

	private:
		QList<vrangeitem*> items_;
		QSet<Tag*> tags_;
		QStringList incNames_;
};


#endif // VRANGEVIEW_H
