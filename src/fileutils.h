/*
 * (C) 2009 Nikhil Marathe <nsm.nikhil@gmail.com>
 * Licensed under the GNU General Public License.
 * See LICENSE for details
 */
#ifndef QOM_FILEUTILS
#define QOM_FILEUTILS

#include <QTcpSocket>
#include <QTcpServer>

#include "messenger.h"
#include "ipobjects.h"

struct FileInfo
{
    QString fileName;
    int fileID;
    qint64 size;
    uint mtime;
    qint64 offset;
    QHash<QString,QString> xattrs;
    int type;
};

class FileTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    FileTcpServer(QObject* parent=0) : QTcpServer(parent)
    {
    };
    
protected:
    void incomingConnection(int socketDescriptor)
    {
        emit incomingConnectionDescriptor(socketDescriptor);
    }
    
signals:
    void incomingConnectionDescriptor(int);
};

class FileUtils : public QObject
{
    Q_OBJECT
    
public:
    FileUtils();
    ~FileUtils() { } ;
    
    // UDP
    QString formatSendFilesRequest(QStringList);
    
    // usually shouldn't be called from outside, see the .cpp for details
    void sendDirectoryRequest(QString, Member*);
    
    // TCP
    //void sendFiles(QStringList);
    //void sendDirectory(QString);
    QList<FileInfo> directoryInfoList(const QString&);
    QString formatFileHeader(const FileInfo&);
    
    void sendFilesUdpRequest(QStringList, Member*, QString);
signals:
    /**
     * Emitted every time a certain number of bytes are transmitted
     * QString is the filename
     * int is the bytes written till now
     * int is the total bytes
     */
    void filePath(QString);
    void newFileSendSocket(QTcpSocket*);
    
public slots:
    QString resolveFilePath(int);
    
private:
    int m_id;
    
    QTcpServer* m_server;
    QHash<int, QString> m_fileIdHash;
    
    int nextId() { return m_id++ ; } ;
    QString formatFileData(QString);
    
private slots:
    // void startSendFile();
    void newConnection();
};

FileUtils* fileUtils();

#endif