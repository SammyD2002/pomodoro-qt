/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#include "preset_manager.h"
#define NUM_ICON_PARTITIONS 4
//Functions in this file should relate to the management of the preset library as a whole.
//This includes loading and saving the preset files, managing the default preset, etc.
//The default default preset, used if default.json is missing.
const QString PresetManager::DEFAULT_STUDY_ICON_PATH = QStringLiteral(":icons/book.svg");
const QString PresetManager::DEFAULT_BREAK_ICON_PATH = QStringLiteral(":icons/smiley.svg");
PresetIconManager* PresetManager::icon_varient_manager = nullptr;
//Defines the settings for the default preset used as fallback or on first run.
const QJsonObject* PresetManager::DEFAULT_PRESET = new QJsonObject({
    {"preset_name", QString("Default")},
    {"len_study", QJsonArray({25.0, 1})},
    {"len_break_s", QJsonArray({5.0, 1})},
    {"len_break_l", QJsonArray({20.0, 1})},
    {"cycle_lim_enabled", true},
    {"max_pomodoros", 4},
    {"max_cycles", 2},
    {"notification_titles", QJsonArray({
        QStringLiteral("Starting Study Session"), QStringLiteral("Study Segment Complete"), QStringLiteral("Study Cycle Complete"),
        QStringLiteral("Break Complete"), QStringLiteral("Study Session Complete"), QStringLiteral("Restarting Study Session")})},
    {"notification_messages", QJsonArray({
        QStringLiteral("Good luck!"), QStringLiteral("Nice job out there. You have completed <current_pomodoro> pomodoros.\nEnjoy your short break!"),
        QStringLiteral("Congratulations! You have completed <current_pomodoro> pomodoros, and have earned your self a long break!"),
        QStringLiteral("Hope you enjoyed the break! Now, GET BACK TO WORK!"),
        QStringLiteral("Congratulations! Hope you got a lot done!"), QStringLiteral("Time to get some more work done!")})},
        {"icon_paths", QJsonArray({
            PresetManager::DEFAULT_STUDY_ICON_PATH,
            PresetManager::DEFAULT_BREAK_ICON_PATH,
            PresetManager::DEFAULT_BREAK_ICON_PATH,
            PresetManager::DEFAULT_BREAK_ICON_PATH
        })}
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
    this->init_preset_list();
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
    this->init_preset_list();
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

//Loads the default preset or fallback preset, then loads and installs the presets from the preset file.
void PresetManager::init_preset_list(){
    this->icon_varient_manager = new PresetIconManager(NUM_ICON_PARTITIONS, this); //Create the icon variant manager.
    //Setup the default preset's icons:
    this->load_preset_icons(this->default_preset);
    //Setup the fallback icons.
    QIcon def_study = QIcon(QPixmap(DEFAULT_STUDY_ICON_PATH));
    QIcon def_break = QIcon(QPixmap(DEFAULT_BREAK_ICON_PATH));
    try{
        this->icon_varient_manager->add_icon(def_study, DEFAULT_STUDY_ICON_PATH);
        this->icon_varient_manager->add_icon(def_break, DEFAULT_BREAK_ICON_PATH);
    }
    catch (std::invalid_argument &err){
        qFatal(err.what());
    }
    //Attempt to load the basic preset file. If successful,
    this->presets_loaded = this->loadPresetFile();
    if(this->presets_loaded){
        this->install_presets();
    }
}

//Makes the presets accessible by creating preset actions. Also attempts to load the preset's icons from a file.
void PresetManager::install_presets(){
    //Go through the array of presets, and add an action to each of the lists for them.
    for(QJsonArray::const_iterator i = this->presets->constBegin(); i != this->presets->constEnd(); i++){
        if(i->isUndefined() || i->isNull() || !i->isObject()){
            qDebug("%s %lli %s", "BUG: Preset at index", (std::distance(this->presets->constBegin(), i)), "was undefined/NULL/invalid.");
            continue;
        }
        QJsonObject preset = i->toObject();
        try{
            QString preset_name = PresetManager::getJsonVal<QString>(preset["preset_name"]);
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
        this->load_preset_icons(&preset);
    }
}

//Loads an image from a file and adds it to the icon manager if successful. Returns the new image on success, nullptr on failure.
const QIcon* PresetManager::load_icon_file(const QString &filename, int urgency, bool returning){
    if(!QFile(filename).exists())
        qCritical("%s", (std::stringstream() << qPrintable(filename) << " could not be loaded. Is the filepath correct?").str().c_str());
    else if(!QFileInfo(filename).isReadable())
        throw std::invalid_argument(("Permission Denied when trying to read " + filename.toStdString() + "."));
    QPixmap pm;
    if(pm.load(filename)){
        const QIcon ic(pm);
        bool added = PresetManager::icon_varient_manager->add_icon(ic, filename, urgency);
        return (returning && added) ? new const QIcon(ic) : nullptr; //Bad design, but I wrote myself into a corner.
    }
    else{
        throw std::invalid_argument(("The file " + filename.toStdString() + " was an invalid image."));
    }
}

//Helper function to get the preset paths from the preset file and put them into an array.
QString* PresetManager::get_preset_icons(QJsonObject* preset) const {
    try{
        return PresetManager::getPresetStringArray(preset->value("icon_paths"), 4);
    }
    catch (json_value_error &err){
        QString message("The preset '");
        (preset->value("preset_name").isString()) ? message += PresetManager::getJsonVal<QString>(preset->value("preset_name")) : message += "<NAME UNDEFINED>";
        message += "' is missing the 'icon_paths' field.";
        qWarning(qPrintable(message));
        return PresetManager::getPresetStringArray(PresetManager::DEFAULT_PRESET->value("icon_paths"));
    }
}

//Takes 'icon_paths' from presets when present, and calls load_icon_file to install them into the icon manager.
//Has increased urgency for the default preset.
void PresetManager::load_preset_icons(QJsonObject* preset){
    QString* icon_files = this->get_preset_icons(preset);
    for(int i = 0; i < 4; i++){
        try{
            (preset == this->default_preset) ? this->load_icon_file(icon_files[i], i) : this->load_icon_file(icon_files[i]);
        }
        catch (std::invalid_argument &err){
            qCritical("%s", err.what());
        }
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

//Confirms a preset contains valid values.
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
        if(!preset->contains(*i)){
            if(*i != "icon_paths"){
                if (missing.isEmpty())
                    missing += *i;
                else
                    missing += ", " + *i;
            }
            else{
                qWarning("%s", (std::stringstream() << "The icon_paths field is missing, will use default icons...").str().c_str());
            }
        }
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
                    PresetManager::getPresetStringArray(preset->value(*i), 6, true); //This function validates.
                else if(*i == "icon_paths"){
                    try{
                        PresetManager::getPresetStringArray(preset->value(*i), 4, true); //This function validates.
                    }
                    catch (json_value_error &err){
                        qWarning("%s", (std::stringstream() << "The icon_paths field is/has " << err.what() << "").str().c_str());
                    }
                }
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

//Creates a copy of a preset after validation and returns a pointer to it.
QJsonObject* PresetManager::CopyPreset(const QJsonObject* base){
    PresetManager::validate_preset(base);
    return new QJsonObject(*base);
}

//Makes sure the preset specified by the preset string exists, replacting the original preset on success.
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

//Replaces the default preset with a copy of the one passed as an argument.
bool PresetManager::update_default_preset(QJsonObject preset){
    if (!(preset.isEmpty())){
        delete this->default_preset;
        this->default_preset = new QJsonObject(preset);
        return true;
    }
    return false;
}

//preset_name = 0,3,5 set the study icon, 1 & 2 set the break icon. 4 Is the end of the cycle, and doesn't alter the icon.
const QIcon* PresetManager::construct_tray_icon(int status, QString icon_name, int percent_complete){
    if(!icon_name.isEmpty()){
        try{
            return this->icon_varient_manager->get_icon(icon_name, percent_complete);
        }
        catch (preset_error &err){
            qCritical("%s", (std::stringstream() << "Error loading icon: " << err.what()).str().c_str());
            return this->construct_tray_icon(status, PresetManager::getJsonVal<QString>(DEFAULT_PRESET->value("icon_paths")[status]), percent_complete);
        }
        catch (json_value_error &err){
            qCritical("%s", (std::stringstream() << "Error loading icon: " << err.what()).str().c_str());
            return this->construct_tray_icon(status, PresetManager::getJsonVal<QString>(DEFAULT_PRESET->value("icon_paths")[status]), percent_complete);
        }
    }
    qWarning("Falling back to default icon...");
    return this->construct_tray_icon(status, PresetManager::getJsonVal<QString>(DEFAULT_PRESET->value("icon_paths")[status]), percent_complete);
}

//Loads the default preset from the default preset file, returning false on a badly formatted file.
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

//Grabs the user preset file path.
const QString PresetManager::getPresetFilePath(){
    return this->preset_file_path;
}
//Grabs the default preset path.
const QString PresetManager::getDefPresetPath(){
    return this->def_preset_path;
}

//Gets the length of a cycle segment in ms.
int PresetManager::getSegmentLength(const QJsonValue arr){
    if(arr.isUndefined())
        throw (json_value_error("undefined"));
    QJsonArray j_arr = arr.toArray();
    int unit = PresetManager::getJsonVal<int>(j_arr[1]);
    double val = PresetManager::getJsonVal<int>(j_arr[0]);
    int final = 0;
    switch(unit){
    case 2:
        val *= 3600; //hr->min (* 60);min->sec (* 60)
        break;
    case 1:
        val *= 60; //min->s;s->ms
        break;
    case 0:
        break;
    default:
        throw(json_value_error("a value out of unit range " + std::to_string(unit)));
    }
    final = static_cast<int>(val) * 1000; //sec->ms (*1000) [double->int]
    return final;
}

//Gets an array of QStrings from a QJsonObject.
//Parse and confirm all values are good before returning the pointer.
QString* PresetManager::getPresetStringArray(const QJsonValue val, int correct_size, bool no_return){
    QString * arr = new QString[correct_size];
    try{
        QJsonArray j_arr = PresetManager::getJsonVal<QJsonArray>(val);
        if (j_arr.size() != correct_size)
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

//Loads the default preset into the timer via the presetLoaded signal.
bool PresetManager::load_default_preset(){
    emit this->presetLoaded(*(this->getDefaultPreset()));
    return true;
}

//Reads the default preset from the default preset file after confirming its existence.
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

//Reads the non-default preset file after confirming its existence.
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

//Writes the default preset to the default preset file.
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

//Writes user presets to the preset file.
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
