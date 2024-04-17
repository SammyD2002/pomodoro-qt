#include "preset_manager.h"
#include <iostream>
//Constructor that reads in presets from a preset file path passed as an argument, skipping if one does not exist.
PresetManager::PresetManager(QString path, QWidget *parent) : QObject(parent){
    //Open the file at the given path, and check for existence:
    this->preset_path = path;
    this->default_preset = this->readDefaultPresetFile();
    this->loadPresetFile();
    for(QJsonArray::const_iterator i = this->presets->constBegin(); i != this->presets->constEnd(); i++){
        this->preset_load_actions.append(new QAction(PresetManager::getJsonVal<QString>((*i).toObject()["preset_name"])));
        this->preset_delete_actions.append(new QAction(PresetManager::getJsonVal<QString>((*i).toObject()["preset_name"])));
        this->preset_edit_actions.append(new QAction(PresetManager::getJsonVal<QString>((*i).toObject()["preset_name"])));
        this->preset_rename_actions.append(new QAction(PresetManager::getJsonVal<QString>((*i).toObject()["preset_name"])));
        this->preset_new_default_actions.append(new QAction(PresetManager::getJsonVal<QString>((*i).toObject()["preset_name"])));
    }
}
//Write the presets file if it doesn't exist yet.
PresetManager::~PresetManager(){
    if(!QDir(this->getPresetPath()).exists())
        QDir().mkpath(this->getPresetPath());
    bool def_write = this->writeDefaultPresetFile();
        std::cout << std::boolalpha << def_write << " file written?" << std::endl;
    if(this->presets != NULL && !this->presets->isEmpty())
        this->writePresetFile();
}

bool PresetManager::rename_preset(int old, QJsonObject new_settings, bool overwrite){
    QString new_name = new_settings["preset_name"].toString();
    if (!overwrite && this->findPreset(new_name) >= 0)
        return false;
    this->presets->replace(old, new_settings);
    this->preset_load_actions[old]->setText(new_name);
    this->preset_delete_actions[old]->setText(new_name);
    this->preset_edit_actions[old]->setText(new_name);
    this->preset_rename_actions[old]->setText(new_name);
    return true;
}


int PresetManager::findPreset(QString preset_name){
    int index = 0;
    for (QJsonArray::const_iterator i = this->presets->cbegin(); i != this->presets->cend(); i++){
        if((*i).toObject()["preset_name"] == preset_name)
            return index;
        else
            index++;
    }
    return -1;
}
//Populate preset menu entry list.
void PresetManager::populate_preset_menu_entries(QMenu* newMenuLoad, QMenu* newMenuDelete, QMenu* newMenuEdit, QMenu* newMenuRename, QMenu *newDefaultPreset){
    QAction* default_preset = new QAction("Default");
    //connect(default_preset, &QAction::triggered, this, &PresetManager::load_default_preset);
    newMenuLoad->addAction(default_preset);
    newMenuLoad->addSeparator();

    newMenuLoad->addActions(this->preset_load_actions);
    newMenuDelete->addActions(this->preset_delete_actions);
    newMenuEdit->addActions(this->preset_edit_actions);
    newMenuRename->addActions(this->preset_rename_actions);
    newDefaultPreset->addActions(this->preset_new_default_actions);
}

bool PresetManager::update_default_preset(QString preset){
    int index = this->findPreset(preset);
    if (index >= 0){
        delete this->default_preset;
        this->default_preset = new QJsonObject(*(this->getPreset(index)));
        return true;
    }
    return false;

}


//The input is validated and confirmed in the editor function(s).
bool PresetManager::update_preset(QJsonObject preset, int index, bool overwrite){
    if(index >= 0){
        if(overwrite){
            this->presets->replace(index, preset);
            return true;
        } else
            return false;
    }
    else{
        this->presets->append(preset);
        this->preset_load_actions.append(new QAction(this->presets->last().toObject()["preset_name"].toString()));
        this->preset_delete_actions.append(new QAction(this->presets->last().toObject()["preset_name"].toString()));
        this->preset_edit_actions.append(new QAction(this->presets->last().toObject()["preset_name"].toString()));
        this->preset_rename_actions.append(new QAction(this->presets->last().toObject()["preset_name"].toString()));
        this->preset_new_default_actions.append(new QAction(this->presets->last().toObject()["preset_name"].toString()));
        emit this->preset_added(this->preset_load_actions.last(), this->preset_delete_actions.last(), this->preset_edit_actions.last(), this->preset_rename_actions.last(), this->preset_new_default_actions.last());
        return true;
    }
}

bool PresetManager::update_preset(QJsonObject preset, bool overwrite){
    return this->update_preset(preset, this->findPreset(preset["preset_name"].toString()), overwrite);
}


//Load the presets into the preset file.
void PresetManager::loadPresetFile(){
    this->presets = this->readPresetFile();
    this->presets_loaded = true;
}

const QString PresetManager::getPresetPath(){
    return this->preset_path + "/pomodoro";
}

const QJsonObject* PresetManager::DEFAULT_PRESET = new QJsonObject({
    {"preset_name", QString()},
    {"len_study", QJsonArray({25.0, 1})},
    {"len_break_s", QJsonArray({5.0, 1})},
    {"len_break_l", QJsonArray({20.0, 1})},
    {"cycle_lim_enabled", true},
    {"max_pomodoros", 2},
    {"max_cycles", 4},
    {"notification_titles", QJsonArray({
                                QString::fromStdString("Starting Study Session"),
                                QString::fromStdString("Study Segment Complete"),
                                QString::fromStdString("Study Cycle Complete"),
                                QString::fromStdString("Short Break Complete"),
                                QString::fromStdString("Study Session Complete"),
                                QString::fromStdString("Restarting Study Session")})},
    {"notification_messages", QJsonArray({
                                QString::fromStdString("Good luck!"),
                                QString::fromStdString(("Nice job out there. You have completed <current_pomodoro> pomodoros.\nEnjoy your short break!")),
                                QString::fromStdString(("Congratulations! You have completed <current_pomodoro> pomodoros, and have earned your self a long break!")),
                                QString::fromStdString("Hope you enjoyed the break! Now, GET BACK TO WORK!"),
                                QString::fromStdString("Congratulations! Hope you got a lot done!"),
                                QString::fromStdString("Time to get some more work done!")})}
    });

int PresetManager::getSegmentLength(const QJsonValue arr){
    try{
        if(arr.isUndefined())
            throw (std::invalid_argument("Unit Array was invalid"));
        QJsonArray j_arr = arr.toArray();
        int unit = PresetManager::getPresetInt(j_arr[1]);
        double val = PresetManager::getPresetDouble(j_arr[0]);
        int final = 0;
        switch(unit){
        case 2:
            val *= 60; //hr->min
        case 1:
            val *= 60; //min->s
        case 0:
            final = static_cast<int>(val) * 1000; //s->ms.
        }
        return final;
    }
    catch (std::invalid_argument &ex){
        return -1;
    }
}


double PresetManager::getPresetDouble(const QJsonValue val){
    if (val.isUndefined())
        throw (std::invalid_argument("Integer was invalid"));
    else
        return val.toDouble();
}

int PresetManager::getPresetInt(const QJsonValue val){
    if (val.isUndefined())
        throw (std::invalid_argument("Integer was invalid"));
    else
        return val.toInt();
}

QString PresetManager::getPresetString(const QJsonValue val){
    if (val.isUndefined())
        throw (std::invalid_argument("String was invalid"));
    else
        return val.toString();
}

QString* PresetManager::getPresetStringArray(const QJsonValue val){ //Parse and confirm all values are good before returning the pointer.
    QString * arr = new QString[6];
    try{
        if(val.isUndefined())
            throw (std::invalid_argument("String Array was invalid"));
        QJsonArray j_arr = val.toArray();
        int pass = 0;
        for(QJsonArray::const_iterator i = j_arr.cbegin(); i != j_arr.cend() && pass < 6; i++){
            arr[pass++] = PresetManager::getPresetString(*i);
        }
        return arr;
    }
    catch (std::invalid_argument &ex){
        delete [] arr;
        throw (std::invalid_argument("String from array was invalid"));
    }
}

/*
//The path in which the preset file is stored at.
void PresetManager::currPrefsToPreset(QString ps_name, PomodoroTimer* timer){ //User prompted for prefix name elsewhere.

}
void PresetManager::startEdit(QString ps_name){

}*/
bool PresetManager::load_default_preset(){
    emit this->presetLoaded(this->getDefaultPreset());
    return true;
}
bool PresetManager::loadPreset(QString ps_name){
    int index = this->findPreset(ps_name);
    if (index >= 0){
        emit this->presetLoaded(new QJsonObject((*(this->presets))[index].toObject()));
        return true;
    }
    return false;

}

bool PresetManager::removePreset(QString ps_name){
    int index = this->findPreset(ps_name);
    if(index >= 0){
        this->presets->removeAt(index);
    }
    else{
        return false;
    }
    for (int i = 0; i < this->preset_delete_actions.length(); i++){
        if (this->preset_delete_actions[i]->text() == ps_name.trimmed()){
            emit this->preset_removed(preset_load_actions[i], preset_delete_actions[i], preset_edit_actions[i], preset_delete_actions[i], preset_new_default_actions[i]);
            this->preset_load_actions.remove(i);
            this->preset_delete_actions.remove(i);
            this->preset_edit_actions.remove(i);
            this->preset_rename_actions.remove(i);
            this->preset_new_default_actions.remove(i);
            return true;
        }
    }
    return true;
}


QJsonObject* PresetManager::readDefaultPresetFile(){
    QJsonDocument presets;
    QFile infile(this->getPresetPath() + "/default.json");
    if (infile.exists() && QFileInfo(infile).isReadable()){
        infile.open(QIODevice::ReadOnly);
        presets = QJsonDocument::fromJson(infile.readAll());
    }
    infile.close();
    if (presets.isEmpty() || presets.isNull())
        return new QJsonObject(*(PresetManager::DEFAULT_PRESET));
    return new QJsonObject(presets.object());
}

QJsonArray* PresetManager::readPresetFile(){
    QJsonDocument presets;
    QFile infile(this->getPresetPath() + "/presets.json");
    if (infile.exists() && QFileInfo(infile).isReadable()){
        infile.open(QIODevice::ReadOnly);
        presets = (QJsonDocument::fromJson(infile.readAll()));
    }
    infile.close();
    if (presets.isEmpty() || presets.isNull())
        return new QJsonArray();
    return new QJsonArray(presets.array());
}

bool PresetManager::writeDefaultPresetFile(){
    QJsonDocument presets;
    presets.setObject(*(this->getDefaultPreset()));
    QFile outfile(this->getPresetPath() + "/default.json");
    if (!(outfile.exists()) || QFileInfo(outfile).isWritable()){
        //outfile << qPrintable(QJsonDocument::toJson()) << endl;
        outfile.open(QIODevice::WriteOnly);
        std::cout << "Writing default preset..." << std::endl;
        std::cout << std::boolalpha << presets.isEmpty() << std::endl;
        int write_status = outfile.write(presets.toJson());
        outfile.close();
        return (write_status != -1);
    }
    else
        return false;
}

bool PresetManager::writePresetFile(){
    QJsonDocument presets;
    presets.setArray(*(this->presets));
    QFile outfile(this->getPresetPath() + "/presets.json");
    if (!(outfile.exists()) || QFileInfo(outfile).isWritable()){
        //outfile << qPrintable(QJsonDocument::toJson()) << endl;
        outfile.open(QIODevice::WriteOnly);
        std::cout << "Writing presets..." << std::endl;
        int write_status = outfile.write(presets.toJson());
        outfile.close();
        return (write_status != -1);
    }
    else
        return false;
}
