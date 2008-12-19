#include <QFileInfo>
#include <QDateTime>

#include "filehandler.h"

FileHandler* handler = NULL;

FileHandler* fileHandler()
{
    if(!handler)
        handler = new FileHandler;
    return handler;
}

FileHandler::FileHandler() : QObject() {
    m_id = 0 ;
    
    m_server = new QTcpServer(this);
    m_server->listen(QHostAddress::Any, UDP_PORT);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(startSendFile()));
}

void FileHandler::sendFilesRequest(QStringList filenames, Member* to, QString message="")
{
    QStringList payload;
    foreach(QString filename, filenames)
    {
        if(QFileInfo(filename).isDir())
        {
            //sendDirectoryRequest(filename, to);
        }
        else
        {
            payload << formatFileData(filename);
        }
    }
    qDebug() << "files "<<payload.join("").toAscii();
    QByteArray out = message.toAscii() + '\0' + payload.join("").toAscii();
    messenger()->sendMessage(QOM_SENDMSG | QOM_FILEATTACHOPT | QOM_SENDCHECKOPT, out, to);
}

QString FileHandler::formatFileData(QString filename)
{
    QFileInfo info(filename);
    if(! info.exists() )
        return "";
    
    QStringList data;
    data << QString::number(nextId());
    data << info.fileName().replace(':', "::");
    data << QString::number(info.size(), 16);
    data << QString::number(info.lastModified().toTime_t(), 16);
    data << QString::number( info.isDir() ? QOM_FILE_DIR : QOM_FILE_REGULAR, 16);
    
    return data.join(":")+":\a";
}

void FileHandler::startSendFile()
{
    while(m_server->hasPendingConnections())
    {
        // create new thread with the connection
        QTcpSocket* sock = m_server->nextPendingConnection();
        qDebug() << "Began new connection "<<sock->peerAddress()<<sock->peerPort();
    }
}