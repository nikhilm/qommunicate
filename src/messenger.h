#include <QUdpSocket>

#include "ipobjects.h"

class Message
{
public:
    Message(quint32, Member*, quint32, QString);
    
    quint32 packetNo() const { return m_packetNo; } ;
    Member* sender() const { return m_sender; } ;
    quint32 command() const { return m_command; } ;
    QString payload() const { return m_payload; } ;
    
    void setPacketNo(quint32 pNo) { m_packetNo = pNo; } ;
    void setSender(Member* snd) { m_sender = snd; } ;
    void setCommand(quint32 cmd) { m_command = cmd; } ;
    void setPayload(QString py) { m_payload = py; } ;
    
    QString toString();
    
private:
    quint32 m_packetNo;
    Member* m_sender;
    quint32 m_command;
    QString m_payload;
    
};