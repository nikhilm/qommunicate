#include "messagedialog.h"

MessageDialog::MessageDialog(QString receiver, QWidget *parent) : QDialog(parent)
{
    receivers = new QStringList(receiver);
    
    ui.setupUi(this);
    setWindowTitle(tr("Send Message to %1").arg(receiver));
}

MessageDialog::MessageDialog(QStringList receivers, QWidget *parent) : QDialog(parent)
{
    this->receivers = &receivers;
    
    ui.setupUi(this);
    setWindowTitle(tr("Send message to %1").arg(receivers.join(",")));
}

MessageDialog::MessageDialog(QWidget *parent) : QDialog(parent)
{
    this->receivers = NULL; // broadcast
    
    ui.setupUi(this);
    setWindowTitle(tr("Multicast message"));
}