/*
 * (C) 2009 Nikhil Marathe <nsm.nikhil@gmail.com>
 * Licensed under the GNU General Public License.
 * See LICENSE for details
 */

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
    m_socket->abort();
    reject();
    
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
    qDebug() << "updateStatus : sent " << bytes;
//     m_totalSent += bytes;
//     setValue( (m_totalSent * 100.0)/ m_fileSize );
//     if(m_totalSent >= m_fileSize)
//     {
//         // we can now start a new transfer
//         m_writingData = false;
//         qDebug() << "updateStatus : Starting new transfer";
//         nextFileRequested();
//     }
}

/*
 * FileInfo.type = -1 if there was an error parsing the header
 */
FileInfo FileSendProgressDialog::parseIncomingFileRequest(const QByteArray& b)
{
    FileInfo info;
    qDebug() << "parseIncomingFileRequest : " << b;
    QList<QByteArray> tokens = b.split(':');
    if(tokens.size() < 7)
    {
        qDebug() << "Bad header" << tokens;
        info.type = -1;
    }
    else
    {
        int command = tokens[4].toInt();
        info.fileID = tokens[6].toInt(0, 16);
        
        // for now we just store the file name, later we'll resolve the path
        info.fileName = QFileInfo(fileUtils()->resolveFilePath(info.fileID)).fileName();
        
        if((command & QOM_COMMAND_MASK) == QOM_GETDIRFILES)
            info.type = QOM_FILE_DIR;
        else if((command & QOM_COMMAND_MASK) == QOM_GETFILEDATA)
        {
            info.type = QOM_FILE_REGULAR;
            info.offset = tokens[7].toInt();
        }
    }
    return info;
}

void FileSendProgressDialog::nextFileRequested()
{
    //qDebug() << "nextFileRequested : entered"<<m_writingData << (m_fileList.empty() ? "" : m_fileList[0].fileName);
    if(m_socket->bytesAvailable())
    {
        qDebug() << "nextFileRequested : parsing header";
        FileInfo next = parseIncomingFileRequest(m_socket->readAll());
        if(next.fileID != -1)
            m_fileList << next;
        qDebug() << "nextFileRequested : header parse done";
    }
    
    if(!m_fileList.empty())
    {
        if(m_writingData)
        {
            //qDebug() << "nextFileRequested exiting";
            return;
        }
        //qDebug() << "nextFileRequested : attempting send";
        FileInfo first = m_fileList.takeFirst();
        //qDebug() << first.fileName << first.type;
        if(first.type == QOM_FILE_REGULAR)
            sendFile(first);
        else if(first.type == QOM_FILE_DIR)
            sendDir(first);
        else if(first.type == QOM_FILE_RETPARENT)
        {
            //qDebug() << fileUtils()->formatFileHeader(first).toAscii();
            writeBlock(fileUtils()->formatFileHeader(first).toAscii());
        }
        if(m_sendBuffer.size()>0)
            writeBlock("");
        nextFileRequested();
    }
    else
    {
        qDebug() << "We're done sending";
        accept();
    }
}

void FileSendProgressDialog::sendFile(const FileInfo& info)
{
    //qDebug() << "sendFile : entered" << m_writingData;
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
    {
        copy.fileName = info.fileName.split(QDir::separator()).last();
        writeBlock(fileUtils()->formatFileHeader(copy).toAscii());
    }
    //qDebug() << fileUtils()->formatFileHeader(copy).toAscii();
    //if(copy.fileName.contains("elf.h"))
    //    qDebug() << "Elf " << fileUtils()->formatFileHeader(copy).toAscii();
    m_writingData = true;
    writeFile(m_socket, &f);
    //nextFileRequested();
}

void FileSendProgressDialog::sendDir(const FileInfo& info)
{
    QList<FileInfo> headers = fileUtils()->directoryInfoList(
                            info.fileID == -1 ? info.fileName : fileUtils()->resolveFilePath(info.fileID));
    
    FileInfo copy = headers.takeFirst();
    //qDebug() << "sendDir: " << fileUtils()->formatFileHeader(copy).toAscii() << m_writingData;
    writeBlock(fileUtils()->formatFileHeader(copy).toAscii());
    
    // this swap is to prepend the new sub directory entries
    // so that they follow the directory name
    headers += m_fileList;
    m_fileList = headers;
    //nextFileRequested();
    //qDebug() << "sendDir : done";
}

void FileSendProgressDialog::writeFile(QTcpSocket* sock, QFile* f)
{
    //qDebug() << "writeFile : entered " <<f->fileName() <<m_writingData;
    while(!f->atEnd())
    {
        QByteArray b = f->read(QOM_FILE_WRITE_SIZE);
        writeBlock(b);
    }
    f->close();
    m_writingData = false;
}

bool FileSendProgressDialog::writeBlock(QByteArray b)
{
    //qDebug() << "writeBlock : entered"<<m_writingData;
    if(b.contains("elf.h"))
        qDebug() << "%% Have to write " << m_sendBuffer;
    m_sendBuffer += b;
    int bytesToWrite = m_sendBuffer.size();
    //qDebug() << bytesToWrite << m_sendBuffer;
    int bytesWritten = 0;
    do
    {
        bytesWritten = m_socket->write(m_sendBuffer);
        //qDebug() << "writeBlock: wrote = " << m_sendBuffer.left(bytesWritten);
        if(bytesWritten == -1)
            return false;
        m_sendBuffer.remove(0, bytesWritten);
        bytesToWrite -= bytesWritten;
    } while(bytesToWrite > 0);
    m_socket->flush();
    return true;
}