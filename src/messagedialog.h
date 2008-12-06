#ifndef QOM_MESSAGEDIALOG
#define QOM_MESSAGEDIALOG

#include <QStringList>

#include "ui_messagedialog.h"

class MessageDialog : public QDialog
{
    Q_OBJECT

public:
    MessageDialog(QString, QWidget *parent=0);
    MessageDialog(QStringList, QWidget *parent=0);
    MessageDialog(QWidget *parent=0);

private:
    Ui::MessageDialog ui;
    
    QStringList* receivers;
};

#endif