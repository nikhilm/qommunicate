#include <QMessageBox>
#include <QMimeData>
#include <QUrl>

#include "ipobjects.h"
#include "membermodel.h"
#include "fileutils.h"


MemberModel::MemberModel(QObject *parent=0) : QStandardItemModel(parent) {
    connect(this, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(updateGroupCount(const QModelIndex&)));
    connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(updateGroupCount(const QModelIndex&)));
}

MemberFilter::MemberFilter(QObject *parent=0) : QSortFilterProxyModel(parent) {}

Qt::ItemFlags MemberModel::flags(const QModelIndex& index) const
{
    if(index.isValid())
        return Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
    return Qt::ItemIsDropEnabled ;
}

QStringList MemberModel::mimeTypes() const
{
    qDebug() << "CAlled";
    return QStringList("*/*");
}

Qt::DropActions MemberModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction ;
}

void MemberModel::setGroupCount(QStandardItem* group)
{
    group->setData(tr("Members: %1").arg(group->rowCount()), Qt::ToolTipRole);
}

void MemberModel::updateGroupCount(const QModelIndex& parent)
{
    if(parent.isValid() && itemFromIndex(parent)->type() == TYPE_GROUP)
        setGroupCount(itemFromIndex(parent));
}

bool MemberModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    qDebug() << "Drop recd";
    QStringList files;
    foreach(QUrl url, data->urls())
    {
        files << url.toLocalFile();
    }
    
    fileUtils()->sendFilesUdpRequest(files, (Member*)itemFromIndex(parent.child(row, column)), "");
    return true;
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
