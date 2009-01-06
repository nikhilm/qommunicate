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
        if(evt->mimeData()->hasUrls())
        {
            evt->acceptProposedAction();
        }
    };
    
    void dropEvent(QDropEvent *);
    
protected:
    void closeEvent(QCloseEvent*);
    
public slots:
    void incomingMessage(Message);
    void messageRecvConfirm();
    void reject();

private:
    Ui::MessageDialog ui;
    
    QList<Member*> receivers;
    
    QTimer* messageTimer;
    
private slots:
    void messageTimeout();
    void on_sendButton_clicked();
};

#endif