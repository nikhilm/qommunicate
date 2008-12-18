#include <QMessageBox>
#include <QMimeData>
#include <QUrl>

#include "ipobjects.h"
#include "membermodel.h"
#include "filehandler.h"


MemberModel::MemberModel(QObject *parent=0) : QStandardItemModel(parent) {
}

MemberFilter::MemberFilter(QObject *parent=0) : QSortFilterProxyModel(parent) {}

Qt::ItemFlags MemberModel::flags(const QModelIndex& index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled ;
}

bool MemberModel::okToInsert( Member* item )
{
    if(MemberUtils::contains("members_list", item))
        return false;

    MemberUtils::insert("members_list", item);
    return true;
}

void MemberModel::insertRow( int row, QStandardItem* item )
{
    if(okToInsert((Member*)item))
        QStandardItemModel::insertRow(row, item);
}

void MemberModel::appendRow( QStandardItem * item)
{
    if(okToInsert((Member*)item))
        QStandardItemModel::appendRow(item);
}

void MemberModel::setGroupCount(QStandardItem* group)
{
    group->setData(tr("Members: %1").arg(group->rowCount()), Qt::ToolTipRole);
}

void MemberModel::updateGroupCount(QStandardItem *item)
{
    if(item->parent())
    {
        setGroupCount(item->parent());
    }
}

bool MemberModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    QStringList files;
    foreach(QUrl url, data->urls())
    {
        files << url.toLocalFile();
    }
    
    fileHandler()->sendFilesRequest(files, (Member*)itemFromIndex(parent.child(row, column)), "");
}

bool MemberFilter::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if( sourceParent.isValid() && sourceModel()->data(sourceParent).toString().contains(filterRegExp()) ) return true;
    
    QString data = sourceModel()->data(sourceModel()->index(sourceRow, 0, sourceParent)).toString();
    
    bool ret = data.contains(filterRegExp());
    
    QModelIndex subIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    if( subIndex.isValid() )
    {
        for(int i = 0; i < sourceModel()->rowCount(subIndex); ++i)
        {
            ret = ret || filterAcceptsRow(i, subIndex);
        }
    }
    return ret;
}
