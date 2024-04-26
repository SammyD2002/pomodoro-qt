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
PresetManager::PresetManager(QWidget *parent, QString path) : QObject(parent){ //path = The path in which the preset files are stored at.
    //Create QFile object for path, and determine if it exists.
    qDebug("Preset files are in the same directory.");
    this->preset_file_path = path + "/presets.json";
    this->def_preset_path = path + "/default.json";
    this->default_preset_loaded = this->loadDefaultPresetFile();
    if (!default_preset_loaded)
        this->update_default_preset(*DEFAULT_PRESET);
    this->presets_loaded = this->loadPresetFile();
    //Go through the array of presets, and add an action to each of the lists for them.
    for(QJsonArray::const_iterator i = this->presets->constBegin(); i != this->presets->constEnd(); i++){
        try{
            QString preset_name = PresetManager::getJsonVal<QString>((*i).toObject()["preset_name"]);
            for(int i = 0; i < 6; i++)
                this->preset_actions[i].append(new QAction(preset_name));
        }
        catch (json_value_error &err){
            QString warning("Name at index ");
            warning += QString::number(std::distance(this->presets->constBegin(), i));
            warning += QString(" is/has ");
            warning += QString(err.what());
            qWarning("%s", warning.toStdString().c_str());
            for(int i = 0; i < 6; i++)
                this->preset_actions[i].append(nullptr);
        }
    }
}

PresetManager::PresetManager(QString def, QString ex, QWidget* parent) : QObject(parent){
    //Create QFile object for path, and determine if it exists.
    if(def.isEmpty())
        this->def_preset_path = this->getDefaultPresetDir() + "/default.json";
    else
        this->def_preset_path = def;
    if(ex.isEmpty())
        this->preset_file_path = this->getDefaultPresetDir() + "/presets.json";
    else
        this->preset_file_path = ex;
    this->default_preset_loaded = this->loadDefaultPresetFile();
    if (!default_preset_loaded)
        this->update_default_preset(*DEFAULT_PRESET);
    this->presets_loaded = this->loadPresetFile();
    //Go through the array of presets, and add an action to each of the lists for them.
    for(QJsonArray::const_iterator i = this->presets->constBegin(); i != this->presets->constEnd(); i++){
        try{
            QString preset_name = PresetManager::getJsonVal<QString>((*i).toObject()["preset_name"]);
            for(int i = 0; i < 6; i++)
                this->preset_actions[i].append(new QAction(preset_name));
        }
        catch (json_value_error &err){
            QString warning("Name at index ");
            warning += QString::number(std::distance(this->presets->constBegin(), i));
            warning += QString(" is/has ");
            warning += QString(err.what());
            qWarning("%s", warning.toStdString().c_str());
            for(int i = 0; i < 6; i++)
                this->preset_actions[i].append(nullptr);
        }
    }
}


//When the PresetManager object is destroyed, write the preset files before the program closes.
PresetManager::~PresetManager(){
    if(!QDir(QFileInfo(this->getDefPresetPath()).absolutePath()).exists())
        QDir().mkpath(QFileInfo(this->getDefPresetPath()).absolutePath());
    if(!QDir(QFileInfo(this->getPresetFilePath()).absolutePath()).exists())
        QDir().mkpath(QFileInfo(this->getPresetFilePath()).absolutePath());
    if (this->default_preset_loaded){ //This is only set to false if there is an issue with the actual default preset file.
        bool def_write = this->writeDefaultPresetFile();
        if (!def_write)
            qCritical("The default preset file could not be written.");
    }
    //std::cout << std::boolalpha << def_write << " file written?" << std::endl;
    if (this->presets_loaded){ //Write to presets file if it didn't exist or if it was successfully loaded.
        bool pre_write = this->writePresetFile();
        if (!pre_write)
            qCritical("The extra preset file could not be written.");
    }

}
//Populate preset menu entry list.
void PresetManager::populate_preset_menu_entries(QMenu *menus[6]){
    QAction* default_preset_load = new QAction("Default");
    QAction* default_preset_edit = new QAction("Default");
    menus[0]->addAction(default_preset_load);
    menus[2]->addAction(default_preset_edit);
    menus[0]->addSeparator();
    menus[2]->addSeparator();
    for(int i = 0; i < 6; i++){
        for(int j = 0; j < this->preset_actions[i].length(); j++){
            if(preset_actions[i][j] != nullptr)
                menus[i]->addAction(preset_actions[i][j]);
        }
    }
}

void PresetManager::validate_preset(const QJsonObject* preset){
    //First, make sure preset_name is present
    std::string preset_name;
    try{
        preset_name = PresetManager::getJsonVal<QString>(preset->value("preset_name")).toStdString();
    }
    catch (json_value_error &err){
        throw preset_error();
    }
    QStringList keys = PresetManager::DEFAULT_PRESET->keys();
    QString missing;
    for(QStringList::const_iterator i = keys.cbegin(); i != keys.cend(); i++){
        if(!preset->contains(*i))
            if (missing.isEmpty())
                missing += *i;
            else
                missing += ", " + *i;
        if (missing.isEmpty()){
            try{
                if(*i == "len_study" || *i == "len_break_l" || *i == "len_break_s"){
                    int time = PresetManager::getSegmentLength(preset->value(*i));
                    if (time <= 0)
                        throw json_value_error("an invalid time");
                }
                else if(*i == "cycle_lim_enabled")
                    PresetManager::getJsonVal<bool>(preset->value(*i)); //Make sure conversion works.
                else if(*i == "max_cycles" || *i == "max_pomodoros"){
                    int max = PresetManager::getJsonVal<int>(preset->value(*i));
                    if(max <= 0)
                        throw json_value_error("an invalid limit");
                }
                else if(*i == "notification_titles" || *i == "notification_messages")
                    PresetManager::getPresetStringArray(preset->value(*i), true); //This function validates.
                else if(*i == "preset_name"){
                    if (preset_name.empty())
                        throw json_value_error("is missing.");
                }
            }
            catch (json_value_error &err){
                std::string attr = (*i).toStdString();
                throw preset_error(preset_name, attr, err);
            }
        }
    }
    if (!missing.isEmpty()){
        missing = QString("is missing the field(s) ") + missing;
        throw preset_error(preset_name, json_value_error(missing.toStdString()));
    }
}


QJsonObject* PresetManager::CopyPreset(const QJsonObject* base){
    PresetManager::validate_preset(base);
    return new QJsonObject(*base);
}


bool PresetManager::update_default_preset(QString preset){
    int index = this->findPreset(preset);
    if (index >= 0){
        QJsonObject* new_default = PresetManager::CopyPreset(this->getPreset(index));
        delete this->default_preset;
        this->default_preset = new_default;
        return true;
    }
    return false;
}

bool PresetManager::update_default_preset(QJsonObject preset){
    if (!(preset.isEmpty())){
        delete this->default_preset;
        this->default_preset = new QJsonObject(preset);
        return true;
    }
    return false;
}
bool PresetManager::loadDefaultPresetFile(){
    try{
        qDebug("%s", (std::stringstream() << "Loading default preset file at " << qPrintable(def_preset_path)).str().c_str());
        this->default_preset = this->readDefaultPresetFile();
    }
    catch (QJsonParseError::ParseError){
        qWarning("Default preset file had bad formatting. Using factory default preset instead.");
        return false;
    }
    return true;
}

//Load the presets into the preset file.
bool PresetManager::loadPresetFile(){
    try{
        qDebug("%s", (std::stringstream() << "Loading preset file at " << qPrintable(preset_file_path)).str().c_str());
        this->presets = this->readPresetFile();
    }
    catch (QJsonParseError::ParseError){
        qWarning("Preset file had bad formatting. No presets created will be saved.");
        return false;
    }
    return true;
}

const QString PresetManager::getPresetFilePath(){
    return this->preset_file_path;
} //Grabs the preset path.
const QString PresetManager::getDefPresetPath(){
    return this->def_preset_path;
}
/*
const QString PresetManager::getPresetPath(){
    return this->preset_path;
}
*/
int PresetManager::getSegmentLength(const QJsonValue arr){
    if(arr.isUndefined())
        throw (json_value_error("undefined"));
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
        break;
    default:
        throw(json_value_error("a value out of unit range " + std::to_string(unit)));
    }
    return final;
}

//Gets an array of QStrings from a QJsonObject.
//Parse and confirm all values are good before returning the pointer.
QString* PresetManager::getPresetStringArray(const QJsonValue val, bool no_return){
    QString * arr = new QString[6];
    try{
        QJsonArray j_arr = PresetManager::getJsonVal<QJsonArray>(val);
        if (j_arr.size() != 6)
            throw json_value_error("bad size " + std::to_string(j_arr.size()));
        int pass = 0;
        for(QJsonArray::const_iterator i = j_arr.cbegin(); i != j_arr.cend(); i++){
            arr[pass++] = PresetManager::getJsonVal<QString>(*i);
        }
        if(no_return){
            delete [] arr;
            return nullptr;
        }
        return arr;
    }
    catch (json_value_error &ex){
        delete [] arr;
        throw ex;
    }
}

bool PresetManager::load_default_preset(){
    emit this->presetLoaded(this->getDefaultPreset());
    return true;
}

QJsonObject* PresetManager::readDefaultPresetFile(){
    QJsonDocument presets;
    QFile infile(this->getDefPresetPath());
    if (infile.exists() && QFileInfo(infile).isReadable()){
        infile.open(QIODevice::ReadOnly);
        presets = QJsonDocument::fromJson(infile.readAll());
    }
    else{
        qDebug("Default preset file could not be found.");
    }
    infile.close();
    if (presets.isEmpty() || presets.isNull()){
        qDebug("Initializing a new Preset List QJsonArray.");
        return new QJsonObject(*(PresetManager::DEFAULT_PRESET));
    }
    return new QJsonObject(presets.object());
}

QJsonArray* PresetManager::readPresetFile(){
    QJsonDocument presets;
    QFile infile(this->getPresetFilePath());
    if (infile.exists() && QFileInfo(infile).isReadable()){
        infile.open(QIODevice::ReadOnly);
        presets = (QJsonDocument::fromJson(infile.readAll()));
    }
    else{
        qDebug("The Preset file could not be found.");
    }
    infile.close();
    if (presets.isEmpty() || presets.isNull()){
        qDebug("Initializing a new Preset List QJsonArray.");
        return new QJsonArray();
    }
    return new QJsonArray(presets.array());
}

bool PresetManager::writeDefaultPresetFile(){
    QJsonDocument presets;
    presets.setObject(*(this->getDefaultPreset()));
    QFile outfile(this->getDefPresetPath());
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
    QFile outfile(this->getPresetFilePath());
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
