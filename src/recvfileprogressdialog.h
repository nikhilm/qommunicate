#ifndef QOM_RECV_FILE_DIALOG
#define QOM_RECV_FILE_DIALOG

#include <QProgressDialog>
#include <QTcpSocket>

#include "messenger.h"

class QFile;

struct RecvFileInfo
{
    QString fileName;
    int fileID;
    qint64 size;
    uint mtime;
    QHash<QString,QString> xattrs;
    int type;
};

class RecvFileProgressDialog : public QProgressDialog
{
    Q_OBJECT
public:
    RecvFileProgressDialog(Message& msg, QWidget* parent=0) :
        QProgressDialog("", tr("&Cancel"), 0, 100, parent), m_msg(msg)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        
        m_socket = NULL;
        
        m_fileHeaders = parsePayloadFileList(m_msg.payload());
        m_waitingForData = 0;
        informUser();
        requestFiles();
        //startReceiving();
    }
    
    ~RecvFileProgressDialog()
    {
        if(m_socket != NULL)
            m_socket->close();
    }
    
private slots:
    void error(QAbstractSocket::SocketError);
    void requestFiles();
    void informUser();
    void readRequest();
    
private:
    Message m_msg;
    QTcpSocket* m_socket;
    int m_requestType;
    //NOTE: this is decremented as data is read, when it reaches 0, the next file request can be sent
    int m_waitingForData;
    QString m_saveDir;
    QFile* m_currentFile;
    
    QList<RecvFileInfo> m_fileHeaders;
    
    QList<RecvFileInfo> parsePayloadFileList(QByteArray);
    void startReceiving();
    bool writeBlock(QByteArray);
    bool writeToFile(QByteArray&);
    bool openFile(const QString&);
};

#endif