TEMPLATES = app
QT += network
CONFIG += qt debug
FORMS += about.ui qommunicate.ui messagedialog.ui settings.ui
HEADERS += qommunicate.h about.h settings.h messagedialog.h membermodel.h ipobjects.h
SOURCES += qommunicate.cpp main.cpp settings.cpp messagedialog.cpp membermodel.cpp
RESOURCES += icons.qrc logo.qrc
