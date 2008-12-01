#include <QStandardItemModel>
#include <QSortFilterProxyModel>

class MemberModel : public QStandardItemModel
{
    Q_OBJECT
public:
    MemberModel(QObject *);
    Qt::ItemFlags flags(const QModelIndex&) const;
};

class MemberFilter : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    MemberFilter(QObject *);
protected:
    bool filterAcceptsRow(int , const QModelIndex &) const;
};