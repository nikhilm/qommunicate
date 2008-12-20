#include "memberutils.h"

QHash<QString, QHash<QString, Member*> > MemberUtils::m_hash;

void MemberUtils::init()
{
}

void MemberUtils::insert(QString group, Member* m)
{
    m_hash[group][m->addressString()] = m ;
}

bool MemberUtils::contains(QString group, Member* m)
{
    return m_hash[group].contains(m->addressString());
}

QHash<QString, Member*> MemberUtils::get(QString group)
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
    Member* ret = m_hash[group][m->addressString()];
    m_hash[group].remove(m->addressString());
    return ret;
}

Member* MemberUtils::remove(QString group, QString ip)
{
    Member* m = new Member;
    m->setAddress(ip);
    return remove(group, m);
}