#ifndef QOM_SEND_FILE_DIALOG
#define QOM_SEND_FILE_DIALOG

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
        setMinimumDuration(1000);
        setRange(0, 0);
        
        m_socket = sock;
        m_to = receiver();
        
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
    void acceptFilePath(QString);
    
private slots:
    void error(QAbstractSocket::SocketError);
    void updateStatus(qint64);
    void nextFileRequested();
    
private:
    //QStringList m_files;
    Member* m_to;
    QTcpSocket* m_socket;
    
    int m_fileSize;
    int m_offset;
    int m_totalSent;
    
    void initialise();
    bool writeBlock(QByteArray);
    void writeFile(QTcpSocket*, QFile*);
    void sendFile(const QString&, int);
    void sendDir(const QString&);
    
    Member* receiver()
    {
        return MemberUtils::get("members_list", m_socket->peerAddress().toString());
    }
};

#endif