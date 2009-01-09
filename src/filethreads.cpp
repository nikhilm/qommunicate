#include "filethreads.h"

#include <QMessageBox>
#include <QProgressDialog>
#include <QDir>
#include <QFileInfo>

#include "constants.h"

void FileSendThread::socketError(QTcpSocket::SocketError error)
{
    qWarning() << "Socket Error:" << m_socket->errorString();
    m_socket->disconnectFromHost();
    emit connectionError(m_socket->errorString());
    exit(1);
}

void FileSendThread::run()
{
}

void FileSendThread::nextFileRequested()
{
    qDebug() << "nextFileRequested";
    m_offset = 0;
    m_totalSent = 0;
    
    if(m_file != NULL)
    {
        m_file->close();
        delete m_file;
        m_file = NULL;
    }
    
    qint64 offset;
    QList<QByteArray> metadata = m_socket->readAll().split(':');
    qDebug() << metadata;
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

void FileSendThread::writeOut(QTcpSocket* sock, QFile* f)
{
    while(!f->atEnd())
    {
        sock->write(f->read(1024));
    }
}

void FileSendThread::acceptFilePath(QString fileName)
{
    qDebug() << " acceptFilePath: preparing to send" << fileName;    
    
    if(QFileInfo(fileName).isDir())
    {
        qDebug() << "Writing dir data";
        QDir dir(fileName);
        //qDebug() << fileUtils()->formatHeirarchialTcpRequest(fileName) ;
        foreach(QString header, fileUtils()->formatHeirarchialTcpRequest(fileName))
        {
            qDebug() << header;
            QFile f(dir.absoluteFilePath(header.section(':', 1, 1)));
            if(!f.open(QIODevice::ReadOnly))
                qDebug() << "Error opening file" << f.error();
            m_socket->write(header.toAscii());
            writeOut(m_socket, &f);
        }
        emit sendingNextFile(fileName);
        return;
    }
    
    emit sendingNextFile(fileName);
    m_file = new QFile(fileName);
    if(!m_file->open(QIODevice::ReadOnly))
    {
        qWarning() << "Could not open file for reading" ;
        exit(1);
    }
    m_file->seek(m_offset);
    m_totalSent = m_offset;
    qDebug() << "Preparing to write";
    writeOut(m_socket, m_file);
}

void FileSendThread::updateProgress(qint64 bytes)
{
    if( m_file == NULL || m_file->size() == 0 )
        return;
    m_totalSent += bytes;
    //qDebug() <<"Progress:"<<(float)m_totalSent/m_file->size()*100.0<<"%";
    emit notifyProgress((float)m_totalSent/m_file->size() * 100.0);
    
}

void FileSendThread::sendFiles()
{
    
}