#include "memberutils.h"

QHash<QString, QSet<Member*> > MemberUtils::m_hash;

void MemberUtils::init()
{
}

void MemberUtils::insert(QString group, Member* m)
{
    m_hash[group] << m ;
}

bool MemberUtils::contains(QString group, Member* m)
{
    qDebug() << "contains " << m->addressString() << "?" << m_hash[group].contains(m);
    return m_hash[group].contains(m);
}

QSet<Member*> MemberUtils::get(QString group)
{
    return m_hash[group];
}

/*
 * Returns the Member* within the set. This means
 * you can construct fake Member* with only the IP
 * to get full details of the member if available.
 * Unless you need custom behaviour, use MemberUtils::get(QString, QString)
 */
Member* MemberUtils::get(QString group, Member* m)
{    
    Member* ret = remove(group, m);
    if(ret->isValid())
        insert(group, ret);
    return ret;
}

Member* MemberUtils::get(QString group, QString ip)
{
    Member* m = new Member;
    m->setAddress(ip);
    return get(group, m);
}

Member* MemberUtils::remove(QString group, Member* m)
{
    QList<Member*> list = m_hash[group].toList();
    int index = list.indexOf(m);
    if(index == -1)
        return new Member;
    
    Member* ret = list.takeAt(index);
    m_hash[group] = list.toSet();
    return ret;
}

Member* MemberUtils::remove(QString group, QString ip)
{
    Member* m = new Member;
    m->setAddress(ip);
    return remove(group, m);
}