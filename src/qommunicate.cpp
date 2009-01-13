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
#include "fileutils.h"
#include "sendfileprogressdialog.h"
#include "recvfileprogressdialog.h"
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
    connect(messenger(), SIGNAL(msg_exit(Message)), this, SLOT(removeMember(Message)));
    connect(messenger(), SIGNAL(msg_recvMsg(Message)), this, SLOT(openDialog(Message)));
    connect(messenger(), SIGNAL(msg_getAbsenceInfo(Message)), this, SLOT(sendAbsenceInfo(Message)));
    
    
    connect(fileUtils(), SIGNAL(newFileSendSocket(QTcpSocket*)), this, SLOT(fileSendRequested(QTcpSocket*)));
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
    MessageDialog dlg(this);
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
    
    filterModel->setSourceModel(model);
    filterModel->setDynamicSortFilter(true);
    filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    
    ui.memberTree->setSelectionMode(ui.memberTree->ExtendedSelection);
    ui.memberTree->setModel(filterModel);
    ui.memberTree->setHeaderHidden(true);
    ui.memberTree->setAcceptDrops(true);
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
    messenger()->multicast(QOM_BR_ABSENCE, me().name().toAscii()+'\0'+myGroup().name().toAscii());
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
    if(MemberUtils::contains("members_list", msg.sender()))
        return;
    
    QList<QByteArray> tokens = msg.payload().split('\a');
    msg.sender()->setName(tokens[0]);
    
    QString groupName;
    if(tokens.size() > 1)
        groupName = tokens[1];
    
    if(groupName.isEmpty())
    {
        model->appendRow(msg.sender());
    }
    else
    {
        QList<QStandardItem*> matches = model->findItems(groupName);
        if(!matches.isEmpty() && matches[0]->type() == TYPE_GROUP)
        {
            matches[0]->appendRow(msg.sender());
        }
        else
        {
            Group * group = new Group(groupName);
            group->appendRow(msg.sender());
            model->appendRow(group);
        }
    }
    
    MemberUtils::insert("members_list", msg.sender());
    ui.memberTree->expandAll();
}

void Qommunicate::addMemberAndAnswer(Message msg)
{
    addMember(msg);
    messenger()->sendMessage(QOM_ANSENTRY, (me().name()+'\0'+myGroup().name()).toAscii(), msg.sender());
}

void Qommunicate::removeMember(Message msg)
{
    QList<QByteArray> tokens = msg.payload().split('\a');
    QString groupName;
    if(tokens.size() > 1)
        groupName = tokens[1];
    
    for(int i = 0; i < model->rowCount(); i++)
    {
        QStandardItem* it = model->item(i, 0);
        if(it->type() == TYPE_MEMBER && ((Member*)it)->name() == msg.sender()->addressString())
        {
            model->removeRow(it->row());
        }
        else
        {
            for(int j = 0; j < it->rowCount(); j++)
            {
                if(((Member*)it->child(j))->addressString() == msg.sender()->addressString())
                    it->removeRow(j);
            }
            if(it->rowCount() == 0)
                model->removeRow(it->row());
        }        
    }
    MemberUtils::remove("members_list", msg.sender()->addressString());
}

void Qommunicate::openDialog(Message msg)
{
    if(MemberUtils::contains("open_conversations", msg.sender()))
        return;
    
    // if set to ignore received messages
    QSettings s;
    if(s.value(tr("no_receive")).toBool())
    {
        qWarning() << "Ignoring message" << msg.payload() << "from" << msg.sender()->name();
        if(msg.command() & QOM_SENDCHECKOPT)
            messenger()->sendMessage(QOM_RECVMSG, QByteArray::number(msg.packetNo()), msg.sender());
        return;
    }
    
    Member* with = MemberUtils::get("members_list", msg.sender());
    if(!with->isValid())
        with = msg.sender();
    MessageDialog *dlg = new MessageDialog(with);
    dlg->setModal(false);
    dlg->show();
    dlg->incomingMessage(msg);
}

void Qommunicate::sendAbsenceInfo(Message msg)
{
    QString payload = ui.statusCombo->currentText();
    if(payload == tr("Available"))
        payload = "Not absence mode";
    messenger()->multicast(QOM_SENDABSENCEINFO, payload.toAscii());
}

void Qommunicate::fileSendRequested(QTcpSocket* sock)
{
    qDebug() << "\n\nReceived new connection";
    new FileSendProgressDialog(sock);
}
        