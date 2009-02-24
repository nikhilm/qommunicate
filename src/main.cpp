/*
 * (C) 2009 Nikhil Marathe <nsm.nikhil@gmail.com>
 * Licensed under the GNU General Public License.
 * See LICENSE for details
 */
#include <QtGui>
#include "qommunicate.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName(QT_TR_NOOP("qommunicate"));
    app.setApplicationName(QT_TR_NOOP("qommunicate"));
    app.setApplicationVersion(QT_TR_NOOP("1.0"));

    app.setQuitOnLastWindowClosed(false);
    Qommunicate *qom = new Qommunicate;
    qom->show();

    return app.exec();
}
