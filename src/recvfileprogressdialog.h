#ifndef QOM_RECV_FILE_DIALOG
#define QOM_RECV_FILE_DIALOG

#include <QProgressDialog>
#include <QTcpSocket>

#include "messenger.h"

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
        
        informUser();
    }
    
    ~RecvFileProgressDialog()
    {
        if(m_socket != NULL)
            m_socket->close();
    }
    
private slots:
    void error(QAbstractSocket::SocketState);
    void requestFiles();
    void informUser();
    
private:
    Message m_msg;
    QTcpSocket* m_socket;
    int m_requestType;
    QString m_saveDir;
    
    QList<RecvFileInfo> parsePayloadFileList(QByteArray);
    void startReceiving();
    bool writeBlock(QByteArray);
};

#endif