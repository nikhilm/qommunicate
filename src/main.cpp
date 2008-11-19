#include <QtGui>
#include "qommunicate.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Qommunicate *qom = new Qommunicate;
    qom->show();

    return app.exec();
}
