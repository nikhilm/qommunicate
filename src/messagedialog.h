#ifndef QOM_MESSAGEDIALOG
#define QOM_MESSAGEDIALOG

#include <QCloseEvent>
#include <QList>

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
    
protected:
    void closeEvent(QCloseEvent*);
    
public slots:
    void incomingMessage(Message);

private:
    Ui::MessageDialog ui;
    
    QList<Member*> receivers;
};

#endif