#include "messagedialog.h"

#include "qommunicate.h"
#include "messenger.h"
#include "constants.h"
#include "sendfileprogressdialog.h"

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

void MessageDialog::reject()
{
    ((Qommunicate*) parent())->dialogClosed(receivers[0]);
    QDialog::reject();
}

void MessageDialog::incomingMessage(Message msg)
{
    if(msg.sender()->addressString() != receivers[0]->addressString())
        return;
    
    QString text = QString("<b style=\"color:blue;\">&lt;%1&gt;</b> ").arg(receivers[0]->name());
    text += msg.payload().replace('\a', "").trimmed();
    ui.messageEdit->append(text);
    
    if(msg.command() & QOM_SENDCHECKOPT)
        messenger()->sendMessage(QOM_RECVMSG, QByteArray::number(msg.packetNo()), receivers[0]);
}

void MessageDialog::on_sendButton_clicked()
{
    if(ui.messageInput->text().trimmed().isEmpty())
        return;
    
    if(receivers.isEmpty())
    {
        messenger()->multicast(QOM_SENDMSG|QOM_BROADCASTOPT, ui.messageInput->text().toAscii());
        QTimer::singleShot(500, this, SLOT(accept()));
        return;
    }
    
    foreach(Member* m, receivers)
    {
        messenger()->sendMessage(
            QOM_SENDMSG|QOM_SENDCHECKOPT |
            ( ui.notifyReadCB->isChecked() ? QOM_SECRETOPT | QOM_READCHECKOPT : 0 ),
            //( ui.sealCB->isChecked() ? QOM_SECRETOPT : 0 ) ,
            ui.messageInput->text().toAscii(), m);
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
    if(! ui.messageInput->text().trimmed().isEmpty())
        ui.messageEdit->append(QString("<b style=\"color:red;\">&lt;%1&gt;</b> %2").arg(me().name()).arg(ui.messageInput->text()));
    
    ui.messageInput->clear();
    ui.messageInput->setEnabled(true);
    
    if(receivers.size() == 1 && messageTimer != NULL)
        messageTimer->stop();
    
    ui.messageInput->setFocus();
}

void MessageDialog::dropEvent(QDropEvent *evt)
{
    QStringList files;
    foreach(QUrl url, evt->mimeData()->urls())
    {
        files << url.toLocalFile();
    }
    
    foreach(Member* to, receivers)
    {
        //new FileSendProgressDialog(files, to, ui.messageInput->text());
        fileUtils()->sendFilesUdpRequest(files, to, ui.messageInput->text());
    }
}
