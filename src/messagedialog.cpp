#include "messagedialog.h"

#include "qommunicate.h"
#include "messenger.h"
#include "constants.h"

MessageDialog::MessageDialog(Member* receiver, QWidget *parent) : QDialog(parent)
{
    receivers << receiver;
    
    ui.setupUi(this);
    setWindowTitle(tr("Conversation: %1").arg(receiver->name()));
    connect(messenger(), SIGNAL(msg_recvMsg(Message)), this, SLOT(incomingMessage(Message)));
    
    ((Qommunicate*)parent)->dialogOpened(receiver);
    
}

MessageDialog::MessageDialog(QList<Member*> receivers, QWidget *parent) : QDialog(parent)
{
    this->receivers = receivers;
    
    ui.setupUi(this);
    
    QStringList titleRecvs;
    Member* t;
    foreach(t, receivers)
        titleRecvs << t->name();
    setWindowTitle(tr("Conversation: %1").arg(titleRecvs.join(",")));
}

MessageDialog::MessageDialog(QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);
    setWindowTitle(tr("Multicast message"));
}

void MessageDialog::closeEvent(QCloseEvent *evt)
{
    if(receivers.size() == 1)
        ((Qommunicate*) parent())->dialogClosed(receivers[0]);
    evt->accept();
}

void MessageDialog::incomingMessage(Message msg)
{
    ui.messageEdit->append(msg.payload());
    messenger()->sendMessage(QOM_RECVMSG, QString::number(msg.packetNo()), receivers[0]);
}