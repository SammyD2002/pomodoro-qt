/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#ifndef POMODORO_TIMER_H
#define POMODORO_TIMER_H
//Class must be derived from qobject for some reason.
#include "widgets.h"
#include "preset_manager.h"
class QTimer;
class QDateTime;
class PomodoroTimer : public QWidget
{
    Q_OBJECT
public:
    const QDateTime* ZERO_TIME = new QDateTime(QDate(1,1,1), QTime(0,0)); //Year/Month/Day 0 is invalid.
    //Constructor. Takes args for a timer, study time, etc.
    PomodoroTimer(double study, double break_s, double break_l, int units[], int m_cycles, int m_pomodoros, bool c_lim, QWidget* parent);
    //Alternate Constructor that takes a pointer to the default preset.
    PomodoroTimer(const QJsonObject* preset, QWidget* parent);
    ~PomodoroTimer(); //This deletes the pointer to the timer wrapper.
    QJsonObject settingsToJson(QString name) const;
    //Start the session. Pass 'true' to display the restart message.
    void initCycle(bool);
    //Pauses the timer if it is running, resumes it if it is paused.
    void toggleTimer();
    //Method to get the program's status. 0 = studying, 1 = short break, 2 = long break.
    int getStatus() const;
    void resumeTimer();
    //Method to stop the timer
    void pauseTimer();
    //Get and set notifications.
    QString getMessageTitle(int status) const;
    QString getMessageTitleTemplate(int status) const;
    QString getMessageBody(int status) const;
    QString getMessageBodyTemplate(int status) const;
    //Get Current Preset
    QJsonObject getPresetJson() const;
    QString* getRunningPresetName() const;
    void constructSettingsJson(int units[3], QString *name = nullptr);
    //Reset the session
    void ResetSession();
    void ResetSegment();
    //Gets time left on the timer.
    QString getTimeRemaining(){
        /*QDateTime remTime;
        if(this->timerInfo->timer->isActive())
            remTime = this->ZERO_TIME->addMSecs((this->timerInfo->timer->remainingTime() / 1000) * 1000);
        else
            remTime = this->ZERO_TIME->addMSecs((this->timerInfo->rem / 1000) * 1000);
        */
        QDateTime remTime = this->ZERO_TIME->addMSecs((this->timerInfo->timer->remainingTime() / 1000) * 1000);
        QString output = remTime.toString("hh:mm:ss");
        if (this->ZERO_TIME->date().daysTo(remTime.date()) >= 1)
            output = QString::number(this->ZERO_TIME->date().daysTo(remTime.date())) + ":" + output;
        return output;
    }
    int getPercentElapsed(){
        //Unchanged if segment = 3.
        float rem = 0;
        float dur = 1;
        if(this->timerInfo->timer->isActive())
            rem = this->timerInfo->timer->remainingTime();
        else
            rem = this->timerInfo->rem;
        switch (this->getCurrentSegment()){
        case 0:
            //studying
            dur = this->len_study;
            break;
        case 1:
            //s_break
            dur = this->len_break_s;
            break;
        case 2:
            //l_break
            dur = this->len_break_l;
            break;
        }
        return 100 - ((rem/dur) * 100);
    }
    int getCurrentSegment(){return this->current_segment;}
    //Functions used to retrieve the value as a string or its native type.
    QString get_len_study_str() const {return QString::number(this->len_study);}
    QString get_len_break_s_str() const {return QString::number(this->len_break_s);}
    QString get_len_break_l_str() const {return QString::number(this->len_break_l);}
    QString get_c_pom_str() const {return QString::number(this->c_pomodoros);}
    QString get_c_cycle_str() const {return QString::number(this->c_cycle);}
    QString get_m_pom_str() const {return QString::number(this->m_pomodoros);}
    QString get_m_cycle_str() const {return QString::number(this->m_cycles);}
    int get_len_study_int() const {return this->len_study;}
    int get_len_break_s_int() const {return this->len_break_s;}
    int get_len_break_l_int() const {return this->len_break_l;}
    int get_c_cycle_int() const {return this->c_cycle;}
    int get_c_pom_int() const {return this->c_pomodoros;}
    int get_m_cycle_int() const {return this->m_cycles;}
    int get_m_pom_int() const {return this->m_pomodoros;}
    QString get_icon_name(int status) const;
    bool is_cycle_lim_enabled() const {return this->c_limit_enabled;}
public slots:
    //Adjust segment length specified by int segment to int new_time in seconds.
    void adjustSegment(int segment, int new_time);
    void setMessageTitles(bool updated[6], QString new_messages[6]);
    void setMessageBodies(bool updated[6], QString new_messages[6]);
    void update_icons(const QStringList &icons);
    //Apply a preset from a QJsonObject passed as an argument:
    bool applyPreset(const QJsonObject &preset);
    //Skip to the next Pomodoro
    void skip_pomodoro();
signals:
    //Emitted when the current segment changes. The int signals the context of the change.
    //0 = Session started, 1 = study -> short break, 2 = study -> long break,
    //3 = x break -> study, 4 = session over, 5 = session restart.
    void segment_changed(int);
    void timer_toggled(bool); //Emitted when the timer is paused or resumed, with a resumed timer having true passed as an argument.
private:
    //Wrapper to hold the timer pointer and its state.
    struct timerWrapper{
        QTimer* timer;
        //Integer used to hold remaining time when the timer is paused.
        int rem;
    };
    timerWrapper* timerInfo;
    //Object that represents the current settings of the timer.
    QJsonObject* currentPreset = NULL;    
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
    //Are we studying or on break? <DEPRECATED>
    bool studying;
    //Current segment
    int current_segment;
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
    //Used if loading without preset support.
    const QString DEFAULT_TITLES[6] = {
        QStringLiteral("Starting Study Session"),
        QStringLiteral("Study Segment Complete"),
        QStringLiteral("Study Cycle Complete"),
        QStringLiteral("Short Break Complete"),
        QStringLiteral("Study Session Complete"),
        QStringLiteral("Restarting Study Session")
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
        QStringLiteral("Good luck!"),
        QStringLiteral("Nice job out there. You have completed <current_pomodoro> pomodoros.\nEnjoy your short break!"),
        QStringLiteral("Congratulations! You have completed <current_pomodoro> pomodoros, and have earned your self a long break!"),
        QStringLiteral("Hope you enjoyed the break! Now, GET BACK TO WORK!"),
        QStringLiteral("Congratulations! Hope you got a lot done!"),
        QStringLiteral("Time to get some more work done!")
    };
    //Current Notification message.
    QString* titles;
    QString* messages;
    //Icons in use (by path)
    QStringList icons;
    //Methods
    QString constructOutput(QString template_string) const;
    void apply_preset_values(const QJsonObject &preset);
private slots:
    //This function checks where we are in a cycle and resets the timer appropriatly.
    void change_segment();
    //NOTE: start() automatically stops a running timer.
    void resetSegment(); //Reset the current segment's timer.
    //void resetSession(); //Reset the session
};

#endif // POMODORO_TIMER_H
