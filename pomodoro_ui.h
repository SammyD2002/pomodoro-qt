#ifndef POMODORO_UI_H
#define POMODORO_UI_H
#include "widgets.h"
#include "pomodoro_timer.h"

/*QT_BEGIN_NAMESPACE
namespace Ui {
class PomodoroUI;
}
QT_END_NAMESPACE*/
class QActionGroup;
class QGridLayout;
class QLabel;
class QPushButton;
class QTime;
class QTimer;
class QSystemTrayIcon;
class QWidget;
class PomodoroTimer;
class PomodoroUI : public QWidget
{
    Q_OBJECT

public:
    PomodoroUI(QWidget *parent = nullptr, bool notify = true, bool log_stdout = false);
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
    QGridLayout *layout;
    QSystemTrayIcon* tray;
    //QMenu *trayIconMenu;
    //The currently active timer.
    QPushButton* toggle;
    QTimer* main_timer;
    QTimer* loop_timer = new QTimer(this);
    PomodoroTimer* cycle;
    QLabel* clock;
    std::string status[2];
    const QTime* ZERO_TIME = new QTime(0,0);
    //Actions for menus
    QList<QAction*>* tray_menu_items;
    QAction* quit;
    QAction* toggle_display;
    QAction* toggle_timer;
    void SetupMenus();
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
    //Override Close and Minimize Slots
};
#endif // POMODORO_UI_H
