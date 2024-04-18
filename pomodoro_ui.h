/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#ifndef POMODORO_UI_H
#define POMODORO_UI_H
#include "widgets.h"
#include "pomodoro_timer.h"
#include "help_browser.h"
#include "timerconfig.h"
//Use namespace as suggested at https://stackoverflow.com/questions/2268749/defining-global-constant-in-c
/*QT_BEGIN_NAMESPACE
namespace Ui {
class PomodoroUI;
}
QT_END_NAMESPACE*/
class QActionGroup;
class QMenuBar;
class QGridLayout;
class QLabel;
class QPushButton;
class QTime;
class QTimer;
class QSystemTrayIcon;
class QWidget;
class QMessageBox;

class PomodoroTimer;
class TimerConfig;
class PomodoroUI : public QWidget
{
    Q_OBJECT

public:
    PomodoroUI(QWidget *parent = nullptr);
//static help* timer_help;
    //PomodoroUI(bool log_stdout = false);
    //Overloaded function for user-determined time frames.
    //PomodoroUI(QWidget *parent = nullptr, bool notify = true, bool log_stdout = false);
    ~PomodoroUI();
public slots:
    void preset_added(QAction* load, QAction* del, QAction* edit, QAction* ren, QAction* def);
    void preset_removed(QAction* load, QAction* del, QAction* edit, QAction* ren, QAction* def);
    void prompt_confirmation(QString Title, QString Message, bool &result, QString accept = QString("Yes"), QString reject = QString("No"));
signals:
    void get_help();
protected:
    //per online example, allows overriding window close/minimize events.
    void closeEvent(QCloseEvent *event) override;
private:
    const int LEN_LOOP = 200; //Length of loop_timer in ms.
    bool notify; //Marked if notifications are enabled.
    bool log_stdout;
    bool warned_tray = false; //Updated to true if/when notification is sent signaling program was closed to tray.
    QIcon study_icon;
    QIcon breaktime_icon;
    //Ui::PomodoroUI *ui;
    TimerConfig *config;
    QGridLayout *layout;
    QSystemTrayIcon* tray;
    //QMenu *trayIconMenu;
    //The currently active timer.
    QPushButton* toggle;
    QTimer* main_timer;
    QTimer* loop_timer;
    PomodoroTimer* cycle;
    QLabel* clock;
    QLabel* pc_status;
    std::string status[2];
    //Object to handle loading and saving of presets.
    PresetManager* preset_manager;
    QMenu* load_preset_menu;
    QMenu* del_preset_menu;
    QMenu* edit_preset_menu;
    QMenu* rename_preset_menu;
    QMenu* new_default_preset_menu;
    //QMenu* edit_preset_menu;
    //QMenu* rename_preset_menu;
    //Actions for menus
    QList<QAction*>* tray_menu_items;
    QMenuBar* top_bar;
    void SetupMenus();
    //void connectConfigSignals();
    void UpdateTrayTooltip();
private slots:
    void retrieve_help();
    void update_timer_display();
    void toggle_pressed();
    void toggled(bool);
    //void restart_timer(); //Should connect to the QTimer::timeout signal.
    void notify_session(int); //Should (If enabled) connect to the segment_changed signal.
    void update_segment(int);
    void window_toggle(QSystemTrayIcon::ActivationReason reason);
    //Called to quit the app and send a goodbye notice. Reason: activate requires a bool param.
    void quitting();
    void update_visible();
    //Start Configuration
    void start_config();
    void finish_config();
    void settings_to_preset(QAction*);
    void rename_preset(QAction*);
    void attempt_preset_load(QAction*);
    void attempt_preset_remove(QAction*);
    void attempt_preset_edit(QAction*);
    void attempt_update_default(QAction*);
    /*void update_study(int);
    void update_break_short(int);
    void update_break_long(int);
    void update_max_pomodoros(int);
    void update_max_cycles(int);
    void update_cycle_limit(bool);*/
};

#endif // POMODORO_UI_H
