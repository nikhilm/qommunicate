#include <QFileInfo>
#include <QDateTime>

#include "fileutils.h"
#include "filethreads.h"

FileUtils* utils = NULL;

FileUtils* fileUtils()
{
    if(!utils)
        utils = new FileUtils;
    return utils;
}

FileUtils::FileUtils() : QObject() {
    m_id = 0 ;
    
    m_server = new FileTcpServer(this);
    m_server->listen(QHostAddress::Any, UDP_PORT);
    connect(m_server, SIGNAL(incomingConnectionDescriptor(int)), this, SIGNAL(incomingTcpConnection(int)));
}

QString FileUtils::formatSendFilesRequest(QStringList filenames)
{
    QStringList payload;
    foreach(QString filename, filenames)
    {
        if(QFileInfo(filename).isDir())
        {
            payload << formatFileData(filename);
        }
        else
        {
            payload << formatFileData(filename);
        }
    }
    return payload.join("");
}

QString FileUtils::formatFileData(QString filename)
{
    QFileInfo info(filename);
    if(! info.exists() )
        return "";
    
    int id = nextId();
    m_fileIdHash.insert(id, info.absoluteFilePath());
    
    QStringList data;
    
    data << QString::number(id);
    data << info.fileName().replace(':', "::");
    data << QString::number(info.size(), 16);
    data << QString::number(info.lastModified().toTime_t(), 16);
    data << QString::number( info.isDir() ? QOM_FILE_DIR : QOM_FILE_REGULAR, 16);
    
    return data.join(":")+":\a";
}

// FileSendThread* FileUtils::startSendFile()
// {
//     while(m_server->hasPendingConnections())
//     {
//         // create new thread with the connection
//         QTcpSocket* sock = m_server->nextPendingConnection();
//         FileSendThread* fst = new FileSendThread(sock);
//         connect(fst, SIGNAL(requestFilePath(int)), this, SLOT(resolveFilePath(int)));
//         connect(this, SIGNAL(filePath(QString)), fst, SLOT(acceptFilePath(QString)));
//         emit peerAccepted(fst);
//         //fst->start();
//     }
// }

void FileUtils::resolveFilePath(int id)
{
    emit filePath(m_fileIdHash[id]);
}