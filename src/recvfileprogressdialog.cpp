/*
 * (C) 2009 Nikhil Marathe <nsm.nikhil@gmail.com>
 * Licensed under the GNU General Public License.
 * See LICENSE for details
 */
#include "recvfileprogressdialog.h"

#include <QStringList>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>
#include <QFile>

#include "constants.h"
#include "memberutils.h"

/**
 * Returns a list of requested files
 */
QList<FileInfo> RecvFileProgressDialog::parsePayloadFileList(QByteArray payload)
{
    QList<FileInfo> recvFiles;
    QList<QByteArray> headers = payload.split(QOM_FILELIST_SEPARATOR);
    foreach(QByteArray header, headers)
    {
        FileInfo info;
        QList<QByteArray> tokens = header.split(':');
        qDebug() << "Tokens:" << tokens;
        if (tokens.size() < 5)
            continue;
        info.fileID = tokens[0].toInt();
        info.fileName = tokens[1];
        info.size = tokens[2].toLongLong(0, 16);
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
    if (e == QAbstractSocket::RemoteHostClosedError)
    {
        
    }
    else
        reject();
}

void RecvFileProgressDialog::startReceiving()
{    
    if (m_socket != NULL)
    {
        m_socket->close();
        delete m_socket;
        m_socket = NULL;
    }
    m_socket = new QTcpSocket(this);
    m_socket->abort();
    m_socket->connectToHost(m_msg.sender()->addressString(), UDP_PORT);
    qDebug() << "Connected to host";
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readRequest()));
    connect(m_socket, SIGNAL(connected()), this, SLOT(requestFiles()));
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
}

void RecvFileProgressDialog::requestFiles()
{
    if (!m_fileHeaders.isEmpty())
    {
        FileInfo info = m_fileHeaders.takeFirst();
        m_requestType = info.type;
        
        QByteArray payload = QByteArray::number(m_msg.packetNo(), 16);
        payload += ":";
        payload += QByteArray::number(info.fileID, 16);
        qDebug() << "Writing request for file data"<<info.fileName << "id" << info.fileID;
        
        QString path = m_saveDir + QDir::separator() + info.fileName;
        
        //TODO: fix offset call
        if (m_requestType == QOM_FILE_REGULAR)
        {
            m_currentSize = info.size;
            m_waitingForData = info.size;
            if (!openFile(path))
                return;
            payload += ":0";
            writeBlock(messenger()->makeMessage(QOM_GETFILEDATA, payload).toAscii());
        }
        else if (m_requestType == QOM_FILE_DIR)
        {
            writeBlock(messenger()->makeMessage(QOM_GETDIRFILES, payload).toAscii());
        }
    }
    else
    {
        emit allDownloadsDone(tr("Successfully received files from %1").arg(MemberUtils::get("members_list", m_msg.sender()->addressString())->name()));
        accept();
    }
}

bool RecvFileProgressDialog::informUser()
{
    QString message(m_msg.sender()->name());
    message.append(tr(" has sent the following files\n\n"));
    
    qint64 totalSize = 0;
    
    foreach(FileInfo info, m_fileHeaders)
    {
        message.append(info.fileName);
        message.append(" (");
        if (info.type == QOM_FILE_REGULAR)
        {
            message.append(QString::number(info.size));
            totalSize += info.size;
            message.append(" bytes)\n");
        }
        else if (info.type == QOM_FILE_DIR)
        {
            message.append("Directory, unknown size)\n");
        }
    }
    
    message.append(tr("\nTotal Size: "));
    if (totalSize > 1024*1024)
    {
        message.append(QString::number(totalSize / (1024*1024) ));
        message.append("Mb");
    }
    else if ( totalSize > 1024 )
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
    if (!msg.isEmpty())
        message.append("\n\nMessage: " + msg);
    
    message.append(tr("\n\nDo you want to accept the files?"));

    m_notifyDialog = new QMessageBox(QMessageBox::Question,
                                tr("Receiving files"),
                                message,
                                QMessageBox::Yes | QMessageBox::No);
    m_notifyDialog->setDefaultButton(QMessageBox::Yes);
    connect(m_notifyDialog, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(userInformed(QAbstractButton*)));
    m_notifyDialog->setModal(false);
    m_notifyDialog->show();
    return true;
}

void RecvFileProgressDialog::userInformed (QAbstractButton *userResponse)
{
    disconnect(m_notifyDialog, SIGNAL(buttonClicked(QAbstractButton*)),
               this, SLOT(userInformed(QAbstractButton*)));
    QMessageBox::StandardButton userClicked = m_notifyDialog->standardButton(userResponse);
    if (userClicked == QMessageBox::Yes) {
        while (true) {
            m_saveDir = QFileDialog::getExistingDirectory(this, tr("Save To"));
            if (m_saveDir.isEmpty()) { // browse cancelled
                reject();
                return;
            }
            else if ( !(QFile::permissions(m_saveDir) & QFile::WriteUser) ) { // no write permissions in destination
                QMessageBox::StandardButton which = QMessageBox::critical(this, "Error saving files",
                                                                          "You do not have permission to save files in "+m_saveDir,
                                                                          QMessageBox::Retry | QMessageBox::Cancel,
                                                                          QMessageBox::Retry);
                if (which == QMessageBox::Cancel) { // user does not want to enter another destination
                    reject();
                    return;
                }
                else { // user wants to retry
                    continue;
                }
            }
            else {
                break;
            }
        }
        startReceiving();
    }
    else {
        reject();
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
        if (bytesWritten == -1)
            return false;
        b.remove(0, bytesWritten);
        bytesToWrite -= bytesWritten;
    } while(bytesToWrite > 0);
    return true;
}

/**
* Instead of actually reading the data
* it notifies seperate functions depending
* on what we are reading, which read as required
*/
void RecvFileProgressDialog::readRequest()
{
//     QByteArray data;
//     while(m_socket->bytesAvailable())
//     {
//         data += m_socket->read(1024);
//         //qDebug() << "Reading";
//     }
    //qDebug() << ":: readRequest";
//     qDebug() << data << "\n\n";
    if (m_requestType == QOM_FILE_REGULAR)
        requestWriteToFile();
    else if (m_requestType == QOM_FILE_DIR)
        requestWriteToDirectory();
}

void RecvFileProgressDialog::requestWriteToFile()
{
    QByteArray data = m_socket->readAll();
    writeToFile(data);
    if (m_waitingForData == 0)
    {
        emit downloadDone(tr("%1 downloaded").arg(m_currentFile->fileName()));
        startReceiving();
    }
}

/**
 * Attempts to read headers individually
 * and then read the file as required
 */
void RecvFileProgressDialog::requestWriteToDirectory()
{
    while(m_socket->bytesAvailable())
    {
        if (m_inHeader)
        {
            m_header += m_socket->read(1);
            //qDebug() << m_header;
            QByteArray drop; // TODO: ideally not required
            FileInfo info = parseDirectoryHeader(m_header, &drop);
            
            if (info.fileID >= 0)
            {
                qDebug() << "# Header parsed successfully";
                qDebug() << m_header;
                qDebug() << info.fileName << info.size << info.type;
                // we got a valid header, process it
                m_inHeader = false;
                m_header.clear();
                
                if (info.type == QOM_FILE_REGULAR)
                {
                    qDebug() << "Now writing new file" << info.fileName;
                    if (!openFile(m_dir->absoluteFilePath(info.fileName), false))
                        return ;
                    m_waitingForData = info.size;
                    m_currentSize = info.size;
                }
                else if (info.type == QOM_FILE_DIR)
                {
                    qDebug() << "Creating directory" << info.fileName;
                    bool made = false;
                    if (m_dir == NULL)
                        made = makeDirectory(m_saveDir + QDir::separator() + info.fileName);
                    else
                        made = makeDirectory(m_dir->absoluteFilePath(info.fileName));
                    if (!made)
                        return;
                    m_inHeader = true;
                    continue;
                }
                else if (info.type == QOM_FILE_RETPARENT)
                {
                    qDebug() << "Going up";
                    m_dir->cdUp();
                    if (m_dir->absolutePath() == m_saveDir)
                    {
                        startReceiving();
                        //accept();
                    }
                    m_inHeader = true;
                    continue;
                }
                else
                {
                    qDebug() << "Bad header : " << info.type;
                    QMessageBox::critical(this, tr("Receive failed"), tr("Bad header! Could not understand file type %1").arg(info.type));
                    reject();
                }
            }
        }
        // write to file whatever we get
        if (!m_inHeader)
        {            
            QByteArray fileData = m_socket->read(m_waitingForData);
            writeToFile(fileData);
            if (m_waitingForData == 0)
            {
                m_inHeader = true;
            }
        }
    }
                    
}

bool RecvFileProgressDialog::openFile(const QString& fileName, bool askOverwrite)
{
    m_currentFile = new QFile(fileName);
    if (m_currentFile->exists() && askOverwrite)
    {
        if (QMessageBox::No == QMessageBox::question(this, tr("Replace file"), tr("File %1 exists, overwrite?").arg(m_currentFile->fileName()),
                               QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
        {
            m_waitingForData = 0;
            return false;
        }
    }
    if (!m_currentFile->open(QIODevice::Truncate | QIODevice::WriteOnly))
    {
        qWarning() << "Cannot open" << fileName << "for writing";
        m_currentFile->close();
        m_waitingForData = 0;
        return false;
    }
    qDebug() << fileName << "Opened with truncate";
    return true;
}

/*
 * TODO: i know, the remainder thing is a bad hack.
 */
bool RecvFileProgressDialog::writeToFile(QByteArray& b, QByteArray* remainder)
{
    if (m_currentFile == NULL)
        return false;
    while(b.size() > 0)
    {
        if (m_waitingForData < b.size())
        {
            qDebug() << "Excess data, setting remainder";
            if (remainder != NULL)
            {
                *remainder = b.right(b.size() - m_waitingForData);
            }
            b = b.left(m_waitingForData);
        }
        int written = m_currentFile->write(b);
        //qDebug() << "Wrote" << written << "bytes to" << m_currentFile->fileName();
        if (written == -1)
        {
            qWarning() << "Write error" << m_currentFile->errorString();
            m_currentFile->close();
            return false;
        }
        m_waitingForData -= written;
        //qDebug() << "Wrote" << written << "bytes" << "Left" << m_waitingForData;
        b.remove(0, written);
    }
    
    setLabelText(tr("Receiving %1").arg(m_currentFile->fileName()));
    setValue((float)(m_currentSize-m_waitingForData)/m_currentSize * 100.0);
    if (m_waitingForData <= 0)
    {
        //qDebug() << "Finished writing" << m_currentFile->fileName();
        m_currentFile->close();
    }
    return true;
}

bool RecvFileProgressDialog::makeDirectory(const QString& path)
{
    qDebug() << "makeDirectory" << path;
    if (m_dir != NULL)
    {
        delete m_dir;
        m_dir = NULL;
    }
    m_dir = new QDir(path);
    
    if (!m_dir->exists() && !m_dir->mkdir(path))
    {
        // TODO: convert to QMessageBox
        qWarning() << "Could not create directory" << m_dir->absolutePath() ;
        return false;
    }
    return true;
}

bool RecvFileProgressDialog::writeToDirectory(QByteArray& b)
{
    m_header += b;
    //qDebug() << "At the beginning of writeToDirectory" << m_header;
    QByteArray leftover;
    FileInfo info = parseDirectoryHeader(m_header, &leftover);
    if (m_waitingForData != 0 && m_header.isEmpty())
    {
        qDebug() << "Need to flush";
        exit(5);
    }
    
    //info is filled, only leftover is valuable
    if (info.fileID >= 0)
    {
        m_inHeader = false;
        m_header = leftover;
        //qDebug() << "! Leftover" << leftover;
    
        if (info.type == QOM_FILE_REGULAR)
        {
            qDebug() << "Now writing new file" << info.fileName;
            if (!openFile(m_dir->absoluteFilePath(info.fileName)))
                return false;
            m_waitingForData = info.size;
            m_currentSize = info.size;
            
            QByteArray moreLeftover;
            
            // leftover can't immediately have another header
            // any headers it has now are in moreLeftover
            if (!writeToFile(leftover, &moreLeftover))
                return false;
            //qDebug() << "!moreLeftover" << moreLeftover;
            qDebug() << "line 306:" << m_waitingForData;
            if (m_waitingForData == 0)
            {
                m_inHeader = true;
                m_header = moreLeftover;
            }
            else
            {
                m_inHeader = false;
                m_header.clear();
            }
        }
        else if (info.type == QOM_FILE_DIR)
        {
            qDebug() << "Creating new directory" << info.fileName;
            bool made = false;
            if (m_dir == NULL)
                made = makeDirectory(m_saveDir + QDir::separator() + info.fileName);
            else
                made = makeDirectory(m_dir->absoluteFilePath(info.fileName));
            if (!made)
                return false;
            m_inHeader = true;
            //m_header.clear();
        }
        else if (info.type == QOM_FILE_RETPARENT)
        {
            qDebug() << "Going up";
            m_dir->cdUp();
            m_inHeader = true;
        }
        //qDebug() << "After all processing of header, in header?" << m_inHeader<<"\n";
    }
    
    if (!m_inHeader)
    {
        qDebug() << "> In file";
        QByteArray leftover;
        writeToFile(m_header, &leftover);
        qDebug() << "line 337:" << m_waitingForData ;
        if (m_waitingForData == 0)
        {
            m_header = leftover;
            m_inHeader = true;
        }
        else
        {
            m_header.clear();
            m_inHeader = false;
        }
    }
    qDebug() << " << Returning from writeToDirectory";
    return true;
}

/**
 * Tries to parse a header if it can
 * if it can, fills in FileInfo correctly
 * Otherwise sets FileInfo.fileID to -1, other fields are kept default
 * If any data is left over after parsing header, it is stored in remainder.
 */
FileInfo RecvFileProgressDialog::parseDirectoryHeader(const QByteArray& a, QByteArray* remainder)
{
    bool bad = false;
    FileInfo ret;
    if (a.contains(":"))
    {
        QList<QByteArray> tokens = a.split(':');
        int headerSize = tokens[0].toInt(0, 16);
        //qDebug() << "Header size:"<<headerSize;
        if ( headerSize == 0 ) //bad data, perhaps binary
            bad = true;
        else if (a.size() < headerSize || tokens.size() < 4)
        {
            //qDebug() << "not enough data" << tokens << "expected" << tokens[0].size();
            bad = true;
        }
        else
        {
            //qDebug() << "Header list" << tokens;
            ret.fileID = 0;
            ret.fileName = tokens[1];
            ret.size = tokens[2].toLongLong(0, 16);
            ret.type = tokens[3].toInt(0, 16);
            
            *remainder = a.right(a.size() - headerSize);
        }
    }
    else
        bad = true;
    
    if (bad)
    {
        ret.fileID = -1;
    }
    
    return ret;
}
