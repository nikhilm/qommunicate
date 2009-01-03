#include <QSettings>
#include <QNetworkInterface>
#include <QMessageBox>

#include "messenger.h"

Member member_me("", QHostInfo::localHostName(), QHostAddress(QHostAddress::LocalHost).toString(), "Available");
Group group_me("");

Messenger* m = NULL;

Messenger* messenger()
{
    if(!m)
        m = new Messenger;
    return m;
}

Member me()
{
    return member_me;
}

Group myGroup()
{
    return group_me;
}

Message::Message(quint32 pNo, Member* sender, quint32 cmd, QByteArray payload)
{
    setPacketNo(pNo);
    setSender(sender);
    setCommand(cmd);
    setPayload(payload);
}

QByteArray Message::toAscii()
{
    QByteArray arr;
    
    arr += QByteArray::number(INT_VERSION);
    arr += ':';
    arr += QByteArray::number(packetNo());
    arr += ':';
    arr += sender()->name().toAscii();
    arr += ':';
    arr += sender()->host().toAscii();
    arr += ':';
    arr += QByteArray::number(command());
    arr += ':';
    arr += payload();
    
    return arr;
}

QString Message::toString()
{
    return QString(toAscii());
}

Message Message::fromAscii(QByteArray arr)
{
    QList<QByteArray> tokens = arr.split(':');
    
    // reconstitute payload, if payload itself contains ':'
    for(int i = 6; i < tokens.size(); i++)
        tokens[5] += ":"+tokens[i];
    
    quint32 pNo = tokens[1].toInt();
    
    Member* m = new Member;
    m->setName(QString(tokens[2])); // default name, this will be replaced when BR_ENTRY is received
    m->setHost(QString(tokens[3]));
    
    quint32 cmd = tokens[4].toInt();
    QByteArray py = tokens[5];
    
    return Message(pNo, m, cmd, py);
}

Message Message::fromString(QString s)
{
    return Message::fromAscii(s.toAscii());
}

Messenger::Messenger() : QObject()
{
    socket = NULL;
    reset();
}

void Messenger::reset()
{
    m_packetNo = 1;
    
    if(socket) {
        socket->close();
        delete socket;
        socket = NULL;
    }
    
    socket = new QUdpSocket(this);
    
    if(!socket->bind(UDP_PORT))
    {
        QMessageBox::critical(0, tr("Connection Error"), tr("Could not bind, perhaps port %1 is being used by another application.").arg(UDP_PORT));
        exit(1);
    }
    
    QSettings s;
    member_me.setName(s.value(tr("nick")).toString());
    group_me.setName(s.value(tr("group")).toString());
    
    connect(socket, SIGNAL(readyRead()), this, SLOT(receiveData()));
}

Message Messenger::makeMessage(quint32 command, QByteArray payload)
{
    Message msg(packetNo(), &member_me, command, payload);
    // NOTE: This is an IMPORTANT step. that's why sniffing the data will give you big command numbers
    msg.setCommand(msg.command() /*| QOM_ENCRYPTOPT*/ | QOM_FILEATTACHOPT | QOM_ABSENCEOPT );
    
    return msg;
}

bool Messenger::sendMessage(quint32 command, QByteArray payload, Member* to)
{
    return sendMessage( makeMessage(command, payload), to );
}

bool Messenger::sendMessage(Message msg, Member* to)
{    
    const QByteArray data = msg.toAscii();
    return socket->writeDatagram(data, to->address(), UDP_PORT) != -1;
}

/*
 * Sends a broadcast
 * and then also multicasts to all ips stored in the Settings
 */
bool Messenger::multicast(quint32 command, QByteArray payload)
{
    QByteArray data = makeMessage(command, payload).toAscii();
    
    socket->writeDatagram(data, QHostAddress::Broadcast, UDP_PORT);
    
    QString ip;
    foreach(ip, ips())
    {
        socket->writeDatagram(data, QHostAddress(ip), UDP_PORT);
    }
}

void Messenger::receiveData()
{
    while(socket->hasPendingDatagrams())
    {
        QByteArray data;
        data.resize(socket->pendingDatagramSize());
        QHostAddress from;
        
        socket->readDatagram(data.data(), data.size(), &from);
        
        if(QNetworkInterface::allAddresses().contains(from))
        {
            continue;
        }
        
        data.replace('\0', QOM_HOSTLIST_SEPARATOR);
        
        Message msg = Message::fromAscii(data.data());

        msg.sender()->setAddress(from.toString());
        
        switch(msg.command() & 255)
        {
            case QOM_ANSENTRY:
                emit msg_ansEntry(msg);
                break;
            
            case QOM_BR_ENTRY:
                emit msg_entry(msg);
                break;
                
            case QOM_BR_EXIT:
                emit msg_exit(msg);
                break;
                
            case QOM_SENDMSG:
                emit msg_recvMsg(msg);
                break;
                
            case QOM_RECVMSG:
                emit msg_recvConfirmMsg(msg);
                break;
                
            case QOM_GETABSENCEINFO:
                emit msg_getAbsenceInfo(msg);
        }
    }
}

QStringList Messenger::ips() const
{
    QSettings s;
    s.beginGroup("ips");
    
    QStringList ret;
    
    QString ipKey;
    foreach(ipKey, s.childKeys())
    {
        ret << s.value(ipKey).toString();
    }
    
    return ret;
}

bool Messenger::login()
{
    multicast(QOM_BR_ENTRY, member_me.name().toAscii()+'\0'+group_me.name().toAscii());
}

bool Messenger::logout()
{
    multicast(QOM_BR_EXIT, member_me.name().toAscii()+'\0'+group_me.name().toAscii());
}
