#include "messagedialog.h"

#include "qommunicate.h"
#include "messenger.h"
#include "constants.h"

MessageDialog::MessageDialog(Member* receiver, QWidget *parent) : QDialog(parent)
{
    receivers << receiver;
    
    ui.setupUi(this);
    messageTimer = NULL;
    setWindowTitle(tr("Conversation: %1").arg(receiver->name()));
    connect(messenger(), SIGNAL(msg_recvMsg(Message)), this, SLOT(incomingMessage(Message)));
    connect(messenger(), SIGNAL(msg_recvConfirmMsg(Message)), this, SLOT(messageRecvConfirm()));
    
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
    ui.messageEdit->append(QString("<b style=\"color:blue;\">&lt;%1&gt;</b> %2\n").arg(receivers[0]->name()).arg(msg.payload().trimmed()));
    messenger()->sendMessage(QOM_RECVMSG, QString::number(msg.packetNo()), receivers[0]);
}

void MessageDialog::on_sendButton_clicked()
{
    Member* m;
    foreach(m, receivers)
    {
        messenger()->sendMessage(QOM_SENDMSG, ui.messageInput->text(), m);
    }
    if(receivers.size() == 1)
    {
        if(messageTimer != NULL)
            delete messageTimer;
        messageTimer = new QTimer(this);
        messageTimer->setSingleShot(true);
        connect(messageTimer, SIGNAL(timeout()), this, SLOT(messageTimeout()));
        
        ui.messageInput->setEnabled(false);
        messageTimer->start(5000);
    }
    else
    {
        messageRecvConfirm();
        QTimer::singleShot(500, this, SLOT(accept()));
    }
        
}

void MessageDialog::messageTimeout()
{
    if(QMessageBox::warning(this, 
            tr("Sending Failed"),
            tr("Failed to send message. To <b>retry</b> click Ok"),
            QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
    {
        on_sendButton_clicked();
    }
    else
        ui.messageInput->setEnabled(true);
}

void MessageDialog::messageRecvConfirm()
{
    ui.messageEdit->append(QString("<b style=\"color:red;\">&lt;%1&gt;</b> %2").arg(me().name()).arg(ui.messageInput->text()));
    
    ui.messageInput->clear();
    ui.messageInput->setEnabled(true);
    
    if(receivers.size() == 1 && messageTimer != NULL)
        messageTimer->stop();
}