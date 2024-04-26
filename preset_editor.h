#ifndef PRESET_EDITOR_H
#define PRESET_EDITOR_H
class TimerConfig;
class SegmentEditor;
class MessageEditor;
class NotificationEditor;
#include "preset_manager.h"
#include "timerconfig.h"

//Child Class of TimerConfig that is used to edit presets.
class PresetEditor : public TimerConfig {
    Q_OBJECT
public:
    //PresetEditor(PomodoroTimer* timer, PresetManager* current_presets, QWidget* parent);
    PresetEditor(PomodoroTimer* timer, QString preset_name, PresetManager* current_presets, QWidget* parent, bool update_default = false);
    ~PresetEditor();
signals:
    void request_overwrite(QString Title, QString Message, bool &result, QString accept = QString("Yes"), QString reject = QString("No"));
private:
    bool update_default;
    const QJsonObject* original_preset;
    QJsonObject* new_preset = NULL;
    PresetManager* preset_manager;
    virtual bool apply_changes(double new_vals[5], QString (&new_titles)[6], QString(&new_messages)[6]) override;
    QJsonObject construct_preset(double new_vals[5], QString (&new_titles)[6], QString(&new_messages)[6]);
    QLabel* preset_name_label;
    QLineEdit* preset_name_title;
    //void get_preset_values(int i) const; //Unused?
private slots:
    void submit_and_restart() override;
};

class PresetSegmentEditor : public SegmentEditor{
    Q_OBJECT
public:
    PresetSegmentEditor(TimerConfig* parent) : SegmentEditor(parent) {}
    virtual bool convert() const override {return false;}
};

class PresetNotificationTitleEditor : public NotificationEditor{
    Q_OBJECT
public:
    PresetNotificationTitleEditor(TimerConfig* parent) : NotificationEditor(parent) {}
};

class PresetNotificationMessageEditor : public MessageEditor{
    Q_OBJECT
public:
    PresetNotificationMessageEditor(TimerConfig* parent) : MessageEditor(parent) {}
};

#endif // PRESET_EDITOR_H
