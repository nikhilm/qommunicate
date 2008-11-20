#include <QMessageBox>

#include "about.h"
#include "settings.h"
#include "qommunicate.h"

Qommunicate::Qommunicate(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
}

void Qommunicate::on_searchEdit_textChanged(const QString &text)
{
    QMessageBox::information(this, tr("Text Changed"), text);
}

void Qommunicate::on_action_About_triggered()
{
    AboutDialog(this).exec();
}

void Qommunicate::on_action_Settings_triggered()
{
    SettingsDialog(this).exec();
}

void Qommunicate::on_actionBroadcast_triggered()
{
    //BroadcastDialog(this).exec();
}

void Qommunicate::on_actionQuit_triggered()
{
    qApp->quit();
}