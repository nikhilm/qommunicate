TEMPLATES = app
QT += network
CONFIG += qt debug
FORMS += about.ui qommunicate.ui messagedialog.ui settings.ui
HEADERS += qommunicate.h about.h settings.h messagedialog.h membermodel.h ipobjects.h messenger.h constants.h
SOURCES += qommunicate.cpp main.cpp settings.cpp messagedialog.cpp membermodel.cpp messenger.cpp
RESOURCES += icons.qrc logo.qrc
