/*
 * (C) 2009 Nikhil Marathe <nsm.nikhil@gmail.com>
 * Licensed under the GNU General Public License.
 * See LICENSE for details
 */
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
    QString group = settings->value(tr("group"),"").toString();
    ui.groupBox->addItems( settings->value(tr("groups"), QVariantList()).toStringList() );
    ui.groupBox->setCurrentIndex( ui.groupBox->findText(group) );
    
    //Qt::CheckState popupState = settings->value(tr("popup_messages")).toBool() ? Qt::Checked : Qt::Unchecked ;
    //Qt::CheckState soundState = settings->value(tr("play_sound")).toBool() ? Qt::Checked : Qt::Unchecked ;
    Qt::CheckState receiveState = settings->value(tr("no_receive")).toBool() ? Qt::Checked : Qt::Unchecked ;
    Qt::CheckState showMulticastPopup = settings->value(tr("showMulticastPopup")).toBool() ? Qt::Checked : Qt::Unchecked;
    
    //ui.popupCB->setCheckState(popupState);
    //ui.playSoundCB->setCheckState(soundState);
    ui.noReceiveCB->setCheckState(receiveState);
    ui.multicastPopupCB->setCheckState(showMulticastPopup);
    ui.multicastPopupCB->setEnabled(receiveState);
}

void SettingsDialog::on_buttonBox_accepted()
{
    settings->setValue(tr("nick"), ui.nickEdit->text());

    QString group = ui.groupBox->lineEdit()->text();
    if (ui.groupBox->findText(group) < 0) {
        ui.groupBox->insertItem(0, group);
        ui.groupBox->setCurrentIndex(0);
    }
    else {
        ui.groupBox->setCurrentIndex ( ui.groupBox->findText(group) );
    }
    settings->setValue(tr("group"), ui.groupBox->currentText());

    QStringList groups;
    for (int i=0; i<ui.groupBox->count(); i++) {
        groups << ui.groupBox->itemText(i);
    }
    settings->setValue(tr("groups"), groups);

    //settings->setValue(tr("popup_messages"), ui.popupCB->isChecked());
    //settings->setValue(tr("play_sound"), ui.playSoundCB->isChecked());
    settings->setValue(tr("no_receive"), ui.noReceiveCB->isChecked());
    settings->setValue(tr("showMulticastPopup"), ui.multicastPopupCB->isChecked());
    settings->sync();
    emit settingsChanged();
}

void SettingsDialog::dropEvent(QDropEvent *evt)
{
    QFile file(evt->mimeData()->urls()[0].toLocalFile()); 
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(this, tr("Error"), tr("Error reading file %1").arg(file.fileName()));
        return;
    }
    
    int i = 0;
    while(!file.atEnd())
    {
        QString line = QString(file.readLine());
        line.chop(1);
        if(!line.contains('='))
            continue;
        
        QString ip = line.split('=')[1].remove('"');
        if(!ip.contains(QRegExp("[0-255.]*")))
            continue;
        
        settings->setValue(QString("ips/ip%1").arg(i++), ip);
    }
    
    settings->sync();
}
