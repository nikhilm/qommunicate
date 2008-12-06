#ifndef QOM_MESSENGER
#define QOM_MESSENGER

#include <QUdpSocket>
#include <QTcpSocket>
#include <QHostInfo>

#include "constants.h"
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
    static Message fromString(QString);
    
private:
    quint32 m_packetNo;
    Member* m_sender;
    quint32 m_command;
    QString m_payload;
    
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
        
        if(fileSocket)
        {
            fileSocket->close();
            delete fileSocket;
            fileSocket = NULL;
        }
    };
    
    bool sendMessage(Message, Member*);
    bool sendMessage(quint32, QString, Member*);
    
    bool multicast(Message, QList<Member*>);
    bool multicast(quint32, QString, QList<Member*>);
    
    void sendFile(QString);
    
    bool login();
    bool logout();
    void reset();
    
signals:
    void receivedMessage(Message* );
    // TODO: add user-atomic cases
    
private:
    
    QUdpSocket* socket;
    QTcpSocket* fileSocket;
    quint32 m_packetNo;
    
    quint32 packetNo() { return m_packetNo++ ; } ;
    
    QStringList ips() const; // used for login and logout
    
    Message makeMessage(quint32, QString);
    
private slots:
    void receiveData();
};

Messenger * messenger();
#endif