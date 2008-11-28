#include "ui_settings.h"

class QSettings;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *);

private slots:
    void on_buttonBox_accepted();
    
    void on_customGrpButton_clicked();
    
private:
    Ui::SettingsDialog ui;
    QSettings *settings;
};
