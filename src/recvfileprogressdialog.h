/*
 * (C) 2009 Nikhil Marathe <nsm.nikhil@gmail.com>
 * Licensed under the GNU General Public License.
 * See LICENSE for details
 */
#ifndef QOM_RECV_FILE_DIALOG
#define QOM_RECV_FILE_DIALOG

#include <QProgressDialog>
#include <QTcpSocket>
#include <QDir>
#include <QMessageBox>

#include "messenger.h"
#include "fileutils.h"

class QFile;

class RecvFileProgressDialog : public QProgressDialog
{
    Q_OBJECT
public:
    RecvFileProgressDialog(Message& msg, QWidget* parent=0) :
        QProgressDialog("", tr("&Cancel"), 0, 100, 0), m_msg(msg)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        setMinimumDuration(0);
        
        m_socket = NULL;
        m_currentFile = NULL;
        m_dir = NULL;
        
        m_fileHeaders = parsePayloadFileList(m_msg.payload());
        m_waitingForData = 0;
        m_inHeader = true;
        
        informUser();
    }
    
    ~RecvFileProgressDialog()
    {
        if (m_socket != NULL)
        {
            m_socket->close();
            m_socket->deleteLater();
        }
        if (m_currentFile != NULL)
        {
            m_currentFile->close();
            delete m_currentFile;
            m_currentFile = NULL;
        }
        if (m_dir != NULL)
        {
            delete m_dir;
            m_dir = NULL;
        }
    }
    
signals:
    void downloadDone(QString);
    void allDownloadsDone(QString);
    
private slots:
    void error(QAbstractSocket::SocketError);
    void requestFiles();
    bool informUser();
    void readRequest();
    void userInformed(QAbstractButton*);
    
private:
    QMessageBox *m_notifyDialog;
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
    
    QList<FileInfo> m_fileHeaders;
    
    QList<FileInfo> parsePayloadFileList(QByteArray);
    void startReceiving();
    bool writeBlock(QByteArray);
    bool writeToFile(QByteArray&, QByteArray* remainder=NULL);
    bool openFile(const QString&, bool askOverwrite=true);
    bool makeDirectory(const QString&);
    bool writeToDirectory(QByteArray&);
    FileInfo parseDirectoryHeader(const QByteArray&, QByteArray*);
    void requestWriteToFile();
    void requestWriteToDirectory();
};

#endif
