#ifndef QOM_RECV_FILE_DIALOG
#define QOM_RECV_FILE_DIALOG

#include <QProgressDialog>
#include <QTcpSocket>
#include <QDir>

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
        setMinimumDuration(1000);
        
        m_socket = NULL;
        m_currentFile = NULL;
        m_dir = NULL;
        
        m_fileHeaders = parsePayloadFileList(m_msg.payload());
        m_waitingForData = 0;
        m_inHeader = true;
        
        informUser();
        requestFiles();
        //startReceiving();
    }
    
    ~RecvFileProgressDialog()
    {
        if(m_socket != NULL)
            m_socket->close();
        qDebug() << "RecvFileProgressDialog destroyed";
    }
    
public slots:
    void accept();
    
private slots:
    void error(QAbstractSocket::SocketError);
    void requestFiles();
    void informUser();
    void readRequest();
    
private:
    Message m_msg;
    QTcpSocket* m_socket;
    int m_requestType;
    QString m_saveDir;
    
    /*
     * Used for file writing
     */
    //NOTE: this is decremented as data is read, when it reaches 0, the next file request can be sent
    int m_waitingForData;
    int m_currentSize;
    QFile* m_currentFile;
    
    /*
     * Used for directories
     */
    bool m_inHeader;
    QDir* m_dir;
    QByteArray m_header;
    
    QList<RecvFileInfo> m_fileHeaders;
    
    QList<RecvFileInfo> parsePayloadFileList(QByteArray);
    void startReceiving();
    bool writeBlock(QByteArray);
    bool writeToFile(QByteArray&, QByteArray* remainder=NULL);
    bool openFile(const QString&);
    bool makeDirectory(const QString&);
    bool writeToDirectory(QByteArray&);
    RecvFileInfo parseDirectoryHeader(const QByteArray&, QByteArray*);
    void requestWriteToFile();
    void requestWriteToDirectory();
};

#endif