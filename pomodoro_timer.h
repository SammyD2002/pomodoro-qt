#ifndef POMODORO_TIMER_H
#define POMODORO_TIMER_H
#include <iostream>
//Class must be derived from qobject for some reason.
#include "widgets.h"
#include "timerconfig.h"
#include "preset_manager.h"
class QTimer;
class QDateTime;
class PomodoroTimer : public QWidget
{
    Q_OBJECT
public:
    const QDateTime* ZERO_TIME = new QDateTime(QDate(1,1,1), QTime(0,0)); //Year/Month/Day 0 is invalid.
    //TODO: Create constructor used to read from config file, or with defaults.
    //Constructor. Takes args for a timer, study time, etc.
    PomodoroTimer(QTimer * timer, double study, double break_s, double break_l, int units[], int m_cycles, int m_pomodoros, bool log_stdout, bool c_lim, QWidget* parent);
    PomodoroTimer(QTimer* timer, const QJsonObject* preset, bool log_stdout, QWidget* parent);

    ~PomodoroTimer(); //This deletes the pointer to the timer wrapper.
    QJsonObject settingsToJson(QString name);
    //This function checks where we are in a cycle and resets the timer appropriatly.
    void initCycle(bool);
    //Pauses the timer if it is running, resumes it if it is paused.
    void toggleTimer();
    //Method to get the program's status. 0 = studying, 1 = short break, 2 = long break.
    int getStatus();
    void resumeTimer();
    //Method to stop the timer
    void pauseTimer();
    //Get and set notifications.
    QString getMessageTitle(int status);
    QString getMessageTitleTemplate(int status);
    QString getMessageBody(int status);
    QString getMessageBodyTemplate(int status);
    //Get Current Preset
    QJsonObject getPresetJson(QString name);
    void constructSettingsJson(int units[3]);
    //Reset the session
    void ResetSession();
    void ResetSegment();
    QString get_len_study_str(){
        return QString::number(this->len_study);
    }
    QString get_len_break_s_str(){
        return QString::number(this->len_break_s);
    }
    QString get_len_break_l_str(){
        return QString::number(this->len_break_l);
    }
    QString get_c_pom_str(){
        return QString::number(this->c_pomodoros);
    }
    QString get_c_cycle_str(){
        return QString::number(this->c_cycle);
    }
    QString get_m_pom_str(){
        return QString::number(this->m_pomodoros);
    }
    QString get_m_cycle_str(){
        return QString::number(this->m_cycles);
    }
    int get_len_study_int(){
        return this->len_study;
    }
    int get_len_break_s_int(){
        return this->len_break_s;
    }
    int get_len_break_l_int(){
        return this->len_break_l;
    }
    int get_c_cycle_int(){
        return this->c_cycle;
    }
    int get_c_pom_int(){
        return this->c_pomodoros;
    }
    int get_m_cycle_int(){
        return this->m_cycles;
    }
    int get_m_pom_int(){
        return this->m_pomodoros;
    }
    bool is_cycle_lim_enabled(){
        return this->c_limit_enabled;
    }
    void toggle_log_stdout(bool nval){
        this->log_stdout = nval;
    }

public slots:
    //Adjust segment length specified by int segment to int new_time in seconds.
    void adjustSegment(int segment, int new_time);
    void setMessageTitles(bool updated[6], QString new_messages[6]);
    void setMessageBodies(bool updated[6], QString new_messages[6]);
    //Apply a preset from a QJsonObject passed as an argument:
    bool applyPreset(const QJsonObject* preset);

signals:
    //Emitted when the current segment changes. The int signals the context of the change.
    //0 = Session started, 1 = study -> short break, 2 = study -> long break,
    //3 = x break -> study, 4 = session over, 5 = session restart.
    void segment_changed(int);
    void timer_toggled(bool); //Emitted when the timer is paused or resumed, with a resumed timer having true passed as an argument.
private:
    //NOTE: QTimer::timerId() returns -1 if the timer is stopped. Also, time is set in ms, so consider this for start & stop functions.
    //Wrapper to hold the timer pointer and its state.
    struct timerWrapper{
        QTimer* timer;
        //True if timer is paused. If it is paused, then rem should be set to the timer's remaining time.
        //Paused replaced by a call to QTimer::isActive().
        int rem;
    };
    timerWrapper* timerInfo;
    //Create Json for currentPreset that is updated with each edit to the timer.
    QJsonObject* currentPreset = NULL;
    bool log_stdout;

    //Timer vars
    //The cycle the program is currently on.
    int c_cycle;
    //The cycle that the timer will not restart upon completion.
    int m_cycles;
    //Checked before enforcing m_cycles limit:
    bool c_limit_enabled;
    //Number of pomodoros completed in cycle.
    int c_pomodoros;
    //Number of pomodoros per cycle.
    int m_pomodoros;
    //Length of the study, short break, and long break sessions.
    int len_study;
    int len_break_s;
    int len_break_l;
    //Are we studying or on break?
    bool studying;
    //Sending of notifications handled by ui class.
    //Notification Messages:
    //Pointers to notification structs that hold the messages.
    /*
    0: notification* session_start;
    1: notification* study_complete
    2: notification* break_s_complete;
    3: notification* cycle_complete;
    4: notification* session_complete;
    5: notification* session_restart;
    */
    //NOTE: Only message titles can be set here due to the use of other vars in the message body.
    const QString DEFAULT_TITLES[6] = {
        QString::fromStdString("Starting Study Session"),
        QString::fromStdString("Study Segment Complete"),
        QString::fromStdString("Study Cycle Complete"),
        QString::fromStdString("Short Break Complete"),
        QString::fromStdString("Study Session Complete"),
        QString::fromStdString("Restarting Study Session")
    };
    /* Strings to replace:
     * Number of x
     *  <current_pomodoro>: The current pomodoro
     *  <pomodoros_per_cycle>: The number of pomodoros in each cycle.
     *  <current_cycle>: The current cycle
     *  <cycles_per_session>: The number of cycles in each session. Returns ∞ if no cycle limit set.
     * Length of x
     *  <len_study>
     *  <len_break_s>
     *  <len_break_l>
     */
    const QString DEFAULT_MESSAGES[6] = {
        QString::fromStdString("Good luck!"),
        QString::fromStdString(("Nice job out there. You have completed <current_pomodoro> pomodoros.\nEnjoy your short break!")),
        QString::fromStdString(("Congratulations! You have completed <current_pomodoro> pomodoros, and have earned your self a long break!")),
        QString::fromStdString("Hope you enjoyed the break! Now, GET BACK TO WORK!"),
        QString::fromStdString("Congratulations! Hope you got a lot done!"),
        QString::fromStdString("Time to get some more work done!")
    };
    //Current Notification message.
    QString* titles;
    QString* messages;
    //Methods
    QString constructOutput(QString template_string);
private slots:
    void change_segment();
    //NOTE: start() automatically stops a running timer.
    void resetSegment(); //Reset the current segment's timer.
    //void resetSession(); //Reset the session
};

#endif // POMODORO_TIMER_H
