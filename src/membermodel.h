#ifndef QOM_MEMBERMODEL
#define QOM_MEMBERMODEL

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QSet>

#include "memberutils.h"

class Member;

class MemberModel : public QStandardItemModel
{
    Q_OBJECT
public:
    MemberModel(QObject *);
    Qt::ItemFlags flags(const QModelIndex&) const;
    
    bool dropMimeData(const QMimeData*, Qt::DropAction, int, int, const QModelIndex&);
    
private slots:
    void updateGroupCount(QStandardItem *);
    
private:
    
    void setGroupCount(QStandardItem*);
};

class MemberFilter : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    MemberFilter(QObject *);
protected:
    bool filterAcceptsRow(int , const QModelIndex &) const;
};

#endif