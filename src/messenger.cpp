#include "messenger.h"

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
    lst << "1" ; // TODO: use version
    lst << QString(packetNo());
    lst << sender()->name();
    lst << sender()->host();
    lst << QString(command());
    lst << payload();
    
    return lst.join(":");
}