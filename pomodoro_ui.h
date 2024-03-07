#ifndef POMODORO_UI_H
#define POMODORO_UI_H
#include "widgets.h"
#include "pomodoro_timer.h"
#include "timerconfig.h"

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
class PomodoroTimer;
class TimerConfig;
class PomodoroUI : public QWidget
{
    Q_OBJECT

public:
    PomodoroUI(QWidget *parent = nullptr);
    //PomodoroUI(bool log_stdout = false);
    //Overloaded function for user-determined time frames.
    //PomodoroUI(QWidget *parent = nullptr, bool notify = true, bool log_stdout = false);
    ~PomodoroUI();
protected:
    //per online example, allows overriding window close/minimize events.
    void closeEvent(QCloseEvent *event) override;
private:
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
    const QTime* ZERO_TIME = new QTime(0,0);
    //Actions for menus
    QList<QAction*>* tray_menu_items;
    QMenuBar* top_bar;
    void SetupMenus();
    void connectConfigSignals();
    void UpdateTrayTooltip();

private slots:
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
    void update_study(int);
    void update_break_short(int);
    void update_break_long(int);
    void update_max_pomodoros(int);
    void update_max_cycles(int);
    void update_cycle_limit(bool);
};
#endif // POMODORO_UI_H
