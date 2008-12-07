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
    
    populateTree();
    
    firstRun();
    messenger()->login();
    
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanup()));
    
    connect(messenger(), SIGNAL(msg_ansEntry(Message)), this, SLOT(addMember(Message)));
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
    model->item(0)->appendRow( new QStandardItem(QString("member 5")));
    model->item(0)->appendRow( new QStandardItem(QString("nikhil")));
    
    filterModel->setSourceModel(model);
    filterModel->setDynamicSortFilter(true);
    
    ui.memberTree->setSelectionMode(ui.memberTree->ExtendedSelection);
    ui.memberTree->setModel(filterModel);
    ui.memberTree->setHeaderHidden(true);
    ui.memberTree->expandAll();
}

void Qommunicate::on_memberTree_doubleClicked(const QModelIndex& index)
{
    QModelIndex item;
    
    MessageDialog *dlg;
    if(index.isValid())
    {
        // only single item clicked
        dlg = new MessageDialog(ui.memberTree->model()->data(index).toString());
    }
    else
    {
        QStringList receivers;
        foreach(item, ui.memberTree->selectionModel()->selectedRows())
            receivers << ui.memberTree->model()->data(item).toString() ;
        dlg = new MessageDialog(receivers);
    }
    dlg->exec();
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