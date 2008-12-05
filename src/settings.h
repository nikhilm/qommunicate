#include <QDragEnterEvent>
#include <QMessageBox>
#include <QUrl>

#include "ui_settings.h"

class QSettings;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *);
    
    ~SettingsDialog()
    {
        delete settings;
        settings = NULL;
    };
    
    void dragEnterEvent(QDragEnterEvent *evt)
    {
        if(evt->mimeData()->hasUrls())
        {
            evt->acceptProposedAction();
        }
    };
    
    void dropEvent(QDropEvent *);

private slots:
    void on_buttonBox_accepted();
    
    void on_customGrpButton_clicked();
    
private:
    Ui::SettingsDialog ui;
    QSettings *settings;
    
    void loadSettings();
};
