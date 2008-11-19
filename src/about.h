#include "ui_about.h"

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    AboutDialog(QWidget *parent = 0) : QDialog(parent)
    {
        ui.setupUi(this);
    }

private:
    Ui::AboutDialog ui;
};
