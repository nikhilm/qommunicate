#include "ui_qommunicate.h"

class Qommunicate : public QMainWindow
{
    Q_OBJECT

public:
    Qommunicate(QWidget *parent = 0);

private slots:
    void on_searchEdit_textChanged(const QString &);
    void on_action_About_triggered();
    void on_action_Settings_triggered();
    void on_actionBroadcast_triggered();
    void on_actionQuit_triggered();

private:
    Ui::MainWindow ui;
};
