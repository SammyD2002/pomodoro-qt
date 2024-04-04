#ifndef PRESET_MANAGER_H
#define PRESET_MANAGER_H
#include "widgets.h"
#include "timerconfig.h"
//TODO: Create inherited class of timerconfig.h for editing presets:
class QJsonDocument;
class QJsonArray;
class PresetManager{
    Q_OBJECT
public:
    //The path in which the preset file is stored at.
    void currPrefsToPreset(QString ps_name, PomodoroTimer* timer); //User prompted for prefix name elsewhere.
    void startEdit(QString ps_name);
    void loadPreset(QString ps_name);
private:
    const QJsonObject preset_template = {
        {"preset_name", QString()},
        {"len_study", -1},
        {"len_break_s", -1},
        {"len_break_l", -1},
        {"max_pomodoros", -1},
        {"max_cycles", -1}
    };
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
    const QString getPresetPath(); //Grabs the preset path.
    QJsonArray* readPresetFile();
    bool writePresetFile();

};

#endif // PRESET_MANAGER_H
