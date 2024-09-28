/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#include "pomodoro_timer.h"
//Public Member Functions:
PomodoroTimer::PomodoroTimer(double study, double break_s, double break_l, int units[], int m_cycles, int m_pomodoros, bool c_lim, QWidget* parent) : QWidget(parent){
    this->timerInfo = new PomodoroTimer::timerWrapper;
    this->timerInfo->timer = new QTimer(this);
    this->timerInfo->rem = -1;
    this->len_study = TimerConfig::convert_time(study, units[0]); //Times sent as s, this adjusts them to ms.
    this->len_break_s = TimerConfig::convert_time(break_s, units[1]);
    this->len_break_l = TimerConfig::convert_time(break_l, units[2]);
    this->m_cycles = m_cycles;
    this->c_cycle = 0;
    this->c_limit_enabled = c_lim;
    this->m_pomodoros = m_pomodoros;
    this->c_pomodoros = 0; //Should start the timer upon determining that we are about to do the first segment.
    //initialize the array of titles
    for(int i = 0; i < 6; i++){
        this->titles[i] = QString("");
        this->messages[i] = QString("");
    }
    this->icons << "" << "" << "" << "";
    connect(this->timerInfo->timer, &QTimer::timeout, this, &PomodoroTimer::change_segment);
    this->constructSettingsJson(units);
}

PomodoroTimer::PomodoroTimer(const QJsonObject* preset, QWidget* parent) : QWidget(parent){
    //Inititialize Icon List to empty strings
    this->icons << "" << "" << "" << "";
    this->timerInfo = new PomodoroTimer::timerWrapper;
    this->timerInfo->timer = new QTimer(this);
    this->applyPreset(*preset);
    connect(this->timerInfo->timer, &QTimer::timeout, this, &PomodoroTimer::change_segment);
}

void PomodoroTimer::apply_preset_values(const QJsonObject &preset){
        std::string preset_name;
        try{
            preset_name = PresetManager::getJsonVal<QString>(preset.value("preset_name")).toStdString();
        }
        catch (PresetManager::json_value_error &ex){
            throw PresetManager::preset_error();
        }
        try{
            //Assign the study length.
            this->len_study = PresetManager::getSegmentLength(preset["len_study"]);
        }
        catch (PresetManager::json_value_error &ex){
            throw PresetManager::preset_error(preset_name, "len_study", ex);
        }
        try{
            //Assign the short break length.
            this->len_break_s = PresetManager::getSegmentLength(preset["len_break_s"]);
        }
        catch (PresetManager::json_value_error &ex){
            throw PresetManager::preset_error(preset_name, "len_break_s", ex);
        }
        //Assign the long break length.
        try{
            this->len_break_l = PresetManager::getSegmentLength(preset["len_break_l"]);
        }
        catch (PresetManager::json_value_error &ex){
            throw PresetManager::preset_error(preset_name, "len_break_l", ex);
        }
        this->c_cycle = 1;
        try{
            this->m_cycles = PresetManager::getJsonVal<int>(preset["max_cycles"]);
        }
        catch (PresetManager::json_value_error &ex){
            throw PresetManager::preset_error(preset_name, "max_cycles", ex);
        }
        this->c_pomodoros = 1;
        try{
            this->m_pomodoros = PresetManager::getJsonVal<int>(preset["max_pomodoros"]);
        }
        catch (PresetManager::json_value_error &ex){
            throw PresetManager::preset_error(preset_name, "max_pomodoros", ex);
        }
        try{
            this->titles = PresetManager::getPresetStringArray(preset["notification_titles"]);
        }
        catch (PresetManager::json_value_error &ex){
            throw PresetManager::preset_error(preset_name, "notification_titles", ex);
        }
        try{
            this->messages = PresetManager::getPresetStringArray(preset["notification_messages"]);
        }
        catch (PresetManager::json_value_error &ex){
            throw PresetManager::preset_error(preset_name, "notification_messages", ex);
        }
        try{
            this->c_limit_enabled = PresetManager::getJsonVal<bool>(preset["cycle_lim_enabled"]);
        }
        catch (PresetManager::json_value_error &ex){
            throw PresetManager::preset_error(preset_name, "cycle_lim_enabled", ex);
        }
        try{
            QJsonArray paths = PresetManager::getJsonVal<QJsonArray>(preset.value("icon_paths"));
            for(int i = 0; i < 4; i++)
                this->icons[i] = ((paths.at(i).isUndefined()) ? "" : paths.at(i).toString());
        }
        catch (PresetManager::json_value_error &ex){
            qWarning("'icon_paths' field is missing, using default icons...");
            this->icons << "" << "" << "" << "";
        }
    }
bool PomodoroTimer::applyPreset(const QJsonObject &preset){
    try{
        this->apply_preset_values(preset);
        //If we made it here, the json_value_error exception was NOT thrown. proceed with updateing the preset pointer.
        if(this->currentPreset != nullptr)
            delete this->currentPreset;
        //(Hopefully) Create a copy of the Preset.
        this->currentPreset = new QJsonObject(preset);
        return true;
    }
    catch (PresetManager::preset_error &err){
        this->apply_preset_values(*this->currentPreset);
        throw PresetManager::preset_error(err);
    }
}

PomodoroTimer::~PomodoroTimer(){
    delete (this->timerInfo);
    delete (this->currentPreset);
}

void PomodoroTimer::constructSettingsJson(int units[3], QString* name){
    if (!name)
        name = new QString("FALLBACK");
    QJsonArray titles;
    QJsonArray messages;
    QJsonArray icons;
    for(int i = 0; i < 6; i++){
        titles.append(this->titles[i]);
        messages.append(this->messages[i]);
        if(!this->icons.isEmpty() && i < 4)
            icons.append(this->icons[i]);
    }
    this->currentPreset = new QJsonObject({
        {"preset_name", *name},
        {"len_study", QJsonArray({(static_cast<double>(this->len_study) / TimerConfig::UNIT_MULT[units[0]]), units[0]})}, //length, unit.
        {"len_break_s", QJsonArray({(static_cast<double>(this->len_break_s) / TimerConfig::UNIT_MULT[units[1]]), units[1]})},
        {"len_break_l", QJsonArray({(static_cast<double>(this->len_break_l) / TimerConfig::UNIT_MULT[units[2]]), units[2]})},
        {"cycle_lim_enabled", this->c_limit_enabled},
        {"max_pomodoros", this->m_pomodoros},
        {"max_cycles", this->m_cycles},
        {"notification_titles", titles},
        {"notification_messages", messages},
    });
    if(this->icons.length() == 4)
        this->currentPreset->insert("icon_paths", icons);

    delete name; //Prevents memory leaks!
}

QJsonObject PomodoroTimer::getPresetJson() const{
    return QJsonObject(*(this->currentPreset));
}
//Copies the name of the running timer to a newly allocated string.
QString* PomodoroTimer::getRunningPresetName() const{
    if (this->currentPreset){ //If the current preset is defined:
        try{
            return new QString(PresetManager::getJsonVal<QString>(this->currentPreset->value("preset_name")));
        }
        catch (PresetManager::json_value_error const&){
            return new QString("UNDEFINED");
        }
    }
    return nullptr;
}

void PomodoroTimer::initCycle(bool reset){
    this->timerInfo->timer->start(this->len_study);
    this->studying = true;
    this->c_pomodoros = this->c_cycle = 1;
    this->current_segment = 0;
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
//Called when the main timer elapses.
void PomodoroTimer::change_segment(){
    //Conditions met if study timer runs out.
    if (this->studying){
        qInfo("Done studying...");
        this->studying = false;
        if(this->c_pomodoros < this->m_pomodoros){
            qInfo("Starting Short Break...");
            this->timerInfo->timer->start(this->len_break_s);
            this->current_segment = 1;
            emit this->segment_changed(1);
        }
        else{
            qInfo("Starting Long Break...");
            this->timerInfo->timer->start(this->len_break_l);
            this->current_segment = 2;
            emit this->segment_changed(2);
        }
    }
    else{
        this->current_segment = 0;
        qInfo("Back studying...");
        this->studying = true;
        if(this->c_pomodoros < this->m_pomodoros){
            qInfo("Incrementing pomodoro count...");
            this->timerInfo->timer->start(this->len_study);
            this->c_pomodoros++;
            emit this->segment_changed(3);
        }
        else if (this->c_limit_enabled && this->c_cycle >= this->m_cycles){ //If this is true, stop and send message.
            qInfo("Cycle limit reached. Stopping...");
            this->current_segment = 3;
            this->c_cycle = 0;
            this->timerInfo->timer->stop();
            emit this->segment_changed(4);
        }
        else{
            qInfo("End of cycle reached. Resetting pomodoro count...");
            this->timerInfo->timer->start(this->len_study);
            this->c_pomodoros = 1;
            this->c_cycle++;
            emit this->segment_changed(3);
        }
    }
}
//NOTE: start() automatically stops a running timer. Also, 1000 ms per s.
void PomodoroTimer::resetSegment(){
    bool prev_active = this->timerInfo->timer->isActive();
    if (this->studying){
        this->timerInfo->timer->start(this->len_study);
    } else if(this->c_pomodoros < this->m_pomodoros){
        this->timerInfo->timer->start(this->len_break_s);
    } else{
        this->timerInfo->timer->start(this->len_break_l);
    }
    if (!prev_active) //Re-enable loop_timer and switch back labels
        emit this->timer_toggled(true);
}

void PomodoroTimer::skip_pomodoro(){
    if(this->current_segment != 3){
        this->timerInfo->timer->stop();
        this->studying = false;
        this->current_segment = (this->c_pomodoros < this->m_pomodoros) ? 1 : 2;
        this->change_segment();
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

void PomodoroTimer::ResetSession(){
    this->initCycle(false);
}

//Adjust segment length specified by int segment to int new_time in seconds.
void PomodoroTimer::adjustSegment(int segment, int new_time){
    switch(segment){
    case 1:
        //std::cout << "UPDATING STUDY..." << std::endl;
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
void PomodoroTimer::update_icons(const QStringList &icons){
    if(icons.length() == 4){
        for(int i = 0; i < 4; i++)
            this->icons[i] = icons[i];
    }
}
//false=0, true=1
//Method to get the program's status. 1 = studying, 2 = short break, 3 = long break. Integer is negative if timer is paused.
int PomodoroTimer::getStatus() const{
    if(this->studying)
        return 1; //* (1 - (2 * int(this->timerInfo->timer->isActive()))); //Evaluates to -1 if timer is paused.
    else
        return (2 + int(this->c_pomodoros == this->m_pomodoros)); //* (1 - (2 * int(this->timerInfo->timer->isActive())));
}

/* Strings to replace:
     * Name of x
     *  <preset>: The name of the most recently loaded preset. (Loading preset a, then editing the timer will still print a.)
     * Number of x
     *  <current_pomodoro>: The current pomodoro
     *  <pomodoros_per_cycle>: The number of pomodoros in each cycle.
     *  <current_cycle>: The current cycle
     *  <cycles_per_session>: The number of cycles in each session. Returns ∞ if no cycle limit set.
     * Length of x
     *  <len_study[/s,/m,/h]>: The length of the study session in seconds/minutes/hours. Default: Minutes
     *  <len_break_s>
     *  <len_break_l>
     */
QString PomodoroTimer::constructOutput(QString template_string) const{
    QString return_string(template_string); //Setup the copy of the template string.
    if(template_string.contains("<task_name>")){
        QJsonObject c_task(todo_list::get_todo_list()->get_task_info(0));
        if(!c_task.isEmpty()){
            return_string = return_string.replace("<task_name>", c_task.value("title").toString(""));
        }
    }
    if(template_string.contains("<task_desc>")){
        QJsonObject c_task(todo_list::get_todo_list()->get_task_info(0));
        if(!c_task.isEmpty()){
            return_string = return_string.replace("<task_desc>", c_task.value("description").toString(""));
        }
    }
    if(template_string.contains("<task_pomodoros>")){
        QJsonObject c_task(todo_list::get_todo_list()->get_task_info(0));
        if(!c_task.isEmpty()){
            int n = c_task.value("description").toInt(-2);
            if(n < 0)
                return_string = return_string.replace("<task_pomodoros>", "∞");
            else
                return_string = return_string.replace("<task_pomodoros>", QString::number(n));
        }
    }

    //First replace the preset and task names to (in theory) make use of later replacements.


    return_string = return_string.replace("<preset>",
        PresetManager::getJsonVal<QString>((*this->currentPreset)["preset_name"]), Qt::CaseInsensitive);
    //Replace all occurences of current & maximum pomodoros/cycles.
    return_string = return_string.replace("<current_pomodoro>", QString(this->get_c_pom_str()), Qt::CaseInsensitive);
    return_string = return_string.replace("<pomodoros_per_cycle>", QString(this->get_m_pom_str()), Qt::CaseInsensitive);
    return_string = return_string.replace("<current_cycle>", QString(this->get_c_cycle_str()), Qt::CaseInsensitive);
    if(this->is_cycle_lim_enabled())
        return_string = return_string.replace("<cycles_per_session>", QString(this->get_m_cycle_str()), Qt::CaseInsensitive);
    else
        return_string = return_string.replace("<cycles_per_session>", QString("∞"), Qt::CaseInsensitive);
    //Set vars to be used in replacement.
    double study = static_cast<double>(this->get_len_study_int());
    double break_s = static_cast<double>(this->get_len_break_s_int());
    double break_l = static_cast<double>(this->get_len_break_l_int());
    //Segment Lengths
    return_string = return_string.replace("<len_study/s>", QString::number(study / 1000.0), Qt::CaseInsensitive);
    return_string = return_string.replace("<len_study/m>", QString::number(study / 60000.0), Qt::CaseInsensitive);
    return_string = return_string.replace("<len_study/h>", QString::number(study / 360000.0), Qt::CaseInsensitive);
    return_string = return_string.replace("<len_study>", QString::number(study / 60000.0), Qt::CaseInsensitive);

    return_string = return_string.replace("<len_break_s/s>", QString::number(break_s / 1000.0), Qt::CaseInsensitive);
    return_string = return_string.replace("<len_break_s/m>", QString::number(break_s / 60000.0), Qt::CaseInsensitive);
    return_string = return_string.replace("<len_break_s/h>", QString::number(break_s / 360000.0), Qt::CaseInsensitive);
    return_string = return_string.replace("<len_break_s>", QString::number(break_s / 60000.0), Qt::CaseInsensitive);

    return_string = return_string.replace("<len_break_l/s>", QString::number(break_l / 1000.0), Qt::CaseInsensitive);
    return_string = return_string.replace("<len_break_l/m>", QString::number(break_l / 60000.0), Qt::CaseInsensitive);
    return_string = return_string.replace("<len_break_l/h>", QString::number(break_l / 360000.0), Qt::CaseInsensitive);
    return_string = return_string.replace("<len_break_l>", QString::number(break_l / 60000.0), Qt::CaseInsensitive);
    //Return the newly constructed string.
    return return_string;
}

//Get a specific message based on the int passed as an arg.
QString PomodoroTimer::getMessageTitle(int status) const{
    if (status <= 5 && status >= 0){
        if (this->titles[status].isEmpty())
            return this->constructOutput(this->DEFAULT_TITLES[status]);
        return this->constructOutput(this->titles[status]);
    }
    return NULL;
}

//Get a specific message based on the int passed as an arg.
QString PomodoroTimer::getMessageTitleTemplate(int status) const{
    if (status <= 5 && status >= 0){
        if (this->titles[status].isEmpty())
            return this->DEFAULT_TITLES[status];
        return this->titles[status];
    }
    return NULL;
}

QString PomodoroTimer::getMessageBody(int status) const{
    if(status <= 5 && status >= 0){
        if(this->messages[status].isEmpty())
            return this->constructOutput(this->DEFAULT_MESSAGES[status]);
        else
            return this->constructOutput(this->messages[status]);
    }
    else
        return QString("Status out of index?");
}

QString PomodoroTimer::getMessageBodyTemplate(int status) const{
    if(status <= 5 && status >= 0){
        if(this->messages[status].isEmpty())
            return this->DEFAULT_MESSAGES[status];
        else
            return this->messages[status];
    }
    else
        return QString("Status out of index?");
}

QString PomodoroTimer::get_icon_name(int status) const{
    QString icon = (this->icons.length() > status) ? this->icons[status] : "";
    //qInfo("%s", (std::stringstream() << "Icon title: " << qPrintable(icon)).str().c_str());
    return icon;
}


//Update the contents of messages.
void PomodoroTimer::setMessageTitles(bool updated[6], QString new_messages[6]){
    for(int i = 0; i < 6; i++){
        if(updated[i] && !(new_messages[i].isEmpty()))
            this->titles[i] = new_messages[i];
    }
}

void PomodoroTimer::setMessageBodies(bool updated[6], QString new_messages[6]){
    for(int i = 0; i < 6; i++){
        if(updated[i] && !(new_messages[i].isEmpty()))
            this->messages[i] = new_messages[i];
    }
}

//Private Member Functions
//Method to start the timer.
void PomodoroTimer::resumeTimer(){
    this->timerInfo->timer->start(this->timerInfo->rem);
    emit this->timer_toggled(true);
}

//Method to stop the timer. SHOULD ONLY BE CALLED BY toggleTimer().
void PomodoroTimer::pauseTimer(){
    if(this->timerInfo->timer->isActive())
        this->timerInfo->rem = this->timerInfo->timer->remainingTime();
    else
        this->timerInfo->rem = this->timerInfo->timer->interval();
    this->timerInfo->timer->stop();
    emit this->timer_toggled(false);
}

void PomodoroTimer::ResetSegment(){
    this->resetSegment();
}
