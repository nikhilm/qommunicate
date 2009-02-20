#ifndef QOM_SEND_FILE_DIALOG
#define QOM_SEND_FILE_DIALOG

#include <QProgressDialog>
#include <QTcpSocket>
#include <QFile>
#include <QQueue>

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
        setMinimumDuration(1000);
        setRange(0, 0);
        
        m_socket = sock;
        m_to = receiver();
        m_totalSent = m_fileSize = 0;
        m_writingData = false;
        
        initialise();
        exec();
    }
    
    ~FileSendProgressDialog()
    {
        qDebug() << "Destroying dialog";
        if(m_socket != NULL)
        {
            m_socket->close();
        }
        qDebug() << "Deleted socket";
    };
    
signals:
    void requestFilePath(int);
    
public slots:
    void accept();
    
private slots:
    void error(QAbstractSocket::SocketError);
    void updateStatus(qint64);
    void nextFileRequested();
    
private:
    //QStringList m_files;
    Member* m_to;
    QTcpSocket* m_socket;
    
    bool m_writingData;
    QList<FileInfo> m_fileList;
    
    int m_fileSize;
    int m_totalSent;
    
    QByteArray m_sendBuffer;
    
    void initialise();
    bool writeBlock(QByteArray);
    void writeFile(QTcpSocket*, QFile*);
    void sendFile(const FileInfo&);
    void sendDir(const FileInfo&);
    FileInfo parseIncomingFileRequest(const QByteArray&);
    
    Member* receiver()
    {
        return MemberUtils::get("members_list", m_socket->peerAddress().toString());
    }
};

#endif