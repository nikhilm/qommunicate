#include <QSettings>
#include <QMessageBox>
#include <QInputDialog>

#include "settings.h"

SettingsDialog::SettingsDialog(QWidget *parent = 0) : QDialog(parent)
{
    ui.setupUi(this);
    ui.groupBox->insertItem(0, "None", QString());
    ui.groupBox->setCurrentIndex(0);
    settings = new QSettings;
}

void SettingsDialog::on_buttonBox_accepted()
{
    QMessageBox::information(this, tr("Saving"), settings->fileName());
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