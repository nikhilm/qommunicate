#include "filehandler.h"

void FileHandler::sendFilesRequest(QStringList filenames, Member* to, QString message="")
{
    foreach(QString filename, filenames)
    {
        if(QFileInfo(filename).isDir())
        {
            sendDirectoryRequest(filename, to);
        }
        else
        {
            QStringList payload(message);
        }
    }
}