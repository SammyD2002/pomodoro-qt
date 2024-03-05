#include "pomodoro_ui.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PomodoroUI w;
    //w.show();
    return a.exec();
}
