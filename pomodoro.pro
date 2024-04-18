QT       += core gui help

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    help_browser.cpp \
    main.cpp \
    notification_editor.cpp \
    pomodoro_timer.cpp \
    #pomodoro_tray.cpp \
    pomodoro_ui.cpp \
    preset_editor.cpp \
    preset_list.cpp \
    preset_manager.cpp \
    segment_editor.cpp \
    timerconfig.cpp

HEADERS += \
    help_browser.h \
    pomodoro_timer.h \
    #pomodoro_tray.h \
    pomodoro_ui.h \
    preset_manager.h \
    timerconfig.h \
    widgets.h

FORMS += \
    pomodoro_ui.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    doc_html/doc.qhcp \
    doc_html/doc.qhp \
    doc_html/help.qch \
    icons/book.svg \
    icons/smiley.svg

RESOURCES += \
    icons.qrc
