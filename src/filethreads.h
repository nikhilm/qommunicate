#ifndef QOM_FILETHREADS
#define QOM_FILETHREADS

#include <QTcpSocket>
#include <QFile>
#include <QThread>

#include "ipobjects.h"
#include "memberutils.h"
#include "fileutils.h"

class FileSendThread : public QThread
{
    Q_OBJECT
    
public:
    FileSendThread(int socketDescriptor, QObject* parent=0) : QThread(parent)
    {        
        m_file = NULL;
        
        m_socket = new QTcpSocket;
        if(!m_socket->setSocketDescriptor(socketDescriptor)) {
            emit socketError(m_socket->error());
            return;
        }
        qDebug() << "FileSendThread: socket setup";
        connect(m_socket, SIGNAL(readyRead()), this, SLOT(nextFileRequested()));  
        connect(m_socket, SIGNAL(disconnected()), this, SLOT(done()));
        connect(m_socket, SIGNAL(disconnected()), m_socket, SLOT(deleteLater()));
        connect(m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(updateProgress(qint64)));
        
        connect(this, SIGNAL(requestFilePath(int)), fileUtils(), SLOT(resolveFilePath(int)));
        connect(fileUtils(), SIGNAL(filePath(QString)), this, SLOT(acceptFilePath(QString)));
        
        start();
        
        //setMember(MemberUtils::get("members_list", m_socket->peerAddress().toString()));
        //emit requestMemberName(m_socket->peerAddress().toString());
    } ;
    
    ~FileSendThread() { };
    
    void run();
    
//     void setMember(Member* m)
//     {
//         //m_member = m;
//     } ;
    
signals:
    void requestMemberName(QString);
    void notifyProgress(int);
    void requestFilePath(int);
    void readyToSend();
    void connectionError(QString);
    void sendingNextFile(QString);
    
public slots:
    void acceptFilePath(QString);
    void nextFileRequested();
    void sendFiles();
    void done();
    
private slots:
    void socketError(QTcpSocket::SocketError) ;
    void updateProgress(qint64);
    
private:
    QTcpSocket* m_socket;
    //Member* m_member;
    
    QFile* m_file;
    qint64 m_offset;
    
    qint64 m_totalSent;
    
    //QProgressDialog(s
    
};

#endif