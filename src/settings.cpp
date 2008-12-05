#include <QSettings>
#include <QDir>
#include <QMessageBox>
#include <QInputDialog>

#include "settings.h"

SettingsDialog::SettingsDialog(QWidget *parent = 0) : QDialog(parent)
{
    ui.setupUi(this);
    
    loadSettings();
}

void SettingsDialog::loadSettings()
{    
    settings = new QSettings;
    
    ui.nickEdit->setText( settings->value(tr("nick"), QDir::home().dirName()).toString() );
    ui.groupBox->insertItem(0, settings->value(tr("group"), tr("None")).toString() );
    ui.groupBox->setCurrentIndex(0);
    
    Qt::CheckState popupState = settings->value(tr("popup_messages")).toBool() ? Qt::Checked : Qt::Unchecked ;
    Qt::CheckState soundState = settings->value(tr("play_sound")).toBool() ? Qt::Checked : Qt::Unchecked ;
    Qt::CheckState broadcastState = settings->value(tr("ignore_broadcast")).toBool() ? Qt::Checked : Qt::Unchecked ;
    
    ui.popupCB->setCheckState(popupState);
    ui.playSoundCB->setCheckState(soundState);
    ui.noBroadcastCB->setCheckState(broadcastState);
}

void SettingsDialog::on_buttonBox_accepted()
{
    settings->setValue(tr("nick"), ui.nickEdit->text());
    settings->setValue(tr("group"), ui.groupBox->currentText());
    settings->setValue(tr("popup_messages"), ui.popupCB->isChecked());
    settings->setValue(tr("play_sound"), ui.playSoundCB->isChecked());
    settings->setValue(tr("ignore_broadcast"), ui.noBroadcastCB->isChecked());
    settings->sync();
}

void SettingsDialog::on_customGrpButton_clicked()
{
    bool ok;
    QString grp = QInputDialog::getText(this, tr("New Group"), tr("Enter name of the new group"), QLineEdit::Normal, "", &ok);
    
    if(ok && !grp.isEmpty())
    {
        ui.groupBox->addItem(grp);
        ui.groupBox->setCurrentIndex(ui.groupBox->count()-1);
    }
}