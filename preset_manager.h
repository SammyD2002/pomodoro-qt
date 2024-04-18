/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#ifndef PRESET_MANAGER_H
#define PRESET_MANAGER_H
#include "widgets.h"
#include "timerconfig.h"
#include "pomodoro_timer.h"
class QJsonDocument;
class QJsonArray;
class TimerConfig;
struct ParseError;
class PresetManager : public QObject{
    Q_OBJECT

public:
//Member Attributes - Static Const
    //The path in which the preset file is stored at.
    static const QJsonObject* DEFAULT_PRESET;
//Member Functions - Static Const
    //Takes a QJsonArray used in a presets time, and returns that time in ms using the unit in the array.
    static int getSegmentLength(const QJsonValue arr);
    //Gets a const pointer to the default preset
    const QJsonObject* getDefaultPreset() const {return this->default_preset;}
    const QJsonObject* getPreset(int i) const {return new QJsonObject((*(this->presets))[i].toObject());}
    const QJsonObject* getPreset(QString s) const {return (this->findPreset(s) >= 0) ? this->getPreset(this->findPreset(s)) : nullptr;}
    //Return the index that a preset is located at, -1 otherwise.
    int findPreset(QString preset_name) const;
    //Loads the preset <ps_name> into the timer.
    bool loadPreset(QString ps_name); //Not const as the timer is updated.
//Member Functions - Static
    //Return the value contained in val as its proper type, throw an error if it is undefined.
    //This function allowed for the removal of multiple different functions for each type.
    template <typename T>
    static T getJsonVal(const QJsonValue val);
    static QString* getPresetStringArray(const QJsonValue val); //Parse and confirm all values are good before returning the pointer.
//Standard Member Functions
    //Constructor that reads in presets from a preset file path passed as an argument, skipping if one does not exist.
    PresetManager(QString path, QWidget* parent = nullptr);
    ~PresetManager();
    //void currPrefsToPreset(QString ps_name, PomodoroTimer* timer); //User prompted for prefix name elsewhere.
    void startEdit(QString ps_name);
    bool removePreset(QString ps_name);
    void loadPresetFile();
    bool update_preset(QJsonObject preset, int index, bool overwrite = false);
    bool update_default_preset(QString preset);
    bool rename_preset(int old, QJsonObject new_settings, bool overwrite = true);
    void populate_preset_menu_entries(QMenu* newMenuLoad, QMenu* newMenuDelete, QMenu* newMenuEdit, QMenu* newMenuRename, QMenu* newDefaultPreset);
    QJsonObject* readDefaultPresetFile();
    QJsonArray* readPresetFile();
    bool writeDefaultPresetFile();
    void insert_preset(QJsonObject preset);
    bool writePresetFile();
public slots:
    bool update_preset(QJsonObject preset, bool overwrite = false);
    bool load_default_preset();
signals:
    void presetFileLoaded();
    void preset_removed(QAction* load, QAction* del, QAction* edit, QAction* ren, QAction* def);
    void preset_added(QAction* load, QAction* del, QAction* edit, QAction* ren, QAction* def);
    bool presetLoaded(const QJsonObject*) const; //From my understanding, using const here will only make sender const.
private:
//Member Attributes
    const QJsonObject preset_template = {
        {"preset_name", QString()},
        {"len_study", QJsonArray({-1.0, 0})}, //length, unit.
        {"len_break_s", QJsonArray({-1.0,0})},
        {"len_break_l", QJsonArray({-1.0,0})},
        {"cycle_lim_enabled", true},
        {"max_pomodoros", -1},
        {"max_cycles", -1},
        {"notification_titles", QJsonArray({QString(), QString(), QString(), QString(), QString()})},
        {"notification_messages", QJsonArray({QString(), QString(), QString(), QString(), QString()})}
    };
    //DefaultDir: QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QList<QAction*> preset_load_actions;
    QList<QAction*> preset_delete_actions;
    QList<QAction*> preset_edit_actions;
    QList<QAction*> preset_rename_actions;
    QList<QAction*> preset_new_default_actions;
    QJsonArray* presets;
    QJsonObject* default_preset;
    QString preset_path;
    bool presets_loaded = false;
//Member Functions:
    const QString getPresetPath(); //Grabs the preset path.
};

//Replaces the getPreset<Type> functions.
template <class T>
T PresetManager::getJsonVal(QJsonValue val){
    //Create an object of t_type.
    if (val.isUndefined() || !(QMetaType::fromType<typeof(T)>().isValid()))
        throw (std::invalid_argument("Bad val"));
    T t_type = T();
    //Make a new t_type'd value for t.
    QJsonValue t_val(t_type);
    if (val.type() == t_val.type())
        return val.toVariant().value<T>();
    else{
        std::cout << "Val: " << val.type() << std::endl;
        std::cout << "T Json Type: " << t_val.type() << std::endl;
        throw (std::invalid_argument("Bad type!"));
    }
}

#endif // PRESET_MANAGER_H
