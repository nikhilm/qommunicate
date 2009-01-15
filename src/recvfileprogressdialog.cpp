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
        
        QString path = m_saveDir + QDir::separator() + info.fileName;
        
        //TODO: fix offset call
        if(m_requestType == QOM_FILE_REGULAR)
        {
            m_currentSize = info.size;
            m_waitingForData = info.size;
            if(!openFile(path))
                continue;
            setLabelText(path);
            payload += ":0";
            writeBlock(messenger()->makeMessage(QOM_GETFILEDATA, payload).toAscii());
        }
        else if(m_requestType == QOM_FILE_DIR)
        {
            writeBlock(messenger()->makeMessage(QOM_GETDIRFILES, payload).toAscii());
        }
    }
}

void RecvFileProgressDialog::informUser()
{
    QString message(m_msg.sender()->name());
    message.append(tr(" has sent the following files\n\n"));
    
    qint64 totalSize = 0;
    
    foreach(RecvFileInfo info, m_fileHeaders)
    {
        message.append(info.fileName);
        message.append(" (");
        if(info.type == QOM_FILE_REGULAR)
        {
            message.append(QString::number(info.size));
            totalSize += info.size;
            message.append(" bytes)\n");
        }
        else if(info.type == QOM_FILE_DIR)
        {
            message.append(" (Directory, unknown size)");
        }
    }
    
    message.append(tr("\nTotal Size: "));
    if(totalSize > 1024*1024)
    {
        message.append(QString::number(totalSize / (1024*1024) ));
        message.append("Mb");
    }
    else if( totalSize > 1024 )
    {
        message.append(QString::number(totalSize / 1024));
        message.append("Kb");
    }
    else
    {
        message.append(QString::number(totalSize));
        message.append("bytes");
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
    else if(m_requestType == QOM_FILE_DIR)
        writeToDirectory(data);
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
    setValue((float)(m_currentSize-m_waitingForData)/m_currentSize * 100.0);
    if(m_waitingForData <= 0)
    {
        qDebug() << "Finished writing" << m_currentFile->fileName();
        m_currentFile->close();
        m_requestType = 0;
    }
    return true;
}

bool RecvFileProgressDialog::makeDirectory(const QString& path)
{
    qDebug() << "makeDirectory" << path;
    if(m_dir != NULL)
    {
        delete m_dir;
        m_dir = NULL;
    }
    m_dir = new QDir(path);
    if(!m_dir->exists() && !m_dir->mkdir(path))
    {
        // TODO: convert to QMessageBox
        qWarning() << "Could not create directory" << m_dir->absolutePath() ;
        return false;
    }
    return true;
}

bool RecvFileProgressDialog::writeToDirectory(QByteArray& b)
{
    //qDebug() << "Write to directory recvd" << b;
    if(m_inHeader)
    {
        m_header += b;
        qDebug() << "m_header"<<m_header;
        QByteArray leftover;
        RecvFileInfo info = parseDirectoryHeader(m_header, &leftover);
        if(info.fileID < 0)
        {
            qDebug() << "Could not parse headers";
            m_inHeader = false;
            return false;
        }
        if(info.type == QOM_FILE_REGULAR)
        {
            qDebug() << "Creating file" << info.fileName;
            if(!openFile(m_dir->absoluteFilePath(info.fileName)))
                return false;
            if(!writeToFile(leftover))
                return false;
            m_inHeader = false;
        }
        else if(info.type == QOM_FILE_DIR)
        {
            qDebug() << "Writing dir" << info.fileName;
            if(!makeDirectory(m_dir->absoluteFilePath(info.fileName)))
                return false;
            m_header.clear();
        }
        else if(info.type == QOM_FILE_RETPARENT)
        {
            qDebug() << "Going up";
            m_dir->cdUp();
        }
        m_header = leftover;
    }
    
    if(!m_inHeader)
    {
        qDebug() << "Writing to file";
        writeToFile(b);
        if(m_waitingForData == 0)
        {
            m_inHeader = true;
        }
    }
    return true;
}

/**
 * Tries to parse a header if it can
 * if it can, fills in RecvFileInfo correctly
 * Otherwise sets RecvFileInfo.fileID to -1, other fields are kept default
 * If any data is left over after parsing header, it is stored in remainder.
 */
RecvFileInfo RecvFileProgressDialog::parseDirectoryHeader(const QByteArray& a, QByteArray* remainder)
{
    bool bad = false;
    RecvFileInfo ret;
    
    if(a.contains(":"))
    {
        QList<QByteArray> tokens = a.split(':');
        int headerSize = tokens[0].toInt(0, 16);
        qDebug() << "Header size:"<<headerSize;
        if(a.size() < (headerSize-tokens[0].size()))
        {
            qDebug() << "not enough data";
            bad = true;
        }
        else
        {
            ret.fileName = tokens[1];
            ret.size = tokens[2].toInt(0, 16);
            ret.type = tokens[3].toInt(0, 16);
            
            qDebug() << "Parsed" << ret.fileName << ret.size << ret.type;
            
            *remainder = a.right(a.size() - headerSize);
            qDebug() << "leftovers" << *remainder;
            qDebug() << "\n";
        }
    }
    else
        bad = true;
    
    if(bad)
    {
        ret.fileID = -1;
    }
    
    return ret;
}