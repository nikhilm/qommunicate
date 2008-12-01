#include <QStandardItemModel>
#include <QSortFilterProxyModel>

class MemberModel : public QStandardItemModel
{
    Q_OBJECT
public:
    MemberModel(QObject *);
};

class MemberFilter : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    MemberFilter(QObject *);
protected:
    bool filterAcceptsRow(int , const QModelIndex &) const;
};