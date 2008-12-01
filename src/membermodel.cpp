#include "membermodel.h"


MemberModel::MemberModel(QObject *parent=0) : QStandardItemModel(parent) {}

MemberFilter::MemberFilter(QObject *parent=0) : QSortFilterProxyModel(parent) {}

bool MemberFilter::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QString data = sourceModel()->data(sourceModel()->index(sourceRow, 0, sourceParent)).toString();
    
    return data.contains(filterRegExp());
}