#include "sendfileprogressdialog.h"

#include <QFileInfo>
#include <QDir>

void FileSendProgressDialog::initialise()
{
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
    connect(m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(updateStatus(qint64)));
    //connect(m_socket, SIGNAL(connected()), this, SLOT(sendFiles()));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(nextFileRequested()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(accept()));
    connect(m_socket, SIGNAL(disconnected()), m_socket, SLOT(deleteLater()));
    connect(this, SIGNAL(attemptSend()), this, SLOT(sendFiles()));
    connect(this, SIGNAL(canceled()), this, SLOT(reject()));
}

void FileSendProgressDialog::error(QAbstractSocket::SocketError e)
{
    qDebug() << "Socket error" << m_socket->errorString();
    
}

void FileSendProgressDialog::accept()
{
    qDebug() << "Accept called";
    
    setLabelText(tr("Waiting for %1 to accept").arg(m_to->name()));
    setRange(0, 0);
    QProgressDialog::accept();
}

void FileSendProgressDialog::updateStatus(qint64 bytes)
{
    m_totalSent += bytes;
    setValue( (m_totalSent * 100.0)/ m_fileSize );
    if(m_totalSent == m_fileSize);
    {
        // we can now start a new transfer
        m_writingData = false;
        nextFileRequested();
    }
}

/*
 * FileInfo.type = -1 if there was an error parsing the header
 */
FileInfo FileSendProgressDialog::parseIncomingFileRequest(const QByteArray& b)
{
    FileInfo info;
    
    QList<QByteArray> tokens = b.split(':');
    if(tokens.size() < 8)
    {
        qDebug() << "Bad header" << b;
        info.type = -1;
    }
    else
    {
        int command = tokens[4].toInt();
        info.fileID = tokens[6].toInt(0, 16);
        
        // for now we just store the file name, later we'll resolve the path
        info.fileName = QFileInfo(fileUtils()->resolveFilePath(info.fileID)).fileName();
        
        if(command == QOM_GETDIRFILES)
            info.type = QOM_FILE_DIR;
        else if(command == QOM_GETFILEDATA)
        {
            info.type = QOM_FILE_REGULAR;
            info.offset = tokens[7].toInt();
        }
    }
    return info;
}

void FileSendProgressDialog::nextFileRequested()
{
    if(m_socket->bytesAvailable())
    {
        FileInfo next = parseIncomingFileRequest(m_socket->readAll());
        if(next.fileID != -1)
            m_fileList << next;
    }
    
    if(!m_fileList.empty())
    {   
        if(m_writingData)
            return;
        
        FileInfo first = m_fileList.takeFirst();
        qDebug() << first.fileName << first.type;
        if(first.type == QOM_FILE_REGULAR)
            sendFile(first);
        else if(first.type == QOM_FILE_DIR)
            sendDir(first);
        else if(first.type == QOM_FILE_RETPARENT)
        {
            qDebug() << fileUtils()->formatFileHeader(first).toAscii();
            writeBlock(fileUtils()->formatFileHeader(first).toAscii());
        }
    }
    else
    {
        qDebug() << "We're done sending";
        accept();
    }
}

void FileSendProgressDialog::sendFile(const FileInfo& info)
{
    QFile f( info.fileID == -1 ? info.fileName : fileUtils()->resolveFilePath(info.fileID) );
    if(!f.open(QIODevice::ReadOnly))
    {
        qWarning() << "Could not open file for reading" << f.fileName()<<f.errorString();
        return;
    }
    m_fileSize = f.size();
    f.seek(info.offset);
    setRange(0, 100);
    setLabelText(tr("Sending %1").arg(f.fileName()));
    
    // we do the whole splitting thing, since when sending the header
    // we don't want the whole path
    FileInfo copy = info;
    if(info.fileID == -1)
        copy.fileName = info.fileName.split(QDir::separator()).last();
    
    qDebug() << fileUtils()->formatFileHeader(copy).toAscii();
    writeBlock(fileUtils()->formatFileHeader(copy).toAscii());
    m_writingData = true;
    writeFile(m_socket, &f);
}

void FileSendProgressDialog::sendDir(const FileInfo& info)
{
    QList<FileInfo> headers = fileUtils()->directoryInfoList(
                            info.fileID == -1 ? info.fileName : fileUtils()->resolveFilePath(info.fileID));
    qDebug() << fileUtils()->formatFileHeader(headers.takeFirst()).toAscii();
    
    FileInfo copy = info;
    if(info.fileID == -1)
        copy.fileName = info.fileName.split(QDir::separator()).last();
    writeBlock(fileUtils()->formatFileHeader(copy).toAscii());
    
    // this swap is to prepend the new sub directory entries
    // so that they follow the directory name
    headers += m_fileList;
    m_fileList = headers;
}

void FileSendProgressDialog::writeFile(QTcpSocket* sock, QFile* f)
{
    while(!f->atEnd())
    {
        writeBlock(f->read(QOM_FILE_WRITE_SIZE));
    }
    f->close();
}

bool FileSendProgressDialog::writeBlock(QByteArray b)
{
    //qDebug() << "Trying to write: " << b ;
    int bytesToWrite = b.size();
    int bytesWritten = 0;
    do
    {
        bytesWritten = m_socket->write(b);
        //qDebug() << "writeBlock: wrote = " << bytesWritten;
        if(bytesWritten == -1)
            return false;
        b.remove(0, bytesWritten);
        bytesToWrite -= bytesWritten;
    } while(bytesToWrite > 0);
    return true;
}