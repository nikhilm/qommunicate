#include "ui_messagedialog.h"

class MessageDialog : public QDialog
{
    Q_OBJECT

public:
    MessageDialog(QString with, QWidget *parent = 0) : QDialog(parent)
    {
        ui.setupUi(this);
        setWindowTitle(tr("Chat with %1").arg(with));
    }

private:
    Ui::MessageDialog ui;
};
