/*
 * (C) 2009 Nikhil Marathe <nsm.nikhil@gmail.com>
 * Licensed under the GNU General Public License.
 * See LICENSE for details
 */
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
    
    m_online = true;
    setWindowTitle(tr("Conversation: %1").arg(receiver->name()));
    connect(messenger(), SIGNAL(msg_recvMsg(Message)), this, SLOT(incomingMessage(Message)));
    connect(messenger(), SIGNAL(msg_recvConfirmMsg(Message)), this, SLOT(messageRecvConfirm(Message)));
    connect(messenger(), SIGNAL(msg_exit(Message)), this, SLOT(userOffline(Message)));
    connect(messenger(), SIGNAL(msg_entry(Message)), this, SLOT(userOnline(Message)));
    
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
    // no notifications for multicast
    // TODO: is that the right choice?
    ui.notifyReadCB->setEnabled(false);
}

void MessageDialog::reject()
{
    if(receivers.size() == 1)
        ((Qommunicate*) parent())->dialogClosed(receivers[0]);
    QDialog::reject();
}

void MessageDialog::incomingMessage(Message msg)
{
    if(msg.sender()->addressString() != receivers[0]->addressString())
        return;
    
    QString text = QString("<b style=\"color:blue;\">%1 : </b> ")
                        .arg(Qt::escape(receivers[0]->name()));
    text += Qt::escape(msg.payload().replace('\a', "").trimmed());
    ui.messageEdit->append(text);
    QApplication::alert(this, 0);
    
    if(msg.command() & QOM_SENDCHECKOPT)
        messenger()->sendMessage(QOM_RECVMSG, QByteArray::number(msg.packetNo()), msg.sender());
    
    if(msg.command() & QOM_READCHECKOPT)
        messenger()->sendMessage(QOM_READMSG|(msg.command() & QOM_READCHECKOPT), QByteArray::number(msg.packetNo()), msg.sender());
}

void MessageDialog::on_sendButton_clicked()
{
    if(ui.messageInput->text().trimmed().isEmpty())
        return;
    
    if(receivers.isEmpty())
    {
        messenger()->multicast(QOM_SENDMSG|QOM_MULTICASTOPT, ui.messageInput->text().toAscii());
        QTimer::singleShot(500, this, SLOT(accept()));
        return;
    }
    
    int flags = QOM_SENDMSG | QOM_SENDCHECKOPT |
                (receivers.size() > 1 ? QOM_MULTICASTOPT : 0) |
                (ui.notifyReadCB->isChecked() ? QOM_READCHECKOPT | QOM_SECRETOPT : 0);
    
    foreach(Member* m, receivers)
    {
        messenger()->sendMessage( flags, ui.messageInput->text().toAscii(), m);
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

void MessageDialog::messageRecvConfirm(Message msg)
{
    if(msg.sender()->addressString() != receivers[0]->addressString())
        return;
    
    if(! ui.messageInput->text().trimmed().isEmpty())
        ui.messageEdit->append(QString("<b style=\"color:red;\">%1 : </b> %2")
                            .arg(Qt::escape(me().name()))
                            .arg(Qt::escape(ui.messageInput->text())));
    
    ui.messageInput->clear();
    ui.messageInput->setEnabled(true);
    
    if(receivers.size() == 1 && messageTimer != NULL)
        messageTimer->stop();
    
    ui.messageInput->setFocus();
}

void MessageDialog::userOffline(Message msg)
{
    if(!m_online || msg.sender()->addressString() != receivers[0]->addressString())
        return;
    
    ui.messageEdit->append(QString("<b style=\"color:magenta\">%1 went offline</b>").arg(Qt::escape(msg.sender()->name())));
    ui.messageInput->setEnabled(false);
    m_online = false;
}

void MessageDialog::userOnline(Message msg)
{
    if(m_online || msg.sender()->addressString() != receivers[0]->addressString())
        return;
    
    ui.messageEdit->append(QString("<b style=\"color:orange\">%1 came online</b>").arg(Qt::escape(msg.sender()->name())));
    
    ui.messageInput->setEnabled(true);
    m_online = true;
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
