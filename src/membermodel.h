#ifndef QOM_MEMBERMODEL
#define QOM_MEMBERMODEL

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QSet>

class Member;

class MemberModel : public QStandardItemModel
{
    Q_OBJECT
public:
    MemberModel(QObject *);
    Qt::ItemFlags flags(const QModelIndex&) const;
    
    void insertRow(int, QStandardItem*);
    void appendRow(QStandardItem*);
    
private slots:
    void updateGroupCount(QStandardItem *);
    
private:
    QSet<Member*> members;
    
    void setGroupCount(QStandardItem*);
    bool okToInsert(Member*);
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