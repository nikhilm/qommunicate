#include <QFileInfo>
#include <QDateTime>
#include <QDir>

#include "fileutils.h"

FileUtils* utils = NULL;

FileUtils* fileUtils()
{
    if(!utils)
        utils = new FileUtils;
    return utils;
}

FileUtils::FileUtils() : QObject() {
    m_id = 0 ;
    
    m_server = new QTcpServer(this);
    m_server->listen(QHostAddress::Any, UDP_PORT);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

QString FileUtils::formatSendFilesRequest(QStringList filenames)
{
    QStringList payload;
    foreach(QString filename, filenames)
    {
        int id = nextId();
        m_fileIdHash.insert(id, filename);
        payload << (QString::number(id)+":"+formatFileData(filename)) ;
    }
    return payload.join("");
}

QString FileUtils::formatFileData(QString filename)
{
    QFileInfo info(filename);
    if(! info.exists() )
        return "";
    if(! info.isReadable())
        return "";
    
    QStringList data;
    
    data << info.fileName().replace(':', "::");
    data << QString::number( info.isDir() ? 0 : info.size(), 16);
    data << QString::number(info.lastModified().toTime_t(), 16);
    data << QString::number( info.isDir() ? QOM_FILE_DIR : QOM_FILE_REGULAR, 16);
    
    return data.join(":")+":\a";
}

/*
Returns header-size:filename:file-size:fileattr[:extend-attr=val1
    [,val2...][:extend-attr2=...]]: for each file.
    Caller should tack on content-data
*/
QStringList FileUtils::formatHeirarchialTcpRequest(const QString& dirName)
{
    QStringList headers;
    QDir dir(dirName);
    QStringList entries = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    entries.prepend(dirName);
    entries << ".";
    QString fileName;
    foreach(fileName, entries)
    {
        QStringList formattedData = formatFileData(dir.absoluteFilePath(fileName)).split(':');
        if(formattedData.isEmpty())
            continue;
        if(formattedData.last() == "\a")
        {
            formattedData.removeLast(); //remove \a at the end
            formattedData.append("");
        }
        formattedData.removeAt(2); //drop mtime
        formattedData.prepend(""); // a colon at the front to simplify things
        if(fileName == ".")
            formattedData.replace(3, QString::number(QOM_FILE_RETPARENT));
        QString headerData = formattedData.join(":");
        int headerSize = headerData.length();
        headerSize += QString::number(headerSize, 16).length();
        headers << ( QString::number(headerSize, 16) + headerData ) ;
        
    }
    return headers;
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
    qDebug() << "resolveFilePath: requested"<<id<<"which is"<<m_fileIdHash[id];
    emit filePath(m_fileIdHash[id]);
}

void FileUtils::sendFilesUdpRequest(QStringList files, Member* to, QString msg="")
{
    qDebug() << "UDP request" << files ;
    QByteArray out = msg.toAscii() + '\0' + formatSendFilesRequest(files).toAscii();
    messenger()->sendMessage(QOM_SENDMSG | QOM_FILEATTACHOPT | QOM_SENDCHECKOPT, out, to);
}

void FileUtils::newConnection()
{
    emit newFileSendSocket(m_server->nextPendingConnection());
}