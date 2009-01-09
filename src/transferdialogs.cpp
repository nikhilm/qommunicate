#include "transferdialogs.h"

#include <QMessageBox>

void FileSendProgressDialog::startSend(int socketDescriptor)
{
    qDebug() << "FileSendProgressDialog::startSend Got new descriptor";
    if(m_fst != NULL)
    {
        qDebug() << "Not null";
        return;
    }
    qDebug() << "Creating thread" ;
    
    m_fst = new FileSendThread(socketDescriptor);
    connect(m_fst, SIGNAL(connectionError(QString)), this, SLOT(error(QString)));
    connect(m_fst, SIGNAL(sendDone()), this, SLOT(accept()));
    connect(m_fst, SIGNAL(notifyProgress(int)), this, SLOT(setValue(int)));
    connect(m_fst, SIGNAL(sendingNextFile(QString)), this, SLOT(setRange(0, 100)));
    connect(m_fst, SIGNAL(sendingNextFile(QString)), this, SLOT(setLabelText(QString)));
}

void FileSendProgressDialog::sendFiles()
{
    m_fst->sendFiles();
}

void FileSendProgressDialog::error(QString e)
{
    QMessageBox::critical(this, tr("Error opening connection"), tr("Error: %1").arg(e));
    reject();
}

void FileSendProgressDialog::accept()
{
    qDebug() << "Accept called";
    delete m_fst;
    m_fst = NULL;
    qDebug() << m_fst;
    m_filesSent--;
    qDebug() << "Files remaining" << m_filesSent;
    if(m_filesSent == 0)
    {
        qDebug() << "All files sent";
        QProgressDialog::accept();
    }
    else
    {
        setLabelText(tr("Waiting for %1 to accept").arg(m_to->name()));
        setRange(0, 0);
    }
}