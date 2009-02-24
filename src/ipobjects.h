/*
 * (C) 2009 Nikhil Marathe <nsm.nikhil@gmail.com>
 * Licensed under the GNU General Public License.
 * See LICENSE for details
 */
#ifndef QOM_IPOBJECTS
#define QOM_IPOBJECTS

#include <QStandardItem>
#include <QHostAddress>

const int TYPE_MEMBER = QStandardItem::UserType+1;
const int TYPE_GROUP = QStandardItem::UserType+2;
class Member : public QStandardItem
{
public:
    Member() : QStandardItem()
    {
        m_address.setAddress(QHostAddress::Null);
    };
    Member(QString name, QString host, QString ip, QString status) : QStandardItem()
    {
        setName(name);
        setHost(host);
        setAddress(ip);
        setStatus(status);
    } ;
    
    ~Member()
    {
        m_address.setAddress(QHostAddress::Null);
    }
    int type() const { return TYPE_MEMBER; } ;
    
    QString name() const { return m_name; } ;
    QString host() const { return m_host; } ;
    QHostAddress address() const { return m_address; } ;
    QString addressString() const { return m_address.toString(); } ;
    QString status() const { return m_status; } ;
    
    void setName(const QString nm) {
        m_name = nm ;
        setData(nm, Qt::DisplayRole);
    } ;
    void setHost(const QString h) {
        m_host = h ;
        setData(tooltip(), Qt::ToolTipRole);
    } ;
    
    void setAddress(QString ip) {
        m_address.setAddress(ip);        
        setData(tooltip(), Qt::ToolTipRole);
    } ;
    
    void setStatus(const QString st) {
        m_status = st;
        setData(tooltip(), Qt::ToolTipRole);
    } ;
    
    bool  isValid() { return !m_address.isNull(); } ;
    
    friend bool operator==(Member&, Member&);
    
private:
    QString m_name;
    QString m_host;
    QHostAddress m_address;
    QString m_status;
    
    QString tooltip() {
        return QObject::tr("Host: %1\nIP:%3\nStatus: %2").arg(host()).arg(status()).arg(addressString());
    } ;
};

inline bool operator==(Member& a, Member& b)
{
    return a.address() == b.address();
}

inline uint qHash(const Member& a)
{
    return qHash(a.address());
}

inline uint qHash(Member* m)
{
    return qHash(m->address());
}

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
