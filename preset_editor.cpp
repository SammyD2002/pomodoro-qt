#include "timerconfig.h"

/*PresetEditor::PresetEditor(PomodoroTimer* timer, PresetManager* current_presets, QWidget* parent) : TimerConfig(1, parent){
    this->parentTimer = timer;
    this->setPlaceholders(timer);
    this->preset_name_label = new QLabel("Preset Name");
    this->preset_name_title = new QLineEdit(this);
    this->layout->addWidget(preset_name_label, 0, 0);
    this->layout->addWidget(preset_name_title, 0, 1);
    this->preset_name_label->setText("Preset Name");
    //Connect UI Actions
}*/
PresetEditor::PresetEditor(PomodoroTimer* timer, QString preset_name, PresetManager* current_presets, QWidget* parent) : TimerConfig(1, parent){
    this->parentTimer = timer;
    this->preset_manager = current_presets;
    int index = current_presets->findPreset(preset_name);
    if (index >= 0)
        this->original_preset = this->preset_manager->getPreset(index);
    else
        this->original_preset = NULL;
    this->preset_name_label = new QLabel("Preset Name");
    this->preset_name_title = new QLineEdit(this);
    this->s_edit = new PresetSegmentEditor(this);
    this->message_edit = new PresetNotificationMessageEditor(this);
    this->title_edit = new PresetNotificationTitleEditor(this);
    this->setupTabBar();
    this->setupButtons("Save and Apply Preset", "Save Preset");
    connect(this->conf_apply, &QPushButton::clicked, this, &PresetEditor::submit);
    connect(this->conf_reset, &QPushButton::clicked, this, &PresetEditor::submit_and_restart);
    if(index >= 0){
        this->s_edit->setPlaceholders(this->original_preset);
        this->message_edit->setPlaceholders(this->original_preset);
        this->title_edit->setPlaceholders(this->original_preset);
    }
    else{
        this->s_edit->setPlaceholders(timer);
        this->message_edit->setPlaceholders(timer);
        this->title_edit->setPlaceholders(timer);
    }
    this->preset_name_title->setText(preset_name);
    this->layout->addWidget(preset_name_label, 0, 0);
    this->layout->addWidget(preset_name_title, 0, 1);
}

PresetEditor::~PresetEditor(){
    delete this->original_preset;
    if(this->new_preset != NULL)
        delete this->new_preset;
}

void SegmentEditor::setPlaceholders(const QJsonObject *preset){
    //Set the bool here early.
    bool check = (*preset)["cycle_lim_enabled"].toBool();
    check = !check;
    std::cout << std::boolalpha << check << std::endl;
    this->studyUnit->setCurrentIndex(PresetManager::getPresetInt((*preset)["len_study"].toArray()[1]));
    this->study->setText(ms_to_unit(PresetManager::getSegmentLength((*preset)["len_study"]), this->studyUnit->currentIndex()));

    this->short_break_unit->setCurrentIndex(PresetManager::getPresetInt((*preset)["len_break_s"].toArray()[1]));
    this->short_break->setText(ms_to_unit(PresetManager::getSegmentLength((*preset)["len_break_s"]), this->short_break_unit->currentIndex()));

    this->long_break_unit->setCurrentIndex(PresetManager::getPresetInt((*preset)["len_break_l"].toArray()[1]));
    this->long_break->setText(ms_to_unit(PresetManager::getSegmentLength((*preset)["len_break_l"]), this->long_break_unit->currentIndex()));

    this->p_per_c->setText(QString::number(PresetManager::getPresetInt((*preset)["max_pomodoros"])));
    this->m_cycle->setText(QString::number(PresetManager::getPresetInt((*preset)["max_cycles"])));
    this->m_cycle_enabled->setChecked(check);
}

void NotificationEditor::setPlaceholders(const QJsonObject *preset){
    QString* titles = PresetManager::getPresetStringArray((*preset)["notification_titles"]);
    for(int i = 0; i < 6; i++)
        this->title_inputs[i]->setText(titles[i]);
}

void MessageEditor::setPlaceholders(const QJsonObject *preset){
    QString* titles = PresetManager::getPresetStringArray((*preset)["notification_messages"]);
    for (int i = 0; i < 6; i++)
        this->title_inputs[i]->setText(titles[i]);
}
//Slots
bool PresetEditor::apply_changes(double new_vals[5], QString (&new_titles)[6], QString(&new_messages)[6]){
    //Create a new iteration of the qjsonobject.
    bool check = this->s_edit->checkCycleLimit();
    check = !check;
    std::cout << std::boolalpha << check << std::endl;
    QString preset;
    QString original_name = (*(this->original_preset))["preset_name"].toString();
    if(this->preset_name_title->text().trimmed().isEmpty())
        preset = original_name;
    else
        preset = this->preset_name_title->text().trimmed();
    std::cout << "Applying edits to preset " << qPrintable(preset) << std::endl;
    /* Now obselete, check performed in update function.
    int preset_index = this->preset_manager->findPreset(preset);
    if (preset_index >= 0 && preset != (*(this->original_preset))["preset_name"].toString())
        return false;
    */
    int * units = new int[3];
    this->s_edit->get_units(units);
    QJsonArray new_title_list;
    QJsonArray new_message_list;
    for (int i = 0; i < 6; i++){
        new_title_list.append(new_titles[i]);
        new_message_list.append(new_messages[i]);
    }
    QJsonObject new_preset = {
        {"preset_name", preset},
        {"len_study", QJsonArray({new_vals[0], units[0]})},
        {"len_break_l", QJsonArray({new_vals[1], units[1]})},
        {"len_break_s", QJsonArray({new_vals[2], units[2]})},
        {"max_pomodoros", new_vals[3]},
        {"cycle_lim_enabled", check},
        {"max_cycles", new_vals[4]},
        {"notification_messages", new_message_list},
        {"notification_titles", new_title_list}
    };
    //Update the timer preset
     //preset == enables overwriting a preset with the same name.
    bool ok = this->preset_manager->update_preset(new_preset, preset == original_name);
    if(!ok){
        emit this->request_overwrite("Preset " + preset + " exists.", "Overwrite it?", ok);
        ok = this->preset_manager->update_preset(new_preset, ok);
    }
    if(ok){
        this->new_preset = new QJsonObject(new_preset); //Makes a copy of the newly updated preset.
        this->close();
    }
    return ok;
}

void PresetEditor::submit_and_restart(){
    if (this->submit())
        this->parentTimer->applyPreset(this->new_preset);
    //Restart the prog
}
