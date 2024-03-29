#include "timerconfig.h"

TimerConfig::TimerConfig(PomodoroTimer* parentTimer){
    //Need to add labels and unit selection boxes.
    this->setModal(true); //Blocks input to other windows while active.
    this->parentTimer = parentTimer;
    this->layout = new  QGridLayout(this);

    //Study Editor
    this->studyLabel = new QLabel(tr("Study Segment Length:"));
    layout->addWidget(this->studyLabel, 0, 0);
    this->study = new QLineEdit(this);
    layout->addWidget(this->study, 0, 1);
    this->studyUnit = this->setupUnitBox(new QComboBox(this));
    layout->addWidget(this->studyUnit, 0, 2);

    //Short Break Editor
    this->short_break_label = new QLabel(tr("Short Break Length:"));
    layout->addWidget(this->short_break_label, 1, 0);
    this->short_break = new QLineEdit(this);
    layout->addWidget(this->short_break, 1, 1);
    this->short_break_unit = this->setupUnitBox(new QComboBox(this));
    layout->addWidget(this->short_break_unit, 1, 2);

    //Long Break Editor
    this->long_break_label = new QLabel(tr("Long Break Length"));
    layout->addWidget(this->long_break_label, 2, 0);
    this->long_break = new QLineEdit(this);
    layout->addWidget(this->long_break, 2, 1);
    this->long_break_unit = this->setupUnitBox(new QComboBox(this));
    layout->addWidget(this->long_break_unit, 2, 2);

    //Pomodoros per Cycle Editor
    this->p_per_c_label = new QLabel(tr("Pomodoros per Cycle:"));
    layout->addWidget(this->p_per_c_label, 0, 3);
    this->p_per_c = new QLineEdit(this);
    layout->addWidget(this->p_per_c, 0, 4);

    //Totals Session Cycles Editor
    this->m_cycle_label = new QLabel(tr("Cycles per Session"));
    layout->addWidget(this->m_cycle_label, 1, 3);
    this->m_cycle = new QLineEdit(this);
    layout->addWidget(this->m_cycle, 1, 4);
    this->m_cycle_enabled_label = new QLabel(tr("Don't Limit Cycles"));
    layout->addWidget(this->m_cycle_enabled_label, 2, 3);
    this->m_cycle_enabled = new QRadioButton();
    layout->addWidget(this->m_cycle_enabled, 2, 4);
    //Connect the radio button to the thing that disables/enables the limit box. Should send isChecked().
    connect(m_cycle_enabled, &QRadioButton::toggled, this->m_cycle, &QLineEdit::setDisabled);
    //TODO: Add Cycle Limit Remover

    //Push Buttons
    this->conf_reset = new QPushButton("Confirm and Reset", this);
    layout->addWidget(conf_reset, 0, 5);
    this->conf_apply = new QPushButton("Confirm");
    layout->addWidget(conf_apply, 1, 5);
    this->abort = new QPushButton("Cancel");
    layout->addWidget(this->abort, 2, 5);
    connect(conf_apply, &QPushButton::clicked, this, &TimerConfig::submit);
    connect(conf_reset, &QPushButton::clicked, this, &TimerConfig::submit_and_restart);
    connect(abort, &QPushButton::clicked, this, &TimerConfig::close);
    //this->show();
}

//Timer close override
void TimerConfig::closeEvent(QCloseEvent *event){
    this->parentTimer->resumeTimer();
    event->accept();
}

QString TimerConfig::ms_to_min(int ms){
    float min = static_cast<float>(ms) / 1000; //ms->s
    min /= 60; //s->min
    return QString::fromStdString(std::to_string(min));
}

void TimerConfig::setPlaceholders(){
    this->study->setText(this->ms_to_min(this->parentTimer->get_len_study_int()));
    this->short_break->setText(this->ms_to_min(this->parentTimer->get_len_break_s_int()));
    this->long_break->setText(this->ms_to_min(this->parentTimer->get_len_break_l_int()));
    this->p_per_c->setText(QString::fromStdString(this->parentTimer->get_m_pom_str()));
    this->m_cycle->setText(QString::fromStdString(this->parentTimer->get_m_cycle_str()));
    this->exec();
}

//Determine if input is a time in the correct format.
/* @DOC
 * Assumptions made for simplicity:
 *  - Either all times may contain leading 0's, or none of them do.
 * Supported Time Formats
 * Unit set to any val:
 * h:m:s
 * h:m:s.z
 *
 * Unit set to hour
 * h:m
 * h:m.%m (TODO)
 * Unit set to minute
 * m:s
 * m:s.z
 */
QTime TimerConfig::input_is_formatted_time(QString in, int mode){ //Determine # of : to intelligently decide what type of time is being typed.
    QTime time = QTime::fromString(in, "h:m:s");
    if (time.isValid())
        return time;
    time = QTime::fromString(in, "h:m:s.z");
    if (time.isValid())
        return time;
    switch (mode){
    case 0: //Unit = s
    case 1: //Unit = m
        time = QTime::fromString(in, "m:s");
        if (time.isValid())
            return time;
        time = QTime::fromString(in, "m:s.z");
        if (time.isValid())
            return time;
        break;
    case 2:
        time = QTime::fromString(in, "h:m");
        if (time.isValid())
            return time;
        break;
    }
    return time; //time returned here will be invalid and must be checked.
}


//Slots
void TimerConfig::submit(){
    //Make sure all data is good:
    bool all_good = true;
    int new_vals[5] ={
        this->time_valid(this->study->text(), this->studyUnit->currentIndex()),
        this->time_valid(this->short_break->text(), this->short_break_unit->currentIndex()),
        this->time_valid(this->long_break->text(), this->long_break_unit->currentIndex()),
        this->cycles_valid(this->p_per_c->text()),
        this->cycles_valid(this->m_cycle->text())};
    for(int i = 0; i < 5 - static_cast<int>(this->m_cycle_enabled->isChecked()); i++){
        if(new_vals[i] < 0){
            //do err for element at i.
            all_good = false;
        }
    }

    if (all_good){
        this->apply_changes();
        this->close();
    }
}

virtual void TimerConfig::apply_changes(int new_vals[5]){
    if (!(new_vals[0] == 0 || new_vals[0] == this->parentTimer->get_len_study_int()))
        emit this->study_updated(new_vals[0]);
    if (!(new_vals[1] == 0 || new_vals[1] == this->parentTimer->get_len_break_s_int()))
        emit this->break_s_updated(new_vals[1]);
    if (!(new_vals[2] == 0 || new_vals[2] == this->parentTimer->get_len_break_l_int()))
        emit this->break_l_updated(new_vals[2]);
    if (!(new_vals[3] == 0 || new_vals[3] == this->parentTimer->get_m_pom_int()))
        emit this->m_pomodoros_updated(new_vals[3]);
    if (!(m_cycle_enabled->isChecked() != this->parentTimer->is_cycle_lim_enabled())) //If lim is enabled, box is unchecked.
        emit this->cycle_limit_toggled(this->m_cycle_enabled->isChecked());
    if (!(m_cycle_enabled->isChecked()) && !(new_vals[4] == 0 || new_vals[4] == this->parentTimer->get_m_cycle_int()))
        emit this->m_cycles_updated(new_vals[4]);
}

void TimerConfig::submit_and_restart(){
    this->submit();
    this->parentTimer->initCycle(false);
    //Restart the prog
}


QComboBox* TimerConfig::setupUnitBox(QComboBox* units){
    units->addItem(tr("Seconds")); //Index = 0
    units->addItem(tr("Minutes")); //Index = 1
    units->addItem(tr("Hours")); //Index = 2
    units->setCurrentIndex(1);
    return units;
}

//TODO: Update this to detect times as hh:mm:ss & return a different integer. Only do this if dec is true.
//NOTE: Other string chars only allowed if also in format.
//0 is false, any other int is true.
bool TimerConfig::input_is_int(QString in, bool dec){
    if (in.isEmpty())
        return 0;
    bool is_int = true;
    for(QString::const_iterator c = in.cbegin(); c != in.cend(); c++){
        if(!(c->isDigit() || (dec && *(c) == QChar('.')))) //If c isn't a number or period if dec enabled.
            return false;
    }
    return true;
}
    //TODO: Implement hh:mm:ss time format
    //-1 = invalid input, -2 = input out of bounds, 0 = empty input, others
int TimerConfig::time_valid(const QString in, int unit){
    //First, attempt to derive time as n s/m/h
    float new_time_f;
    if (in.isEmpty())
        return 0;
    if (!this->input_is_int(in, true)){
        //Since this failed, attempt to get time as hh:mm:ss.ms/mm:ss.ms
        QTime input_time = this->input_is_formatted_time(in, unit);
        if (input_time.isValid())
            return this->parentTimer->ZERO_TIME->msecsTo(input_time); //Get time from 0:0:0.0 to the inputted time.
        return -1;
    }
    else
        new_time_f = in.toFloat();
    int new_time = 0; //If this is not updated, something has broken.
    switch(unit){ //Convert nums to smaller value until ms is reached.
    case 2: //hours
        new_time_f *= 60; //Hours to Minutes = m = 1/60 hours -> 60m = 1h
    case 1: //min
        new_time_f *= 60; //Minutes to Seconds: sec = 1/60 minutes -> 60s = 1m
    case 0: //Seconds
        new_time = new_time_f * 1000; //Seconds to ms: ms = 1/1000 seconds -> 1000ms = 1s
        break;
    };
    if(new_time <= 0)
        return -1;
    else
        return new_time;
}

int TimerConfig::cycles_valid(QString in){
    if (!input_is_int(in, false))
        return 0;
    int new_count = in.toInt();
    if (new_count <= 0)
        return -1;
    return new_count;
}
