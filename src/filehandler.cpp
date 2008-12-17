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