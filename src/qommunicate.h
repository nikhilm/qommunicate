#ifndef QOM_QOMMUNICATE
#define QOM_QOMMUNICATE

#include <QSystemTrayIcon>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include "membermodel.h"
#include "messenger.h"
#include "memberutils.h"

#include "ui_qommunicate.h"

class QTcpSocket;

class Qommunicate : public QMainWindow
{
    Q_OBJECT

public:
    Qommunicate(QWidget *parent = 0);
    void dialogOpened(Member*);
    void dialogClosed(Member*);
    
protected:
    void closeEvent(QCloseEvent *);

private slots:
    void on_searchEdit_textChanged(const QString &);
    
    void on_action_About_triggered();
    void on_action_Settings_triggered();
    void on_actionMulticast_triggered();
    void on_actionQuit_triggered();
    
    void on_memberTree_doubleClicked(const QModelIndex&);
    void on_statusCombo_currentIndexChanged(const QString&);
    
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    
    
    void cleanup();
    
    // Incoming message related slots
    void addMember(Message);
    void addMemberAndAnswer(Message);
    void openDialog(Message);
    void sendAbsenceInfo(Message);
    void removeMember(Message msg);
    
    void fileSendRequested(QTcpSocket*);
    void fileRecvRequested(Message);
    void fileRecvDone(QString);

private:
    Ui::MainWindow ui;
    
    QSystemTrayIcon *trayIcon;
    QLabel memberCountLabel;
    
    MemberModel *model;
    MemberFilter *filterModel;
        
    void createTrayIcon();
    void populateTree();
    void firstRun();
    
    void keyPressEvent(QKeyEvent *);
    bool createGroupMemberList(QStandardItem*, QSet<Member*>&);
    
    void notify(const QString&, const QString&, bool dialog=false);
};

#endif