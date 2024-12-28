#ifndef TABMODEL_H
#define TABMODEL_H

#include <QAbstractTableModel>
#include <QItemSelection>
#include <QColor>
#include <QBrush>
#include <QAction>
#include <QContextMenuEvent>
#include <map>
#include <set>
#include "tag.h"

class tagitem
{
public:
    tagitem(Tag* t, double hue, QModelIndexList indices);
    ~tagitem();
    Tag* tag() {return tag_;}
    const QColor* color() const { return color_;}
    const QColor* fgcolor() const { return fgcolor_;}
    void highlight() {highlight_ =true; color_->setHsv(hue_, 128, 255);}
    void lowlight() {highlight_ =false; color_->setHsv(hue_, 32, 255);}
    const QModelIndexList& indices() const { return indices_;}
private:
    Tag* tag_;
    QColor* color_;
    QColor* fgcolor_;
    QModelIndexList indices_;
    double hue_;
    bool highlight_;
};

class TagModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit TagModel(const std::set<Tag*>& t, QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    QAction* setVRange;

protected:

signals:
    void addVRange_signal(Tag* tag, Val* from, Val* to, const QColor* tagColor);
    
public slots:
    void doubleClicked_slot(QModelIndex);
    void selectionChangedSlot(const QItemSelection &newSelection,
                              const QItemSelection &oldSelection);
    void CodeSelectionChangedSlot( addr_t , addr_t);


private:
    addr_t rip_;
    std::map<Tag*, tagitem*> tagToTagitem_;
    std::map<int, tagitem*, std::greater<int>> indexToTagitem_;
    int size_;
    QItemSelection currentSelection_;
};

#endif // TABMODEL_H
