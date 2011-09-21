/*
 * (C) 2009 Nikhil Marathe <nsm.nikhil@gmail.com>
 * Licensed under the GNU General Public License.
 * See LICENSE for details
 */
#ifndef QOM_MEMBERUTILS
#define QOM_MEMBERUTILS

#include <QHash>

#include "ipobjects.h"

class MemberUtils
{
public:
    static void init();
    
    static void insert(QString, Member*);
    inline static void insert(const char* c, Member* m) { MemberUtils::insert(QString::fromAscii(c), m); } ;
    
    static bool contains(QString, Member*);
    inline static bool contains(const char* c, Member *m) { return contains(QString(c), m); } ;
    
    static Member* get(QString, Member*);
    static Member* get(QString, QString); // IP based
    inline static Member* get(const char* c, QString ip) { return get(QString(c), ip); } ;
    
    static void remove(QString, Member*);
    inline static void remove(const char* c, Member* m) { return remove(QString(c), m); } ;
    static void remove(QString, QString);
    inline static void remove(const char* c, QString ip) { return remove(QString(c), ip); } ;

    inline static void clear() { m_hash.clear(); } ;
    
private:
    static QHash<QString, QHash<QString, Member*> > m_hash;
};

#endif
