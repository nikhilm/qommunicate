#include "ui_messagedialog.h"

class MessageDialog : public QDialog
{
    Q_OBJECT

public:
    MessageDialog(QString title, QWidget *parent = 0) : QDialog(parent)
    {
        ui.setupUi(this);
        setWindowTitle(title);
    }

private:
    Ui::MessageDialog ui;
};
