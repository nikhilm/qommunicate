#include "recvfileprogressdialog.h"

#include <QStringList>
#include <QMessageBox>
#include <QFileDialog>

#include "constants.h"

/**
 * Returns a list of requested files
 */
QList<RecvFileInfo> RecvFileProgressDialog::parsePayloadFileList(QByteArray payload)
{
    // discard the message part, it's not for us to handle.
    if(payload.contains('\0'))
        payload = payload.split('\0')[1];
    
    QList<RecvFileInfo> recvFiles;
    QList<QByteArray> headers = payload.split(QOM_FILELIST_SEPARATOR);
    foreach(QByteArray header, headers)
    {
        RecvFileInfo info;
        QList<QByteArray> tokens = header.split(':');
        if(tokens.size() < 5)
            continue;
        info.fileID = tokens[0].toInt();
        info.fileName = tokens[1];
        info.size = tokens[2].toInt();
        info.mtime = tokens[3].toInt();
        info.type = tokens[4].toInt();
        
        //NOTE: additional tokens can be parsed here if required
        
        recvFiles << info ;
    }
    
    return recvFiles;
}

void RecvFileProgressDialog::error(QAbstractSocket::SocketState e)
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
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketState)), this, SLOT(error(QAbstractSocket::SocketState)));
}

void RecvFileProgressDialog::requestFiles()
{
    foreach(RecvFileInfo info, parsePayloadFileList(m_msg.payload()))
    {
        m_requestType = info.type;
        //TODO: fix offset call
        if(m_requestType == QOM_FILE_REGULAR)
            writeBlock(QByteArray::number(m_msg.packetNo()).append(':').append(info.fileName).append(":0"));
    }
}

void RecvFileProgressDialog::informUser()
{
    QString message(m_msg.sender()->name());
    message.append(tr(" has sent the following files\n"));
    
    foreach(RecvFileInfo info, parsePayloadFileList(m_msg.payload()))
    {
        message.append(info.fileName);
        message.append("\n");
    }
    if(m_msg.payload().contains('\0'))
    {
        QString msg = m_msg.payload().split('\0')[0];
        if(!msg.isEmpty())
            message.append("Message: " + msg);
    }
    message.append(tr("\nDo you want to accept the files?"));
    if(QMessageBox::No == QMessageBox::question(NULL, 
                           tr("Receiving files"), 
                           message, 
                           QMessageBox::Yes | QMessageBox::No,
                           QMessageBox::Yes)) // last is default button
        reject();
    else
        m_saveDir = QFileDialog::getExistingDirectory(this, tr("Save To"));
    qDebug() << "Base save dir" << m_saveDir;
}

// TODO: share with sending dialog?
bool RecvFileProgressDialog::writeBlock(QByteArray b)
{
    int bytesToWrite = b.size();
    int bytesWritten = 0;
    do
    {
        bytesWritten = m_socket->write(b);
        if(bytesWritten == -1)
            return false;
        b.remove(0, bytesWritten);
        bytesToWrite -= bytesWritten;
    } while(bytesToWrite > 0);
    return true;
}