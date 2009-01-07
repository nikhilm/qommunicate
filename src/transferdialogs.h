#ifndef QOM_TRANSFERDIALOGS
#define QOM_TRANSFERDIALOGS

#include <QProgressDialog>

#include "filethreads.h"
#include "fileutils.h"

class FileSendProgressDialog : public QProgressDialog
{
    Q_OBJECT
public:
    FileSendProgressDialog(QStringList files, Member* to, const QString& msg, QWidget* parent=0) : 
        QProgressDialog(tr("Waiting for %1 to accept").arg(to->name()), tr("&Cancel"), 0, 100, parent)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        
        //m_files = files;
        m_to = to;
        m_fst = NULL;
        
        QByteArray out = msg.toAscii() + '\0' + fileUtils()->formatSendFilesRequest(files).toAscii();
        messenger()->sendMessage(QOM_SENDMSG | QOM_FILEATTACHOPT | QOM_SENDCHECKOPT, out, m_to);
        qDebug() << "Sent UPD request for send" ;
        connect(fileUtils(), SIGNAL(incomingTcpConnection(int)), this, SLOT(startSend(int)));        
    }
    
public slots:
    void accept();
    
private slots:
    void startSend(int);
    void sendFiles();
    void error(QString);
    
private:
    //QStringList m_files;
    Member* m_to;
    FileSendThread* m_fst;
};

#endif