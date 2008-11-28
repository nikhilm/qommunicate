#include "messagedialog.h"

MessageDialog::MessageDialog(QString title, QWidget *parent)
{
    ui.setupUi(this);
    setWindowTitle(title);
}