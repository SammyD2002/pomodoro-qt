#include "pomodoro_timer.h"
//Public Member Functions:
PomodoroTimer::PomodoroTimer(QTimer * timer, int study, int break_s, int break_l, int m_cycles, int m_pomodoros, bool log_stdout, bool c_lim){
    this->log_stdout = log_stdout;
    this->timerInfo = new PomodoroTimer::timerWrapper;
    this->timerInfo->timer = timer;
    this->timerInfo->rem = -1;
    this->len_study = study * 1000; //Times sent as s, this adjusts them to ms.
    this->len_break_s = break_s * 1000;
    this->len_break_l = break_l * 1000;
    this->m_cycles = m_cycles;
    this->c_cycle = 0;
    this->c_limit_enabled = c_lim;
    this->m_pomodoros = m_pomodoros;
    this->c_pomodoros = 0; //Should start the timer upon determining that we are about to do the first segment.
    connect(this->timerInfo->timer, &QTimer::timeout, this, &PomodoroTimer::change_segment);
}

PomodoroTimer::~PomodoroTimer(){
    delete (this->timerInfo);
}

void PomodoroTimer::initCycle(bool reset){
    this->timerInfo->timer->start(this->len_study);
    this->studying = true;
    this->c_pomodoros = this->c_cycle = 1;
    //Emit signal to print welcome message.
    //Resets session if resume button is pressed after end of session.
    if(reset){
        emit this->segment_changed(5);
    }
    //Do this at session init.
    else{
        emit this->segment_changed(0);
    }
}

void PomodoroTimer::change_segment(){
    //Conditions met if study timer runs out.
    if (this->studying){
        if (this->log_stdout)
            std::cout << "Done studying..." << std::endl;
        this->studying = false;
        if(this->c_pomodoros < this->m_pomodoros){
            if(this->log_stdout)
                std::cout << "Starting Short Break..." << std::endl;
            this->timerInfo->timer->start(this->len_break_s);
            emit this->segment_changed(1);
        }
        else{
            if(this->log_stdout)
                std::cout << "Starting Long Break..." << std::endl;
            this->timerInfo->timer->start(this->len_break_l);
            emit this->segment_changed(2);
        }
    }
    else{
        if (this->log_stdout)
            std::cout << "Back studying..." << std::endl;
        this->studying = true;
        if(this->c_pomodoros < this->m_pomodoros){
            if(this->log_stdout)
                std::cout << "Incrementing pomodoro count..." << std::endl;
            this->timerInfo->timer->start(this->len_study);
            this->c_pomodoros++;
            emit this->segment_changed(3);
        }
        else if (this->c_limit_enabled && this->c_cycle >= this->m_cycles){ //If this is true, stop and send message.
            if(this->log_stdout)
                std::cout << "Cycle limit reached. Stopping..." << std::endl;
            this->c_cycle = 0;
            this->timerInfo->timer->stop();
            emit this->segment_changed(4);
        }
        else{
            if(this->log_stdout)
                std::cout << "End of cycle reached. Resetting pomodoro count..." << std::endl;
            this->timerInfo->timer->start(this->len_study);
            this->c_pomodoros = 1;
            this->c_cycle++;
            emit this->segment_changed(3);
        }
    }
}
//NOTE: start() automatically stops a running timer. Also, 1000 ms per s.
void PomodoroTimer::resetSegment(){
    if (this->studying){
        this->timerInfo->timer->start(this->len_study);
    } else if(this->c_pomodoros < this->m_pomodoros){
        this->timerInfo->timer->start(this->len_break_s);
    } else{
        this->timerInfo->timer->start(this->len_break_l);
    }
}
//Pauses the timer if it is running, resumes it if it is paused. If at end of session, toggling timer restarts the session.
void PomodoroTimer::toggleTimer(){
    if (this->timerInfo->timer->isActive())
        this->pauseTimer();
    else if(this->c_cycle == 0)
        this->initCycle(true);
    else
        this->resumeTimer();
}
//Adjust segment length specified by int segment to int new_time in seconds.
//To be implemented
void PomodoroTimer::adjustSegment(int segment, int new_time){
    switch(segment){
    case 1:
        std::cout << "UPDATING STUDY..." << std::endl;
        this->len_study = new_time;
        break;
    case 2:
        this->len_break_s = new_time;
        break;
    case 3:
        this->len_break_l = new_time;
        break;
    case 4:
        this->m_pomodoros = new_time;
        break;
    case 5:
        this->m_cycles = new_time;
        break;
    case 6:
        this->c_limit_enabled = static_cast<bool>(new_time);
    };
}
//false=0, true=1
//Method to get the program's status. 1 = studying, 2 = short break, 3 = long break. Integer is negative if timer is paused.
int PomodoroTimer::getStatus(){
    if(this->studying)
        return 1; //* (1 - (2 * int(this->timerInfo->timer->isActive()))); //Evaluates to -1 if timer is paused.
    else
        return (2 + int(this->c_pomodoros == this->m_pomodoros)); //* (1 - (2 * int(this->timerInfo->timer->isActive())));
}
//Private Member Functions
//Method to start the timer.
void PomodoroTimer::resumeTimer(){
    this->timerInfo->timer->start(this->timerInfo->rem);
    emit this->timer_toggled(true);
}
//Method to stop the timer. SHOULD ONLY BE CALLED BY toggleTimer().
void PomodoroTimer::pauseTimer(){
    this->timerInfo->rem = this->timerInfo->timer->remainingTime();
    this->timerInfo->timer->stop();
    emit this->timer_toggled(false);
}

void PomodoroTimer::ResetSegment(){
    this->resetSegment();
}
