#include "sendfileprogressdialog.h"

#include <QFileInfo>
#include <QDir>

void FileSendProgressDialog::initialise()
{
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
    connect(m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(updateStatus(qint64)));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(nextFileRequested()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(accept()));
    connect(m_socket, SIGNAL(disconnected()), m_socket, SLOT(deleteLater()));
    connect(this, SIGNAL(requestFilePath(int)), fileUtils(), SLOT(resolveFilePath(int)));
    connect(fileUtils(), SIGNAL(filePath(QString)), this, SLOT(acceptFilePath(QString)));
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
    //qDebug() << "\n@@ Wrote to socket file?"<<(m_currentFile == NULL ? "NULL":m_currentFile->fileName());
    qDebug() << "bytes written";
    m_totalSent += bytes;
    
    setValue((float)m_totalSent/m_fileSize * 100.0);
    
    if(m_totalSent == m_fileSize)
    {
        qDebug() << "File sent";
        return;
    }
    
}

void FileSendProgressDialog::nextFileRequested()
{
    qDebug() << "next file requested from" << receiver()->name();
    m_offset = 0;
    m_totalSent = 0;
    
    qint64 offset;
    QList<QByteArray> metadata = m_socket->readAll().split(':');
    qDebug() << "Meta data recd" << metadata;
    // this 8 thing is because the last token in metadata is blank
    if(metadata.size() < 8)
    {
        qWarning() << "Error: Bad request from receiver" ;
        return;
    }
    
    int command = metadata[4].toInt();
    switch(command)
    {
        case QOM_GETFILEDATA:
            m_offset = ( metadata[7].isEmpty() ? 0 : metadata[7].toInt() );
            // NOTE: The fallthrough is important here
            
        case QOM_GETDIRFILES:
            emit requestFilePath(metadata[6].toInt());            
            break;
    }
}

bool FileSendProgressDialog::writeBlock(QByteArray b)
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

void FileSendProgressDialog::acceptFilePath(QString fileName)
{
    //qDebug() << " acceptFilePath: preparing to send" << fileName;    
    
    if(QFileInfo(fileName).isDir())
    {
        sendDir(fileName);
        return;
    }
    
    sendFile(fileName, m_offset);
}

void FileSendProgressDialog::sendFile(const QString& fileName, int offset)
{
    QFile f(fileName);
    if(!f.open(QIODevice::ReadOnly))
    {
        qWarning() << "Could not open file for reading" << f.fileName()<<f.errorString();
        return;
    }
    m_fileSize = f.size();
    f.seek(offset);
    m_totalSent = offset;
    setRange(0, 100);
    setLabelText(tr("Sending %1").arg(f.fileName()));
    qDebug() << "Now sending " << f.fileName();
    writeFile(m_socket, &f);
}

void FileSendProgressDialog::sendDir(const QString& fileName)
{
    qDebug() << "Writing dir data";
    QDir dir(fileName);
    QStringList headers = fileUtils()->formatHeirarchialTcpRequest(fileName);
    
    // the directory requires special handling, otherwise QFile tries to open
    // <dirname>/<dirname> which does not exist
    QString first = headers.takeFirst();
    qDebug() << first.toAscii();
    m_socket->write(first.toAscii());
    foreach(QString header, headers)
    {
        QString path = dir.absoluteFilePath(header.section(':', 1, 1));
        
        if(header.section(':', 1, 1) == ".")
        {
            //qDebug() << header.toAscii() << header.size() << (int)header.toAscii()[8];
            m_socket->write(header.toAscii());
            qDebug() << "\n\n<< going back up" ;
        }
        else if(QFileInfo(path).isDir())
        {
            qDebug() << "\n\n>> Going down one level\n\n";
            sendDir(path);
            //qDebug() << "\n\n<< Back up\n\n";
        }
        else
        {
            m_offset = 0;
            m_socket->write(header.toAscii());
            sendFile(path, 0);
        }
    }
}

void FileSendProgressDialog::writeFile(QTcpSocket* sock, QFile* f)
{
    while(!f->atEnd())
    {
        writeBlock(f->read(QOM_FILE_WRITE_SIZE));
    }
    f->close();
}