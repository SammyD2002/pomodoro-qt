/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#include "preset_editor.h"
PresetEditor* PresetEditor::start_edit(PomodoroTimer* timer, QString preset_name, PresetManager* current_presets, QWidget* parent, bool update_default){
    PresetEditor* editor = nullptr;
    if (update_default){
        editor = new PresetEditor(timer, current_presets->getDefaultPreset(), current_presets, parent, true);
    }
    else{
        int index = current_presets->findPreset(preset_name);
        if (index >= 0)
            editor = new PresetEditor(timer, current_presets->getPreset(index), current_presets, parent, false);
        else
            throw PresetManager::preset_error(preset_name.toStdString() + " is not registered.");
    }
    return editor;
}

//Constructs the PresetEditor by calling the TimerConfig constructors before connecting the signals to the appropriate slots.
PresetEditor::PresetEditor(PomodoroTimer* timer, QJsonObject* preset, PresetManager* current_presets, QWidget *parent, bool update_default) : TimerConfig(1, parent){
    this->update_default = update_default;
    this->parentTimer = timer;
    this->preset_manager = current_presets;
    this->original_preset = preset;
    this->preset_name_label = new QLabel(QStringLiteral("Preset Name"));
    this->preset_name_title = new QLineEdit(this);
    this->s_edit = new PresetSegmentEditor(this);
    this->title_edit = new PresetNotificationEditor(this);
    this->setupTabBar();
    this->setupButtons(QStringLiteral("Save and Apply Preset"), QStringLiteral("Save Preset"));
    connect(this->conf_apply, &QPushButton::clicked, this, &PresetEditor::submit);
    connect(this->conf_reset, &QPushButton::clicked, this, &PresetEditor::submit_and_restart);
    if(this->original_preset != nullptr){
        this->s_edit->setPlaceholders(this->original_preset);
        this->title_edit->setPlaceholders(this->original_preset);
        QString original_name = this->original_preset->value(QStringLiteral("preset_name")).toString("");
        if(original_name.isEmpty())
            this->preset_name_title->setText("New Preset");
        else
        this->preset_name_title->setText(original_name);
    }
    else{
        this->s_edit->setPlaceholders(timer); //Used as fallback if preset is somehow outside of list.
        this->title_edit->setPlaceholders(timer);
        this->original_preset = new QJsonObject(timer->getPresetJson());
    }
    this->layout->addWidget(preset_name_label, 0, 0);
    this->layout->addWidget(preset_name_title, 0, 1);
}

//Make sure to delete the new preset objects.
PresetEditor::~PresetEditor(){
    delete this->original_preset;
    if(this->new_preset != nullptr)
        delete this->new_preset;
}

//Overloaded setPlaceholder methods for each tab to get the settings from a QJsonObject. These are NOT for the PresetEditor Subclasses.
void SegmentEditor::setPlaceholders(const QJsonObject *preset){
    //Get the name to check for early errors, and to use later for handling errors.
    std::string preset_name;
    try{
        preset_name = (PresetManager::getJsonVal<QString>(preset->value("preset_name"))).toStdString();
    }
    catch (PresetManager::json_value_error &ex){
        throw PresetManager::preset_error("?", ex);
    }
    try{
        this->m_cycle_enabled->setChecked(!PresetManager::getJsonVal<bool>(preset->value("cycle_lim_enabled")));
    }
    catch (PresetManager::json_value_error &ex){
        throw PresetManager::preset_error(preset_name, "cycle_lim_enabled", ex);
    }
    try{
        this->studyUnit->setCurrentIndex(PresetManager::getJsonVal<int>(preset->value("len_study").toArray()[1]));
    }
    catch (PresetManager::json_value_error &ex){
        throw PresetManager::preset_error(preset_name, "len_study[1]", ex);
    }
    try{
        this->study->setText(ms_to_unit(PresetManager::getSegmentLength((*preset)["len_study"]), this->studyUnit->currentIndex()));
    }
    catch (PresetManager::json_value_error &ex){
        throw PresetManager::preset_error(preset_name, "len_study[0]", ex);
    }
    try{
        this->short_break_unit->setCurrentIndex(PresetManager::getJsonVal<int>((*preset)["len_break_s"].toArray()[1]));
    }
    catch (PresetManager::json_value_error &ex){
        throw PresetManager::preset_error(preset_name, "len_break_s[1]", ex);
    }
    try{
        this->short_break->setText(ms_to_unit(PresetManager::getSegmentLength((*preset)["len_break_s"]), this->short_break_unit->currentIndex()));
    }
    catch (PresetManager::json_value_error &ex){
        throw PresetManager::preset_error(preset_name, "len_break_s[0]", ex);
    }
    try{
        this->long_break_unit->setCurrentIndex(PresetManager::getJsonVal<int>((*preset)["len_break_l"].toArray()[1]));
    }
    catch (PresetManager::json_value_error &ex){
        throw PresetManager::preset_error(preset_name, "len_break_l[1]", ex);
    }
    try{
        this->long_break->setText(ms_to_unit(PresetManager::getSegmentLength((*preset)["len_break_l"]), this->long_break_unit->currentIndex()));
    }
    catch (PresetManager::json_value_error &ex){
        throw PresetManager::preset_error(preset_name, "len_break_l[0]", ex);
    }
    try{
        this->p_per_c->setText(QString::number(PresetManager::getJsonVal<int>((*preset)["max_pomodoros"])));
    }
    catch (PresetManager::json_value_error &ex){
        throw PresetManager::preset_error(preset_name, "max_pomodoros", ex);
    }
    try{
        this->m_cycle->setText(QString::number(PresetManager::getJsonVal<int>((*preset)["max_cycles"])));
    }
    catch (PresetManager::json_value_error &ex){
        throw PresetManager::preset_error(preset_name, "max_cycles", ex);
    }
    //Its icon time!
    this->update_icons(); //Populate the icon list before proceeding further.
    try{
        QString* icon_paths = PresetManager::getPresetStringArray(preset->value("icon_paths"), 4);
        this->set_box_selection(icon_paths[0], this->study_icon);
        this->set_box_selection(icon_paths[1], this->short_break_icon);
        this->set_box_selection(icon_paths[2], this->long_break_icon);
        this->set_box_selection(icon_paths[3], this->complete_icon);
        delete [] icon_paths;
    }
    catch (PresetManager::json_value_error &ex){
        throw PresetManager::preset_error(preset_name, "icon_paths", ex);
    }
}

void NotificationEditor::setPlaceholders(const QJsonObject *preset){
    //First, make sure preset name is present.
    std::string preset_name;
    try{
        preset_name = PresetManager::getJsonVal<QString>(preset->value("preset_name")).toStdString();
    }
    catch (PresetManager::json_value_error &ex){
        throw PresetManager::preset_error();
    }
    try{
        QString* titles = PresetManager::getPresetStringArray((*preset)["notification_titles"]);
        for(int i = 0; i < 6; i++)
            this->title_inputs[i]->setText(titles[i]);
    }
    catch (PresetManager::json_value_error &ex){
        throw PresetManager::preset_error(preset_name, "notification_titles", ex);
    }
    try{
        QString* messages = PresetManager::getPresetStringArray((*preset)["notification_messages"]);
        for(int i = 0; i < 6; i++)
            this->message_inputs[i] = this->init_message_inputs[i] = messages[i];
    }
    catch (PresetManager::json_value_error &ex){
        throw PresetManager::preset_error(preset_name, "notification_messages", ex);
    }
}

//Slots
//Overrides submit and restart to apply the updated preset instead.
void PresetEditor::submit_and_restart(){
    if (this->submit()){
        //Load updated preset from preset manager.
        this->preset_manager->loadPreset(this->new_preset->value("preset_name").toString());
        this->parentTimer->ResetSession();
    }
    //Restart the prog
}

//Overrides the apply_changes function to construct and push an QJsonObject with the presets to the PresetManager object.
bool PresetEditor::apply_changes(double new_vals[5], QString (&new_titles)[6], QString(&new_messages)[6]){
    //Create a new iteration of the qjsonobject.
    bool check = this->s_edit->checkCycleLimit();
    check = !check;
    QString preset;
    QString original_name = (*(this->original_preset))["preset_name"].toString();
    if(this->preset_name_title->text().trimmed().isEmpty()){
        if(original_name.isEmpty())
            return false;
        preset = original_name;
    }
    else{
        preset = this->preset_name_title->text().trimmed();
    }
    qDebug("%s", QString(QString("Applying edits to preset ") + preset).toStdString().c_str());
    int * units = new int[3];
    this->s_edit->get_units(units);
    QJsonArray new_title_list;
    QJsonArray new_message_list;
    for (int i = 0; i < 6; i++){
        new_title_list.append(new_titles[i]);
        new_message_list.append(new_messages[i]);
    }
    QStringList icon_list = this->s_edit->getIconNames();
    QJsonArray icons;
    if(icon_list.length() == 4){
        for(int i = 0; i < 4; i++)
            icons.append(icon_list.at(i));
    }
    else{
        qDebug("Size of icons array was bad size %lli", icon_list.length());
        icons = QJsonArray({
                    QString(PresetManager::DEFAULT_STUDY_ICON_PATH),
                    QString(PresetManager::DEFAULT_BREAK_ICON_PATH),
                    QString(PresetManager::DEFAULT_BREAK_ICON_PATH),
                    QString(PresetManager::DEFAULT_BREAK_ICON_PATH)
                });
    }
    QJsonObject new_preset = {
        {"preset_name", preset},
        {"len_study", QJsonArray({new_vals[0], units[0]})},
        {"len_break_s", QJsonArray({new_vals[1], units[1]})},
        {"len_break_l", QJsonArray({new_vals[2], units[2]})},
        {"max_pomodoros", new_vals[3]},
        {"cycle_lim_enabled", check},
        {"max_cycles", new_vals[4]},
        {"notification_messages", new_message_list},
        {"notification_titles", new_title_list},
        {"icon_paths", icons}
    };
    //Update the timer preset
     //preset == enables overwriting a preset with the same name.
    bool ok;
    if(original_name.isEmpty()){
        ok = this->preset_manager->create_preset(new_preset);
        if(!ok){
            emit this->request_overwrite("Preset " + preset + " exists.", "Overwrite it?", ok);
            ok = this->preset_manager->create_preset(new_preset, ok);
        }
    }
    else if (!update_default){
        ok = this->preset_manager->update_preset(original_name, new_preset, preset == original_name);
        if(!ok){
            emit this->request_overwrite("Preset " + preset + " exists.", "Overwrite it?", ok);
            ok = this->preset_manager->update_preset(original_name, new_preset, ok);
        }
    }
    else{
        ok = this->preset_manager->update_default_preset(new_preset);
    }
    if(ok){
        this->new_preset = new QJsonObject(new_preset); //Makes a copy of the newly updated preset.
        this->close();
    }
    return ok;
}
