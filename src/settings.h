#include "ui_settings.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent = 0) : QDialog(parent)
    {
        ui.setupUi(this);
    }

private:
    Ui::SettingsDialog ui;
};
