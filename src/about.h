/*
 * (C) 2009 Nikhil Marathe <nsm.nikhil@gmail.com>
 * Licensed under the GNU General Public License.
 * See LICENSE for details
 */
#ifndef QOM_ABOUT
#define QOM_ABOUT

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

#endif