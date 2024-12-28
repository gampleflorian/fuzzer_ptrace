#include "rdel.h"
#include "vrangemodel.h"


RDel::RDel(QObject *parent) :
	QItemDelegate(parent)
{
}


QWidget* RDel::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(index);
	Q_UNUSED(option);
	QComboBox* editor = new QComboBox(parent);
	const VRangeModel* model = static_cast<const VRangeModel*>(index.model());
	QStringList items = model->incNames();
	editor->addItems(items);
	return editor;

}
void RDel::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	QComboBox *comboBox = static_cast<QComboBox*>(editor);
	int value = index.model()->data(index, Qt::EditRole).toUInt();
	comboBox->setCurrentIndex(value);
}
void RDel::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	QComboBox *comboBox = static_cast<QComboBox*>(editor);
	model->setData(index, comboBox->currentIndex(), Qt::EditRole);
}
void RDel::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(index);
	Q_UNUSED(option);
	editor->setGeometry(option.rect);
}
