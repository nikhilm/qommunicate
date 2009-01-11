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
    FileSendThread(int socketDescriptor, QObject* parent=0);
    
    ~FileSendThread() {
        
    
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
    void sendDone();
    void fileSent(QString);
    
public slots:
    void acceptFilePath(QString);
    void nextFileRequested();
    void sendFiles();
    
private slots:
    void socketError(QTcpSocket::SocketError) ;
    void updateProgress(qint64);
    
private:
    int m_descriptor;
    QTcpSocket* m_socket;
    //Member* m_member;
    
    QFile* m_file;
    qint64 m_offset;
    
    qint64 m_totalSent;
    
    QTimer* m_timer;
    
    //QProgressDialog(s
    void writeOut(QTcpSocket*, QFile* );
    void initTimer();
    
};

#endif