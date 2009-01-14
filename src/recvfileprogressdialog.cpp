#include "recvfileprogressdialog.h"

#include <QStringList>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>

#include "constants.h"

/**
 * Returns a list of requested files
 */
QList<RecvFileInfo> RecvFileProgressDialog::parsePayloadFileList(QByteArray payload)
{
    QList<RecvFileInfo> recvFiles;
    QList<QByteArray> headers = payload.split(QOM_FILELIST_SEPARATOR);
    foreach(QByteArray header, headers)
    {
        RecvFileInfo info;
        QList<QByteArray> tokens = header.split(':');
        qDebug() << "Tokens:" << tokens;
        if(tokens.size() < 5)
            continue;
        info.fileID = tokens[0].toInt();
        info.fileName = tokens[1];
        info.size = tokens[2].toInt(0, 16);
        info.mtime = tokens[3].toInt(0, 16);
        info.type = tokens[4].toInt(0, 16);
        qDebug() << "Parsed ID:"<<info.fileID<<"name:"<<info.fileName<<"size:"<<info.size<<"mtime"<<info.mtime<<"type:"<<info.type;
        
        //NOTE: additional tokens can be parsed here if required
        
        recvFiles << info ;
    }
    return recvFiles;
}

void RecvFileProgressDialog::error(QAbstractSocket::SocketError e)
{
    qDebug() << "Socket error:" << e << m_socket->errorString();
    reject();
}

void RecvFileProgressDialog::startReceiving()
{
    m_socket = new QTcpSocket(this);
    m_socket->abort();
    m_socket->connectToHost(m_msg.sender()->addressString(), UDP_PORT);
    
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readRequest()));
    connect(m_socket, SIGNAL(connected()), this, SLOT(requestFiles()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(accept()));
    connect(m_socket, SIGNAL(disconnected()), m_socket, SLOT(deleteLater()));
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
}

void RecvFileProgressDialog::requestFiles()
{
    while(!m_fileHeaders.isEmpty())
    {
        if(m_waitingForData > 0)
        {
            qDebug() << "Waiting for"<< m_waitingForData;
            QTimer::singleShot(1000, this, SLOT(requestFiles()));
            break;
        }
        startReceiving();
        RecvFileInfo info = m_fileHeaders.takeFirst();
        m_requestType = info.type;
        
        QByteArray payload = QByteArray::number(m_msg.packetNo(), 16);
        payload += ":";
        payload += QByteArray::number(info.fileID, 16);
        qDebug() << "Writing request for file data"<<info.fileName << "id" << info.fileID;
        
        //TODO: fix offset call
        if(m_requestType == QOM_FILE_REGULAR)
        {
            m_waitingForData = info.size;
            if(!openFile(m_saveDir + QDir::separator() + info.fileName))
                continue;
            payload += ":0";
            writeBlock(messenger()->makeMessage(QOM_GETFILEDATA, payload).toAscii());
        }
    }
}

void RecvFileProgressDialog::informUser()
{
    QString message(m_msg.sender()->name());
    message.append(tr(" has sent the following files\n"));
    
    foreach(RecvFileInfo info, m_fileHeaders)
    {
        message.append(info.fileName);
        message.append("\n");
    }
    
    QString msg = m_msg.payload().left(m_msg.payload().indexOf("\a"));
    if(!msg.isEmpty())
        message.append("\n\nMessage: " + msg);
    
    message.append(tr("\n\nDo you want to accept the files?"));
    if(QMessageBox::No == QMessageBox::question(NULL, 
                           tr("Receiving files"), 
                           message, 
                           QMessageBox::Yes | QMessageBox::No,
                           QMessageBox::Yes)) // last is default button
        reject();
    else
        m_saveDir = QFileDialog::getExistingDirectory(this, tr("Save To"));
    
    if(!m_saveDir.isEmpty())
    {
        qDebug() << m_saveDir;
    }
}

// TODO: share with sending dialog?
bool RecvFileProgressDialog::writeBlock(QByteArray b)
{
    int bytesToWrite = b.size();
    int bytesWritten = 0;
    do
    {
        qDebug() << "Writing" << b << "to socket";
        bytesWritten = m_socket->write(b);
        if(bytesWritten == -1)
            return false;
        b.remove(0, bytesWritten);
        bytesToWrite -= bytesWritten;
    } while(bytesToWrite > 0);
    return true;
}

void RecvFileProgressDialog::readRequest()
{
    QByteArray data;
    while(m_socket->bytesAvailable())
    {
        data += m_socket->read(1024);
        //qDebug() << "Reading";
    }
    //qDebug() << "In read request: " << m_requestType;
    if(m_requestType == QOM_FILE_REGULAR)
        writeToFile(data);
}

bool RecvFileProgressDialog::openFile(const QString& fileName)
{    
    m_currentFile = new QFile(fileName);
    if(m_currentFile->exists())
    {
        if(QMessageBox::No == QMessageBox::question(this, tr("Replace file"), tr("File %1 exists, overwrite?").arg(fileName),
                               QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
        {
            m_waitingForData = 0;
            return false;
        }
    }
    if(!m_currentFile->open(QIODevice::Truncate | QIODevice::WriteOnly))
    {
        qWarning() << "Cannot open" << fileName << "for writing";
        m_currentFile->close();
        m_waitingForData = 0;
        return false;
    }
    qDebug() << fileName << "Opened with truncate";
    return true;
}

bool RecvFileProgressDialog::writeToFile(QByteArray& b)
{
    if(m_currentFile == NULL)
        return false;
    while(b.size() > 0)
    {
        int written = m_currentFile->write(b);
        //qDebug() << "Wrote" << written << "bytes to" << m_currentFile->fileName();
        if(written == -1)
        {
            qWarning() << "Write error" << m_currentFile->errorString();
            m_currentFile->close();
            return false;
        }
        m_waitingForData -= written;
        b.remove(0, written);
    }
    if(m_waitingForData <= 0)
    {
        qDebug() << "Finished writing" << m_currentFile->fileName();
        qDebug() << "Files left" << m_fileHeaders.size();
        m_currentFile->close();
        m_requestType = 0;
    }
    return true;
}