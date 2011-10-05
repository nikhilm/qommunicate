/*
 * (C) 2009 Nikhil Marathe <nsm.nikhil@gmail.com>
 * Licensed under the GNU General Public License.
 * See LICENSE for details
 */
#ifndef QOM_MESSAGEDIALOG
#define QOM_MESSAGEDIALOG

#include <QCloseEvent>
#include <QList>
#include <QUrl>
#include <QTimer>
#include <QMessageBox>

#include "ipobjects.h"
#include "messenger.h"
#include "ui_messagedialog.h"

class MessageDialog : public QDialog
{
    Q_OBJECT

public:
    MessageDialog(Member*, QWidget *parent=0);
    MessageDialog(QList<Member*>, QWidget *parent=0);
    MessageDialog(QWidget *parent=0);
    
    void dragEnterEvent(QDragEnterEvent *evt)
    {
        if(!m_online)
            evt->ignore();
        else if(evt->mimeData()->hasUrls())
        {
            evt->acceptProposedAction();
        }
    };
    
    void dropEvent(QDropEvent *);
    
public slots:
    void incomingMessage(Message);
    void messageRecvConfirm(Message);
    void userOffline(Message);
    void userOnline(Message);
    void reject();

private:
    Ui::MessageDialog ui;
    QList<Member*> receivers;
    QTimer* messageTimer;
    bool m_online;

    void setAttachMenu();
    
private slots:
    void messageTimeout();
    void on_sendButton_clicked();
    void on_actionFiles_triggered();
    void on_actionFolder_triggered();
    void on_messageEdit_anchorClicked(const QUrl &url);
    void on_messageEdit_customContextMenuRequested(const QPoint &pos);
};

#endif
