TEMPLATES = app
QT += network
CONFIG += qt debug
FORMS += about.ui qommunicate.ui messagedialog.ui settings.ui
HEADERS += qommunicate.h about.h settings.h messagedialog.h membermodel.h ipobjects.h messenger.h constants.h memberutils.h fileutils.h sendfileprogressdialog.h recvfileprogressdialog.h
SOURCES += qommunicate.cpp main.cpp settings.cpp messagedialog.cpp membermodel.cpp messenger.cpp fileutils.cpp memberutils.cpp sendfileprogressdialog.cpp recvfileprogressdialog.cpp
RESOURCES += icons.qrc logo.qrc
