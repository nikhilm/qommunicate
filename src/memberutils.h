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
    
    static QHash<QString,Member*> get(QString);
    static Member* get(QString, Member*);
    static Member* get(QString, QString); // IP based
    inline static Member* get(const char* c, QString ip) { return get(QString(c), ip); } ;
    
    static Member* remove(QString, Member*);
    inline static Member* remove(const char* c, Member* m) { return remove(QString(c), m); } ;
    static Member* remove(QString, QString);
    inline static Member* remove(const char* c, QString ip) { return remove(QString(c), ip); } ;
    
private:
    static QHash<QString, QHash<QString, Member*> > m_hash;
};

#endif