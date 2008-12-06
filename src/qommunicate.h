#ifndef QOM_QOMMUNICATE
#define QOM_QOMMUNICATE

#include <QSystemTrayIcon>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include "membermodel.h"
#include "ui_qommunicate.h"

class Qommunicate : public QMainWindow
{
    Q_OBJECT

public:
    Qommunicate(QWidget *parent = 0);
    
protected:
    void closeEvent(QCloseEvent *);

private slots:
    void on_searchEdit_textChanged(const QString &);
    
    void on_action_About_triggered();
    void on_action_Settings_triggered();
    void on_actionMulticast_triggered();
    void on_actionQuit_triggered();
    
    void on_memberTree_doubleClicked(const QModelIndex&);
    
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    
    void cleanup();

private:
    Ui::MainWindow ui;
    
    QSystemTrayIcon *trayIcon;
    
    MemberModel *model;
    MemberFilter *filterModel;
    
    void createTrayIcon();
    void populateTree();
    void firstRun();
    
    void keyPressEvent(QKeyEvent *);
};

#endif