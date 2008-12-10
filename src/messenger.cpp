#include <QSettings>

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

Message::Message(quint32 pNo, Member* sender, quint32 cmd, QString payload)
{
    setPacketNo(pNo);
    setSender(sender);
    setCommand(cmd);
    setPayload(payload);
}

QString Message::toString()
{
    QStringList lst;
    lst << QString("%1").arg(INT_VERSION) ; // TODO: use version
    lst << QString("%1").arg(packetNo());
    lst << sender()->name();
    lst << sender()->host();
    lst << QString("%1").arg(command());
    lst << payload();
    return lst.join(":");
}

Message Message::fromString(QString s)
{
    QStringList tokens = s.split(":");
    quint32 pNo = tokens[1].toInt();
    
    Member* m = new Member;
    m->setName(tokens[2]); // default name, this will be replaced when BR_ENTRY is received
    m->setHost(tokens[3]);
    
    quint32 cmd = tokens[4].toInt();
    QString py = tokens[5];
    
    return Message(pNo, m, cmd, py);
}


Messenger::Messenger() : QObject()
{
    socket = NULL;
    fileSocket = NULL;
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
    socket->bind(UDP_PORT);
    
    QSettings s;
    member_me.setName(s.value(tr("nick")).toString());
    group_me.setName(s.value(tr("group")).toString());
    
    connect(socket, SIGNAL(readyRead()), this, SLOT(receiveData()));
}

Message Messenger::makeMessage(quint32 command, QString payload)
{
    Message msg(packetNo(), &member_me, command, payload);
    // NOTE: This is an IMPORTANT step. that's why sniffing the data will give you big command numbers
    msg.setCommand(msg.command() /*| QOM_ENCRYPTOPT*/ | QOM_FILEATTACHOPT);
    
    return msg;
}

bool Messenger::sendMessage(quint32 command, QString payload, Member* to)
{
    return sendMessage( makeMessage(command, payload), to );
}

bool Messenger::sendMessage(Message msg, Member* to)
{    
    const QByteArray data = msg.toString().toAscii();
    
    return socket->writeDatagram(data, to->address(), UDP_PORT) != -1;
}

bool Messenger::multicast(quint32 command, QString payload, QList<Member*> members)
{
    // TODO: 
}

bool Messenger::multicast(Message msg, QList<Member*> members)
{
}

void Messenger::receiveData()
{
    while(socket->hasPendingDatagrams())
    {
        QByteArray data;
        data.resize(socket->pendingDatagramSize());
        QHostAddress from;
        
        socket->readDatagram(data.data(), data.size(), &from);
        data.replace('\0', QOM_HOSTLIST_SEPARATOR);
        
        Message msg = Message::fromString(QString(data.data()));
                
        //NOTE: this is important too
        msg.setCommand(msg.command() & 0xff);
        
        msg.sender()->setAddress(from.toString());
        
        qDebug() << msg.toString() ;
        switch(msg.command())
        {
            case QOM_ANSENTRY:
                emit msg_ansEntry(msg);
                break;
            
            case QOM_BR_ENTRY:
                emit msg_entry(msg);
                break;
                
            case QOM_SENDMSG:
                emit msg_recvMsg(msg);
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
    QByteArray data(makeMessage(QOM_BR_ENTRY, member_me.name()+'\0'+group_me.name()).toString().toAscii());
    
    QString ip;
    foreach(ip, ips())
    {
        socket->writeDatagram(data.data(), data.size(), QHostAddress(ip), UDP_PORT);
    }
}

bool Messenger::logout()
{
    QByteArray data(makeMessage(QOM_BR_EXIT, member_me.name()+"\0"+group_me.name()).toString().toAscii());
    
    QString ip;
    foreach(ip, ips())
    {
        socket->writeDatagram(data.data(), data.size(), QHostAddress(ip), UDP_PORT);
    }
}