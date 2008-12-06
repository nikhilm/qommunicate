#ifndef QOM_IPOBJECTS
#define QOM_IPOBJECTS

#include <QStandardItem>
#include <QHostAddress>

const int TYPE_MEMBER = QStandardItem::UserType+1;
const int TYPE_GROUP = QStandardItem::UserType+2;
class Member : public QStandardItem
{
public:
    Member() : QStandardItem() {};
    Member(QString name, QString host, QString ip, QString status) : QStandardItem()
    {
        setName(name);
        setHost(host);
        setAddress(ip);
        setStatus(status);
    } ;
    int type() const { return TYPE_MEMBER; } ;
    
    QString name() const { return m_name; } ;
    QString host() const { return m_host; } ;
    QHostAddress* address() const { return m_address; } ;
    QString status() const { return m_status; } ;
    
    void setName(const QString nm) {
        m_name = nm ;
        setData(nm, Qt::DisplayRole);
    } ;
    void setHost(const QString h) {
        m_host = h ;
        setData(tooltip(), Qt::ToolTipRole);
    } ;
    
    void setAddress(QString ip) { m_address = new QHostAddress(ip); } ;
    
    void setStatus(const QString st) {
        m_status = st;
        setData(tooltip(), Qt::ToolTipRole);
    } ;
    
    friend bool operator==(Member& a, Member& b)
    {
        return a.address() == b.address();
    };
    
private:
    QString m_name;
    QString m_host;
    QHostAddress* m_address;
    QString m_status;
    
    QString tooltip() {
        return QObject::tr("Host: %1\nStatus: %2").arg(host()).arg(status());
    } ;
};

class Group : public QStandardItem
{
public:
    Group(QString name) : QStandardItem() {
        setName(name);
    };
    int type() const { return TYPE_GROUP; } ;
    
    QString name() { return m_name; } ;
    
    void setName(const QString nm) {
        m_name = nm ;
        setData(nm, Qt::DisplayRole);
    } ;
    
    friend bool operator==(Group&, Group&);
private:
    QString m_name;
};

#endif
