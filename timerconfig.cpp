#include "timerconfig.h"
//TODO: Add buttons to edit message text (welcome, study done, s & l break done, cycle done, session done).
//REMINDER: Preset headers are commented out in .pro file.

//Multiplier for the index of a UNIT array used in multiple other places in the program.
const double TimerConfig::UNIT_MULT[3] = {1000, 60000, 3600000};

TimerConfig::TimerConfig(PomodoroTimer* parentTimer) : TimerConfig(0, parentTimer){
    //Tabs
    this->s_edit = new SegmentEditor(this);
    this->title_edit = new NotificationEditor(this);
    this->message_edit = new MessageEditor(this);
    this->setupTabBar();
    this->setupButtons();
    //Set the parentTimer variable.
    this->parentTimer = parentTimer;
    connect(this->conf_apply, &QPushButton::clicked, this, &TimerConfig::submit);
    connect(this->conf_reset, &QPushButton::clicked, this, &TimerConfig::submit_and_restart);
    //Connect UI Actions
    //connect(conf_apply, &QPushButton::clicked, this, &TimerConfig::submit);
    //connect(conf_reset, &QPushButton::clicked, this, &TimerConfig::submit_and_restart);
    //Connect post submit actions
    connect(this, &TimerConfig::segment_updated, parentTimer, &PomodoroTimer::adjustSegment);
    connect(this, &TimerConfig::titles_updated, parentTimer, &PomodoroTimer::setMessageTitles);
    connect(this, &TimerConfig::messages_updated, parentTimer, &PomodoroTimer::setMessageBodies);
}
TimerConfig::TimerConfig(int buf_upper, QWidget* parent) : QDialog(parent){
    this->layout = new QGridLayout(this);
    //Need to add labels and unit selection boxes.
    this->setWindowModality(Qt::WindowModal); //Blocks input to other windows while active.

    //Setup the tab widget:
    configTabBar = new QTabWidget(this);
    //Add the tab bar to the window layout
    this->layout->addWidget(this->configTabBar, buf_upper, 0, 1, 3);
    //TODO: Add Cycle Limit Remover

    //Push Buttons
    this->conf_reset = new QPushButton();
    layout->addWidget(conf_reset, 2, 0);
    this->conf_apply = new QPushButton();
    layout->addWidget(conf_apply, 2, 1);
    this->abort = new QPushButton();
    layout->addWidget(this->abort, 2, 2);
    connect(abort, &QPushButton::clicked, this, &TimerConfig::close);
    //this->show();
}

void TimerConfig::setupTabBar(){
    //Add tabs to the tab bar
    configTabBar->addTab(this->s_edit, "Segment Lengths"); //Keep the normal config here.
    configTabBar->addTab(this->title_edit, "Notification Titles"); //Add the message config here.
    configTabBar->addTab(this->message_edit, "Notification Messages");
}

void TimerConfig::setupButtons(QString conf_reset, QString conf_apply, QString abort){
    this->conf_reset->setText(conf_reset);
    this->conf_apply->setText(conf_apply);
    this->abort->setText(abort);
}


//Timer close override
void TimerConfig::closeEvent(QCloseEvent *event){
    emit this->config_complete();
    event->accept();
}

void TimerConfig::reject(){
    emit this->config_complete();
    QDialog::reject();
}

void TimerConfig::setup(){
    this->s_edit->setPlaceholders(this->parentTimer);
    this->title_edit->setPlaceholders(this->parentTimer);
    this->message_edit->setPlaceholders(this->parentTimer);
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
QDateTime TimerConfig::input_is_formatted_time(QString in, int mode){ //Determine # of : to intelligently decide what type of time is being typed.
    QDateTime time = QDateTime::fromString(in, "h:m:s");
    if (time.isValid())
        return time;
    time = QDateTime::fromString(in, "h:m:s.z");
    if (time.isValid())
        return time;
    switch (mode){
    case 0: //Unit = s
    case 1: //Unit = m
        time = QDateTime::fromString(in, "m:s");
        if (time.isValid())
            return time;
        time = QDateTime::fromString(in, "m:s.z");
        if (time.isValid())
            return time;
        break;
    case 2:
        time = QDateTime::fromString(in, "h:m");
        if (time.isValid())
            return time;
        break;
    }
    return time; //time returned here will be invalid and must be checked.
}


//Slots
bool TimerConfig::submit(){
    //Make sure all data is good:
    bool all_good = true;
    double new_vals[5];
    QString new_titles[6];
    QString new_messages[6];
    this->s_edit->getInputs(new_vals, this->s_edit->convert());
    this->title_edit->getTitleInputs(new_titles);
    this->message_edit->getTitleInputs(new_messages);
    for(int i = 0; i < 5 - static_cast<int>(this->s_edit->checkCycleLimit()); i++){
        if(new_vals[i] < 0){
            //do err for element at i.
            all_good = false;
        }
    }

    if (all_good){
        std::cout << "Attempting to apply changes..." << std::endl;
        if(this->apply_changes(new_vals, new_titles, new_messages)){
            this->close();
            return true;
        }
    }
    return false;
}

bool TimerConfig::apply_changes(double new_vals[5], QString (&new_titles)[6], QString (&new_messages)[6]){
    int ms_int[5];
    for (int i = 0; i < 5; i++)
        ms_int[i] = static_cast<int>(new_vals[i]);
    if (!(ms_int[0] == 0 || ms_int[0] == this->parentTimer->get_len_study_int()))
        emit this->segment_updated(1, ms_int[0]);
    if (!(ms_int[1] == 0 || ms_int[1] == this->parentTimer->get_len_break_s_int()))
        emit this->segment_updated(2, ms_int[1]);
    if (!(ms_int[2] == 0 || ms_int[2] == this->parentTimer->get_len_break_l_int()))
        emit this->segment_updated(3, ms_int[2]);
    if (!(ms_int[3] == 0 || ms_int[3] == this->parentTimer->get_m_pom_int()))
        emit this->segment_updated(4, ms_int[3]);
    if (!(this->s_edit->checkCycleLimit() != this->parentTimer->is_cycle_lim_enabled())) //If lim is enabled, box is unchecked.
        emit this->segment_updated(6, !(this->s_edit->checkCycleLimit()));
    if (!(this->s_edit->checkCycleLimit()) && !(new_vals[4] == 0 || new_vals[4] == this->parentTimer->get_m_cycle_int()))
        emit this->segment_updated(5, static_cast<int>(new_vals[4]));
    //Push updated titles.
    bool t_updated[6];
    for (int i = 0; i < 6; i++){
        if (new_titles[i] != this->parentTimer->getMessageTitle(i) && !new_titles[i].isEmpty())
            t_updated[i] = true;
        else
            t_updated[i] = false;
    }
    emit this->titles_updated(t_updated, new_titles);
    //Push updated messages.
    bool m_updated[6];
    for (int i = 0; i < 6; i++){
        if (new_messages[i] != this->parentTimer->getMessageBody(i) && !new_messages[i].isEmpty())
            m_updated[i] = true;
        else
            m_updated[i] = false;
    }
    emit this->messages_updated(m_updated, new_messages);
    int units[3];
    this->s_edit->get_units(units);
    this->parentTimer->constructSettingsJson(units);
    return true;
}

void TimerConfig::submit_and_restart(){
    if (this->submit())
        this->parentTimer->initCycle(false); //TODO: Replace this with an emitted signal that can be rerouted via the preset editor.
    //Restart the prog
}

//TODO: Update this to detect times as hh:mm:ss & return a different integer. Only do this if dec is true.
//NOTE: Other string chars only allowed if also in format.
//0 is false, any other int is true.
bool TimerConfig::input_is_int(QString in, bool dec){
    if (in.isEmpty())
        return 0;
    for(QString::const_iterator c = in.cbegin(); c != in.cend(); c++){
        if(!(c->isDigit() || (dec && *(c) == QChar('.')))) //If c isn't a number or period if dec enabled.
            return false;
    }
    return true;
}
    //TODO: Implement hh:mm:ss time format
    //-1 = invalid input, -2 = input out of bounds, 0 = empty input, others
double TimerConfig::time_valid(const QString in, int unit, bool convert){ //TODO: Prevent user from inputing more than 24 hours.
    double new_time_d = TimerConfig::validate_time_string(in, unit);
    if (convert)
        new_time_d = static_cast<double>(TimerConfig::convert_time(new_time_d, unit)); //If this is not updated, something has broken.
    return new_time_d;
}
int TimerConfig::convert_time(double time, int unit){
    int new_time = 0;
    switch(unit){ //Convert nums to smaller value until ms is reached.
    case 2: //hours
        time *= 60; //Hours to Minutes = m = 1/60 hours -> 60m = 1h
    case 1: //min
        time *= 60; //Minutes to Seconds: sec = 1/60 minutes -> 60s = 1m
    case 0: //Seconds
        new_time = time * 1000; //Seconds to ms: ms = 1/1000 seconds -> 1000ms = 1s
        break;
    };
    if(new_time <= 0)
        return -1;
    else
        return new_time;
}

double TimerConfig::validate_time_string(QString in, int unit){
    //First, attempt to derive time as n s/m/h
    if (in.isEmpty())
        return 0;
    if (!this->input_is_int(in, true)){
        //Since this failed, attempt to get time as hh:mm:ss.ms/mm:ss.ms
        QDateTime input_time = this->input_is_formatted_time(in, unit);
        if (input_time.isValid())
            return this->parentTimer->ZERO_TIME->msecsTo(input_time); //Get time from 0:0:0.0 to the inputted time.
        return -1;
    }
    else
        return in.toDouble();
}

int TimerConfig::cycles_valid(QString in){
    if (!input_is_int(in, false))
        return 0;
    int new_count = in.toInt();
    if (new_count <= 0)
        return -1;
    return new_count;
}
