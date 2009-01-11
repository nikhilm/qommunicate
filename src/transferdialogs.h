#ifndef QOM_TRANSFERDIALOGS
#define QOM_TRANSFERDIALOGS

#include <QProgressDialog>
#include <QTcpSocket>
#include <QFile>

#include "fileutils.h"
#include "memberutils.h"

class FileSendProgressDialog : public QProgressDialog
{
    Q_OBJECT
public:
    FileSendProgressDialog(QTcpSocket* sock, QWidget* parent=0) : 
        QProgressDialog("", tr("&Cancel"), 0, 100, parent)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        setRange(0, 0);
        
        m_currentFile = NULL;
        m_socket = sock;
        m_to = receiver();
        
        initialise();
        exec();
    }
    
    ~FileSendProgressDialog()
    {
        qDebug() << "Destroying dialog";
//         if(m_socket != NULL)
//         {
//             m_socket->close();
//             delete m_socket;
//             m_socket = NULL;
//         }
//         qDebug() << "Deleted socket";
        if(m_currentFile != NULL)
        {
            m_currentFile->close();
            delete m_currentFile;
            m_currentFile = NULL;
        }
    };
    
signals:
    void requestFilePath(int);
    
public slots:
    void accept();
    void acceptFilePath(QString);
    
private slots:
    void error(QAbstractSocket::SocketError);
    void updateStatus(qint64);
    void nextFileRequested();
    
private:
    //QStringList m_files;
    Member* m_to;
    QTcpSocket* m_socket;
    
    QFile* m_currentFile;
    int m_offset;
    int m_totalSent;
    
    void initialise();
    void writeOut(QTcpSocket*, QFile*);
    
    Member* receiver()
    {
        return MemberUtils::get("members_list", m_socket->peerAddress().toString());
    }
};

#endif