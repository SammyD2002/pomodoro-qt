/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
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
    PresetEditor(PomodoroTimer* timer, QJsonObject* preset, PresetManager* current_presets, QWidget* parent, bool update_default=false);
    static PresetEditor* start_edit(PomodoroTimer *timer, QString preset_name, PresetManager* current_presets, QWidget* parent, bool update_default=false);
    ~PresetEditor();
signals:
    void request_overwrite(QString Title, QString Message, bool &result, QString accept = QStringLiteral("Yes"), QString reject = QStringLiteral("No"));
private:
    bool update_default;
    const QJsonObject* original_preset;
    QJsonObject* new_preset = nullptr;
    PresetManager* preset_manager;
    virtual bool apply_changes(double new_vals[5], QString (&new_titles)[6], QString(&new_messages)[6]) override;
    QJsonObject construct_preset(double new_vals[5], QString (&new_titles)[6], QString(&new_messages)[6]);
    QLabel* preset_name_label;
    QLineEdit* preset_name_title;
private slots:
    void submit_and_restart() override;
};

class PresetSegmentEditor : public SegmentEditor{
    Q_OBJECT
public:
    PresetSegmentEditor(TimerConfig* parent) : SegmentEditor(parent) {}
    virtual bool convert() const override {return false;} //Stops inputted time values from being converted to ms.
protected:
    void retrieve_help(){help_browser::load_help(QStringLiteral("PresetEditor"));}
};

class PresetNotificationEditor : public NotificationEditor{
    Q_OBJECT
public:
    PresetNotificationEditor(TimerConfig* parent) : NotificationEditor(parent) {}
protected:
    void retrieve_help(){help_browser::load_help(QStringLiteral("PresetEditor"));}
};

#endif // PRESET_EDITOR_H
