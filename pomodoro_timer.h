#ifndef POMODORO_TIMER_H
#define POMODORO_TIMER_H
#include <iostream>
//Class must be derived from qobject for some reason.
#include "widgets.h"
#include "timerconfig.h"
class QTimer;
class PomodoroTimer : public QObject
{
    Q_OBJECT
public:
    //TODO: Create constructor used to read from config file, or with defaults.
    //Constructor. Takes args for a timer, study time, etc.
    PomodoroTimer(QTimer * timer, int study, int break_s, int break_l, int m_cycles, int m_pomodoros, bool log_stdout, bool c_lim);
    ~PomodoroTimer(); //This deletes the pointer to the timer wrapper.
    //This function checks where we are in a cycle and resets the timer appropriatly.
    void initCycle(bool);
    //Pauses the timer if it is running, resumes it if it is paused.
    void toggleTimer();
    //Adjust segment length specified by int segment to int new_time in seconds.
    void adjustSegment(int segment, int new_time);
    //Method to get the program's status. 0 = studying, 1 = short break, 2 = long break.
    int getStatus();
    void resumeTimer();
    //Method to stop the timer
    void pauseTimer();
    //Reset the session
    void ResetSession();
    void ResetSegment();
    std::string get_len_study_str(){
        return std::to_string(this->len_study);
    }
    std::string get_len_break_s_str(){
        return std::to_string(this->len_break_s);
    }
    std::string get_len_break_l_str(){
        return std::to_string(this->len_break_l);
    }
    std::string get_c_pom_str(){
        return std::to_string(this->c_pomodoros);
    }
    std::string get_c_cycle_str(){
        return std::to_string(this->c_cycle);
    }
    std::string get_m_pom_str(){
        return std::to_string(this->c_pomodoros);
    }
    std::string get_m_cycle_str(){
        return std::to_string(this->c_cycle);
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
    //Method to start the timer.
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
    //Notifications handled by ui class.
private slots:
    void change_segment();
    //NOTE: start() automatically stops a running timer.
    void resetSegment(); //Reset the current segment's timer.
    //void resetSession(); //Reset the session
};

#endif // POMODORO_TIMER_H
