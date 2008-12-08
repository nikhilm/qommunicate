#include "messagedialog.h"

#include "qommunicate.h"

MessageDialog::MessageDialog(Member* receiver, QWidget *parent) : QDialog(parent)
{
    receivers << receiver;
    
    ui.setupUi(this);
    setWindowTitle(tr("Send Message to %1").arg(receiver->name()));
    
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
    setWindowTitle(tr("Send message to %1").arg(titleRecvs.join(",")));
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