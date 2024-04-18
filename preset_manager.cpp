/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#include "preset_manager.h"
//Functions in this file should relate to the management of the preset library as a whole.
//This includes loading and saving the preset files, managing the default preset, etc.
//The default default preset, used if default.json is missing.
const QJsonObject* PresetManager::DEFAULT_PRESET = new QJsonObject({
    {"preset_name", QString("Default")},
    {"len_study", QJsonArray({25.0, 1})},
    {"len_break_s", QJsonArray({5.0, 1})},
    {"len_break_l", QJsonArray({20.0, 1})},
    {"cycle_lim_enabled", true},
    {"max_pomodoros", 2},
    {"max_cycles", 4},
    {"notification_titles", QJsonArray({
        QString("Starting Study Session"), QString("Study Segment Complete"), QString("Study Cycle Complete"),
        QString("Short Break Complete"), QString("Study Session Complete"), QString("Restarting Study Session")})},
    {"notification_messages", QJsonArray({
        QString("Good luck!"), QString(("Nice job out there. You have completed <current_pomodoro> pomodoros.\nEnjoy your short break!")),
        QString(("Congratulations! You have completed <current_pomodoro> pomodoros, and have earned your self a long break!")),
        QString("Hope you enjoyed the break! Now, GET BACK TO WORK!"),
        QString("Congratulations! Hope you got a lot done!"), QString("Time to get some more work done!")})}
});

//Constructor that reads in presets from a preset file path passed as an argument, skipping if one does not exist.
PresetManager::PresetManager(QString path, QWidget *parent) : QObject(parent){ //path = The path in which the preset files are stored at.
    //Create QFile object for path, and determine if it exists.
    this->preset_path = path;
    this->default_preset = this->readDefaultPresetFile();
    this->loadPresetFile();
    //Go through the array of presets, and add an action to each of the lists for them.
    for(QJsonArray::const_iterator i = this->presets->constBegin(); i != this->presets->constEnd(); i++){
        this->preset_load_actions.append(new QAction(PresetManager::getJsonVal<QString>((*i).toObject()["preset_name"])));
        this->preset_delete_actions.append(new QAction(PresetManager::getJsonVal<QString>((*i).toObject()["preset_name"])));
        this->preset_edit_actions.append(new QAction(PresetManager::getJsonVal<QString>((*i).toObject()["preset_name"])));
        this->preset_rename_actions.append(new QAction(PresetManager::getJsonVal<QString>((*i).toObject()["preset_name"])));
        this->preset_new_default_actions.append(new QAction(PresetManager::getJsonVal<QString>((*i).toObject()["preset_name"])));
    }
}
//When the PresetManager object is destroyed, write the preset files before the program closes.
PresetManager::~PresetManager(){
    if(!QDir(this->getPresetPath()).exists())
        QDir().mkpath(this->getPresetPath());
    bool def_write = this->writeDefaultPresetFile();
    //std::cout << std::boolalpha << def_write << " file written?" << std::endl;
    if (def_write) //Only write presets file if the default preset file was written successfully.
        this->writePresetFile();
}
//Populate preset menu entry list.
void PresetManager::populate_preset_menu_entries(QMenu* newMenuLoad, QMenu* newMenuDelete, QMenu* newMenuEdit, QMenu* newMenuRename, QMenu *newDefaultPreset){
    QAction* default_preset = new QAction("Default");
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
//Load the presets into the preset file.
void PresetManager::loadPresetFile(){
    this->presets = this->readPresetFile();
    this->presets_loaded = true;
}

const QString PresetManager::getPresetPath(){
    return this->preset_path + "/pomodoro";
}

int PresetManager::getSegmentLength(const QJsonValue arr){
    try{
        if(arr.isUndefined())
            throw (std::invalid_argument("Unit Array was invalid"));
        QJsonArray j_arr = arr.toArray();
        int unit = PresetManager::getJsonVal<int>(j_arr[1]);
        double val = PresetManager::getJsonVal<int>(j_arr[0]);
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

//Gets an array of QStrings from a QJsonObject.
//Parse and confirm all values are good before returning the pointer.
QString* PresetManager::getPresetStringArray(const QJsonValue val){
    QString * arr = new QString[6];
    try{
        if(val.isUndefined())
            throw (std::invalid_argument("String Array was invalid"));
        QJsonArray j_arr = val.toArray();
        int pass = 0;
        for(QJsonArray::const_iterator i = j_arr.cbegin(); i != j_arr.cend() && pass < 6; i++){
            arr[pass++] = PresetManager::getJsonVal<QString>(*i);
        }
        return arr;
    }
    catch (std::invalid_argument &ex){
        delete [] arr;
        throw (std::invalid_argument("String from array was invalid"));
    }
}

bool PresetManager::load_default_preset(){
    emit this->presetLoaded(this->getDefaultPreset());
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
        //std::cout << "Writing default preset..." << std::endl;
        //std::cout << std::boolalpha << presets.isEmpty() << std::endl;
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
        //std::cout << "Writing presets..." << std::endl;
        int write_status = outfile.write(presets.toJson());
        outfile.close();
        return (write_status != -1);
    }
    else
        return false;
}
