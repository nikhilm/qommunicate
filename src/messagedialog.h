#include "ui_messagedialog.h"

class MessageDialog : public QDialog
{
    Q_OBJECT

public:
    MessageDialog(QWidget *parent = 0) : QDialog(parent)
    {
        ui.setupUi(this);
    }

private:
    Ui::MessageDialog ui;
};
