#include "messenger.h"

Member member_me("", QHostInfo::localHostName(), QHostAddress(QHostAddress::LocalHost).toString(), "Available");

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
    lst << QString(INT_VERSION) ; // TODO: use version
    lst << QString(packetNo());
    lst << sender()->name();
    lst << sender()->host();
    lst << QString(command());
    lst << payload();
    
    return lst.join(":");
}

Message Message::fromString(QString s)
{
    QStringList tokens = s.split(":");
    quint32 pNo = tokens[1].toInt();
    
    Member* m = new Member;
    m->setName(tokens[2]);
    m->setHost(tokens[3]);
    
    quint32 cmd = tokens[4].toInt();
    QString py = tokens[5];
    
    return Message(pNo, m, cmd, py);
}


Messenger::Messenger() : QObject()
{
    reset();
}

void Messenger::reset()
{
    m_packetNo = 0;
    
    delete socket;
    socket = NULL;
    
    socket = new QUdpSocket;
    socket->bind(QHostAddress::LocalHost, UDP_PORT);
    
    connect(socket, SIGNAL(readyRead()), this, SLOT(receiveData()));
}

bool Messenger::sendMessage(quint32 command, QString payload, Member* to)
{
    Message msg(packetNo(), &member_me, command, payload);
    return sendMessage( msg, to );
}

bool Messenger::sendMessage(Message msg, Member* to)
{
    // NOTE: This is an IMPORTANT step. that's why sniffing the data will give you big command numbers
    msg.setCommand(msg.command() | QOM_ENCRYPTOPT | QOM_FILEATTACHOPT);
    
    const QByteArray data = msg.toString().toAscii();
    
    return socket->writeDatagram(data, *(to->address()), UDP_PORT) != -1;
}

bool Messenger::broadcast(quint32 command, QString payload)
{
    // TODO: how to broadcast and then send individually without duplication
}

bool Messenger::broadcast(Message msg)
{
}

void Messenger::receiveData()
{
    while(socket->hasPendingDatagrams())
    {
        QByteArray data;
        data.resize(socket->pendingDatagramSize());
        QHostAddress sender;
        
        socket->readDatagram(data.data(), data.size(), &sender);
        
        Message msg = Message::fromString(data.data());
        
        //NOTE: this is important too
        msg.setCommand(msg.command() & 0xff);
        
        //TODO:emit signals
    }
}