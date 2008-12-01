#include <QMessageBox>
#include "membermodel.h"


MemberModel::MemberModel(QObject *parent=0) : QStandardItemModel(parent) {}

MemberFilter::MemberFilter(QObject *parent=0) : QSortFilterProxyModel(parent) {}

Qt::ItemFlags MemberModel::flags(const QModelIndex& index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled ;
}

bool MemberFilter::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QString data = sourceModel()->data(sourceModel()->index(sourceRow, 0, sourceParent)).toString();
    
    bool ret = data.contains(filterRegExp());
    
    QModelIndex subIndex = sourceModel()->index(sourceRow, 0);
    if( sourceModel()->hasChildren(subIndex) )
    {
        for(int i = 0; i < sourceModel()->rowCount(subIndex); ++i)
        {
            ret = ret || filterAcceptsRow(i, subIndex);
        }
    }
    
    return ret;
}