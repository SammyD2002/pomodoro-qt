#ifndef PRESET_MANAGER_H
#define PRESET_MANAGER_H
#include "widgets.h"
#include "timerconfig.h"
//TODO: Create inherited class of timerconfig.h for editing presets:
class QJsonDocument;
class PresetManager{
    Q_OBJECT
public:
    //The path in which the preset file is stored at.
    void currPrefsToPreset(QString ps_name, PomodoroTimer* timer); //User prompted for prefix name elsewhere.
    void startEdit(QString ps_name);
    void loadPreset(QString ps_name);
    //Tree Balancing Functions:
    void balance(Preset*);
    bool insert(Preset*, Preset*); //Insert Preset object into table.
    //These should return the new leaf node of the calling party.
    Preset* rotate_l(Preset *trunk);
    Preset* rotate_r(Preset *trunk);
    Preset* rotate_rl(Preset *trunk);
    Preset* rotate_lr(Preset *trunk);
private:
    QJsonArray* presets;
    QString presetPath;
    //Preset editor window class:
    class PresetEditor : public TimerConfig{
    public:
        TimerPresets();
        //Specify Overrides here:
    signals:
        void prefix_updated();
    private:
        TimerPreset* presetManager;
        void apply_changes() override;
    };
    class Preset{
    public:
        QString getName() const {return this->name;}
        PresetSettings* getSettings() const {return this->settings;}
        int getWeight();
        int getTreeHeight(Preset* current);
        //Node getters and setters:
        Preset* getLeaf_r(){return this->preset_r;}
        Preset* getLeaf_l(){return this->preset_l;}
        void setLeaf_r(Preset* new_leaf){this->preset_r = new_leaf;}
        void setLeaf_l(Preset* new_leaf){this->preset_l = new_leaf;}
    private:
        QString name; //Name of the presets
        Preset* next = NULL;
        struct PresetSettings{
            int len_study;
            int len_break_s;
            int len_break_l;
            int max_pomodoros;
            int max_cycles;//Set next to NULL by default.
        };
        PresetSettings* settings;
        Preset* preset_l; //TODO: Change this to binary ast tree.
        Preset* preset_r;
        //Comparision operators
        bool operator==(const Preset& rPreset);
        bool operator!=(const Preset& rPreset);
        bool operator>(const Preset& rPreset);
        bool operator<(const Preset& rPreset);
        bool operator<=(const Preset& rPreset);
        bool operator>=(const Preset& rPreset);
        //Overloaded insert operation that takes a pointer to a Preset object.
    };
    Preset* presetList[26] = NULL; //Hash table go brrrrr
    const QString getPresetPath(); //Grabs the preset path.
    QJsonArray* readPresetFile();
    bool writePresetFile();

};

#endif // PRESET_MANAGER_H
