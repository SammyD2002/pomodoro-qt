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
#include "preset_editor.h"
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
    PomodoroUI(PresetManager* preset_manager, QJsonObject* starting_preset, QWidget *parent = nullptr);
    ~PomodoroUI();
public slots:
    void prompt_confirmation(QString Title, QString Message, bool &result, QString accept = QStringLiteral("Yes"), QString reject = QStringLiteral("No"));
signals:
    void get_help();
protected:
    //per online example, allows overriding window close/minimize events.
    void closeEvent(QCloseEvent *event) override;
private:
    const int LEN_LOOP = 100; //Length of loop_timer in ms.
    bool notify; //Marked if notifications are enabled.
    bool warned_tray = false; //Updated to true if/when notification is sent signaling program was closed to tray.
    QString icons_in_use[4];
    QIcon study_icon;
    QIcon breaktime_icon;
    //Ui::PomodoroUI *ui;
    TimerConfig *config;
    QGridLayout *layout;
    QSystemTrayIcon* tray;
    //QMenu *trayIconMenu;
    //The currently active timer.
    QPushButton* toggle;
    //QTimer* main_timer;
    QTimer* loop_timer;
    PomodoroTimer* cycle;
    QLabel* clock;
    QLabel* pc_status;
    QString status[4];
    //Object to handle loading and saving of presets.
    PresetManager* preset_manager;
    QMenu* preset_menus[6];

    //Contains actions added to the tray icon's context menu for use elsewhere.
    QList<QAction*>* tray_menu_items;
    QMenuBar* top_bar;
    //Sets up the menus in th menubar.
    void SetupMenus();
    //Update the time and segment displayed in the tray's tooltips.
    void UpdateTrayTooltip();
    //Disable/Enable the tray and its menus
    void set_tray_enabled(bool);
private slots:
    void retrieve_help();
    void update_timer_display();
    void toggle_pressed();
    void toggled(bool);
    //void restart_timer(); //Should connect to the QTimer::timeout signal.
    void notify_session(int); //Should (If enabled) connect to the segment_changed signal.
    void update_segment(int); //Slot ran when segment changes after time expires.
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
    //Update the entries in the preset menu.
    void preset_added(QAction* set[6]);
    void preset_removed(QAction* set[6]);
};

#endif // POMODORO_UI_H
