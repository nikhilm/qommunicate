TEMPLATES = app
QT += network
CONFIG += qt debug
FORMS += about.ui qommunicate.ui messagedialog.ui settings.ui
HEADERS += qommunicate.h about.h settings.h messagedialog.h membermodel.h ipobjects.h messenger.h constants.h memberutils.h fileutils.h sendfileprogressdialog.h recvfileprogressdialog.h
SOURCES += qommunicate.cpp main.cpp settings.cpp messagedialog.cpp membermodel.cpp messenger.cpp fileutils.cpp memberutils.cpp sendfileprogressdialog.cpp recvfileprogressdialog.cpp
RESOURCES += icons.qrc logo.qrc

unix {
    INSTALL_PREFIX = /usr
    icon48.path = $${INSTALL_PREFIX}/share/icons/hicolor/48x48/apps
    icon48.files = icons/48x48/qommunicate.png
    icon128.path = $${INSTALL_PREFIX}/share/icons/hicolor/128x128/apps
    icon128.files = icons/128x128/qommunicate.png
    desktop.path = $${INSTALL_PREFIX}/share/applications
    desktop.files = qommunicate.desktop
    target.path = $${INSTALL_PREFIX}/bin

    INSTALLS += target icon48 icon128 desktop
}

