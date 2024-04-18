/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#include "pomodoro_ui.h"

#include <QApplication>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PomodoroUI w;
    //w.show();
    return a.exec();
}
