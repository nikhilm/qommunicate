#ifndef QOM_MESSENGER
#define QOM_MESSENGER

#include <QUdpSocket>
#include <QHostInfo>

#include "constants.h"
#include "ipobjects.h"

class Message
{
public:
    Message(quint32, Member*, quint32, QByteArray);
    
    quint32 packetNo() const { return m_packetNo; } ;
    Member* sender() const { return m_sender; } ;
    quint32 command() const { return m_command; } ;
    QByteArray payload() const { return m_payload; } ;
    
    void setPacketNo(quint32 pNo) { m_packetNo = pNo; } ;
    void setSender(Member* snd) { m_sender = snd; } ;
    void setCommand(quint32 cmd) { m_command = cmd; } ;
    void setPayload(QByteArray py) { m_payload = py; } ;
    
    QString toString();
    QByteArray toAscii();
    static Message fromString(QString);
    static Message fromAscii(QByteArray);
    
private:
    quint32 m_packetNo;
    Member* m_sender;
    quint32 m_command;
    QByteArray m_payload;
    
};


class Messenger : public QObject
{
    Q_OBJECT
public:    
    Messenger();
    ~Messenger()
    {
        socket->close();
        delete socket;
        socket = NULL;        
    };
    
    bool sendMessage(Message, Member*);
    bool sendMessage(quint32, QByteArray, Member*);
    
    bool multicast(quint32, QByteArray);
    
    bool login();
    bool logout();
    void reset();
    
signals:
    void receivedMessage(Message* );
    // TODO: add user-atomic cases
    
    void msg_ansEntry(Message);
    void msg_entry(Message);
    void msg_recvMsg(Message);
    void msg_recvConfirmMsg(Message);
    void msg_fileRecvRequest(Message);
    void msg_getAbsenceInfo(Message);
    void msg_exit(Message);
    
private:
    
    QUdpSocket* socket;
    quint32 m_packetNo;
    
    quint32 packetNo() { return m_packetNo++ ; } ;
    
    QStringList ips() const; // used for login and logout
    
    Message makeMessage(quint32, QByteArray);
    
private slots:
    void receiveData();
};

Member me();
Group myGroup();
Messenger* messenger();
#endif