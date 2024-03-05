QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    pomodoro_timer.cpp \
    #pomodoro_tray.cpp \
    pomodoro_ui.cpp \
    timerconfig.cpp

HEADERS += \
    pomodoro_timer.h \
    #pomodoro_tray.h \
    pomodoro_ui.h \
    timerconfig.h \
    widgets.h

FORMS += \
    pomodoro_ui.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    icons/book.svg \
    icons/smiley.svg

RESOURCES += \
    icons.qrc
