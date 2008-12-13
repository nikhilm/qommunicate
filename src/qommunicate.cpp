#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QFile>
#include <QMenu>
#include <QTreeWidget>

#include "about.h"
#include "settings.h"
#include "ipobjects.h"
#include "messenger.h"
#include "messagedialog.h"
#include "qommunicate.h"

Qommunicate::Qommunicate(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
        
    createTrayIcon();
    
    MemberUtils::init();
    populateTree();    
    firstRun();
    messenger()->login();
    
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanup()));
    
    connect(messenger(), SIGNAL(msg_ansEntry(Message)), this, SLOT(addMember(Message)));
    connect(messenger(), SIGNAL(msg_entry(Message)), this, SLOT(addMemberAndAnswer(Message)));
    connect(messenger(), SIGNAL(msg_recvMsg(Message)), this, SLOT(openDialog(Message)));
}

void Qommunicate::on_searchEdit_textChanged(const QString &text)
{
    filterModel->setFilterFixedString(text);
    ui.memberTree->expandAll();
}

void Qommunicate::on_action_About_triggered()
{
    AboutDialog(this).exec();
}

void Qommunicate::on_action_Settings_triggered()
{
    SettingsDialog(this).exec();
}

void Qommunicate::on_actionMulticast_triggered()
{
    MessageDialog dlg;
    dlg.setModal(false);
    dlg.exec();
}

void Qommunicate::on_actionQuit_triggered()
{
    qApp->quit();
}

void Qommunicate::closeEvent(QCloseEvent * event)
{
    setVisible(false);
    event->ignore();
}

void Qommunicate::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if( reason == QSystemTrayIcon::Trigger )
        setVisible(!isVisible());
}

void Qommunicate::createTrayIcon()
{    
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(windowIcon());
    
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    
    QMenu *menu = new QMenu;
    menu->addAction(ui.actionMulticast);
    menu->addAction(ui.actionQuit);
    trayIcon->setContextMenu(menu);
    
    trayIcon->show();
}

void Qommunicate::populateTree()
{
    model = new MemberModel(this);
    filterModel = new MemberFilter(this);
    
    QStandardItem *parent = model->invisibleRootItem();
    for(int i = 0; i < 5 ; i++) {
        parent->appendRow( new Group(QString("group %1").arg(i)) );
    }
    
    model->item(2)->appendRow( new Member("nsm", "ironik", "192.168.0.2", "Available") );
    model->item(0)->appendRow( new Member("nikhil", "jupiter", "192.168.0.5", "Available"));
    model->item(0)->appendRow( new Member("canada", "montreal", "192.168.23.5", "Available"));
    
    filterModel->setSourceModel(model);
    filterModel->setDynamicSortFilter(true);
    
    ui.memberTree->setSelectionMode(ui.memberTree->ExtendedSelection);
    ui.memberTree->setModel(filterModel);
    ui.memberTree->setHeaderHidden(true);
    ui.memberTree->setExpandsOnDoubleClick(false);
    ui.memberTree->expandAll();
}

/*
If item is a group, adds all submembers to receivers and returns true.
otherwise assumes item is a member, inserts in receivers and returns false
*/
bool Qommunicate::createGroupMemberList(QStandardItem* item, QSet<Member*>& receivers)
{
    if(item->type() == TYPE_GROUP)
    {
        if(!item->hasChildren())
            return true;
        
        for(int i = 0; i < item->rowCount(); i++)
        {
            receivers << (Member*)item->child(i, 0);
        }
        return true;
    }
    receivers << (Member*)item;
    return false;
}

void Qommunicate::on_memberTree_doubleClicked(const QModelIndex& proxyIndex)
{
    
    MessageDialog* dlg;
    QSet<Member*> receivers;
    
    MemberModel* model = (MemberModel*) ( (MemberFilter*) ui.memberTree->model() )->sourceModel();
    QModelIndex index = ((MemberFilter*)ui.memberTree->model())->mapToSource(proxyIndex);
    
    if(index.isValid())
    {
        // only single item clicked
        createGroupMemberList(model->itemFromIndex(index), receivers);
    }
    else
    {
        QModelIndex i;
        foreach(i, ui.memberTree->selectionModel()->selectedRows())
        {            
            QStandardItem* item = model->itemFromIndex(((MemberFilter*)ui.memberTree->model())->mapToSource(i));
            createGroupMemberList(item, receivers);
        }
    }
    
    if(receivers.isEmpty())
        return;
    
    QList<Member*> toDialog = receivers.toList();
    if(toDialog.size() == 1 && MemberUtils::contains("open_conversations", toDialog[0]))
        return;
    
    if(toDialog.size() == 1)
    {
        if(!MemberUtils::contains("open_conversations", toDialog[0]))
            dlg = new MessageDialog( toDialog[0], this );
    }
    else
        dlg = new MessageDialog( toDialog, this );
    
    dlg->setModal(false);
    dlg->show();
}

void Qommunicate::on_statusCombo_currentIndexChanged(const QString& text)
{
    //TODO: status change
}

void Qommunicate::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return) {
        event->accept();
        on_memberTree_doubleClicked(QModelIndex());
        return;
    }
    event->ignore();
}

void Qommunicate::firstRun()
{
    QSettings s;
    if( ! s.contains(tr("nick")) )
        ui.action_Settings->trigger();
}

void Qommunicate::dialogOpened(Member* m)
{
    MemberUtils::insert("open_conversations", m);
}

void Qommunicate::dialogClosed(Member *m)
{
    MemberUtils::remove("open_conversations", m);
}

void Qommunicate::cleanup()
{
    messenger()->logout();
    delete messenger();
}

// Message handling slots
void Qommunicate::addMember(Message msg)
{
    msg.sender()->setName(msg.payload().split('\a')[0]);
    model->appendRow(msg.sender());
}

void Qommunicate::addMemberAndAnswer(Message msg)
{
    addMember(msg);
    messenger()->sendMessage(QOM_ANSENTRY, me().name()+'\0'+myGroup().name(), msg.sender());
}

void Qommunicate::openDialog(Message msg)
{
    if(MemberUtils::contains("open_conversations", msg.sender()))
        return;
    
    Member* with = MemberUtils::get("members_list", msg.sender());
    if(!with->isValid())
        with = msg.sender();
    MessageDialog *dlg = new MessageDialog(with, this);
    dlg->setModal(false);
    dlg->show();
    dlg->incomingMessage(msg);
}