#include "filethreads.h"

#include <QMessageBox>
#include <QProgressDialog>
#include <QDir>
#include <QFileInfo>
#include <QTimer>

#include "constants.h"

FileSendThread::FileSendThread(int socketDescriptor, QObject* parent=0) : QThread(parent)
{        
    m_descriptor = socketDescriptor;
    start();
    
    //setMember(MemberUtils::get("members_list", m_socket->peerAddress().toString()));
    //emit requestMemberName(m_socket->peerAddress().toString());
}

void FileSendThread::socketError(QTcpSocket::SocketError error)
{
    qWarning() << "Socket Error:" << m_socket->errorString();
    m_socket->disconnectFromHost();
    emit connectionError(m_socket->errorString());
    exit(1);
}
