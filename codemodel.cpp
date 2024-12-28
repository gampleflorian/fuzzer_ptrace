#include "codemodel.h"
#include <stdio.h>
#include <QDebug>
#include "target.h"

CodeModel::CodeModel(
        const std::map<addr_t, _DInst*>& code,
        const _CodeInfo* ci, QObject *parent) :
    QAbstractTableModel(parent),
    codeToRip_(code), ci_(ci)
{
    size_ = codeToRip_.size();
    for(auto line : codeToRip_) {
        _DInst* inst = line.second;
        code_.push_back(inst);

        _DecodedInst di;
        distorm_format(ci_, inst, &di);
        strCode_.push_back(QString("%1 %2").arg((char*)di.mnemonic.p, (char*)di.operands.p));
	strFunc_.push_back(T::arget().elf()->get_func(inst->addr));
        color_.push_back(QColor(Qt::white));
    }
}

int CodeModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return size_;
}

int CodeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant CodeModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    switch(role) {
    case Qt::DisplayRole:
    {
        if(col==1) {
            return QString("%1").arg(code_[row]->addr,6,16, QChar('0'));
        }else if(col==0) {
            return strCode_[row];
        } else if(col==2) {
		return strFunc_[row];
	}
        break;
    }
    case Qt::BackgroundRole:
    {
        return color_[row];
        break;
    }
    }
    return QVariant();
}


QVariant CodeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString("Code");
            case 1:
                return QString("v address");
            }
        }
    }
    return QVariant();
}


void CodeModel::selectionChangedSlot(const QItemSelection &newSelection,
                                     const QItemSelection &oldSelection)
{
    currentSelection.merge(oldSelection, QItemSelectionModel::Deselect);
    currentSelection.merge(newSelection, QItemSelectionModel::Select);
    int start_row = currentSelection.indexes().first().row();
    int stop_row = currentSelection.indexes().last().row();
    //qDebug() << start_row << " " << stop_row;
    emit selectionChanged(code_[start_row]->addr, code_[stop_row]->addr);
}

addr_t CodeModel::getSelRipFrom()  const
{
    if(currentSelection.empty()) return 0;
    int start_row = currentSelection.indexes().first().row();
    return code_[start_row]->addr;
}


addr_t CodeModel::getSelRipTo()  const
{
    if(currentSelection.empty()) return 0;
    int stop_row = currentSelection.indexes().last().row();
    return code_[stop_row]->addr;
}
