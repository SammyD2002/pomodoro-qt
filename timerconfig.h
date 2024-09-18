/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#ifndef TIMERCONFIG_H
#define TIMERCONFIG_H
#include "widgets.h"
#include "pomodoro_timer.h"
#include "help_browser.h"
struct icon_preview;
class QDialog;
class QPushButton;
class QLineEdit;
class PomodoroTimer;
class QRadioButton;
class QTabWidget;
class SegmentEditor;
class PresetManager;
class NotificationEditor;

//The dialog contatining the tabbar and the confirm/cancel buttons.
//Also contains methods to apply changes.
class TimerConfig : public QDialog
{
    Q_OBJECT
public:
    TimerConfig(int buf_upper, QWidget* parent);
    TimerConfig(PomodoroTimer*);
    static const double UNIT_MULT[3];
    void setup();
    //Return integer in ms if the user input is valid.
    double time_valid(QString, int, bool convert = true) const;
    static int convert_time(double time, int unit);
    double validate_time_string(QString, int) const;
    int cycles_valid(QString) const;
protected:
    //Layout
    QGridLayout* layout;
    //Parent Timer
    PomodoroTimer* parentTimer;
//Buttons:
    QPushButton* abort;
    QPushButton* conf_reset;
    QPushButton* conf_apply;
//Editor Tabs:
    //Segment Editor
    SegmentEditor* s_edit;
    //Message Editors:
    NotificationEditor* title_edit;
    //Setup functions
    void setupTabBar();
    void setupButtons(QString conf_reset = QStringLiteral("Confirm and Reset"), QString conf_apply = QStringLiteral("Confirm"), QString abort = QStringLiteral("Cancel"));
    //Non-Pushbutton Exit functions
    void closeEvent(QCloseEvent *event) override;
    void reject() override; //This is called by the esc key.
signals:
    void segment_updated(int, int);
    void titles_updated(bool updated[6], QString updates[6]);
    void messages_updated(bool updated[6], QString updates[6]);
    void icons_updated(const QStringList &icons);
    void config_complete();
protected slots:
    bool submit();

private:
    //Move these to seperate menu option.
    QDialog* configWindow;
    QTabWidget* configTabBar;

    //Apply settings to running timer.
    virtual bool apply_changes(double new_vals[5], QString (&new_titles)[6], QString(&new_messages)[6]);
    //Confirms that the input is an integer.
    bool input_is_int(QString, bool) const;
    QDateTime input_is_formatted_time(QString, int) const;

private slots:
    virtual void submit_and_restart();
    //void cancel();
};

//Contains methods from the "Segment Editor Class"
class SegmentEditor : public QWidget {
    Q_OBJECT
public:
    SegmentEditor(TimerConfig *parent); //Forces TimerConfig's parent to be set to a TimerConfig object that isn't a nullptr.
    virtual void setPlaceholders(PomodoroTimer* parentTimer);
    virtual void setPlaceholders(const QJsonObject*);
    void getInputs(double (&settings_arr)[5], bool convert = true) const;
    //Converts ms to min for the placeholders
    QString ms_to_unit(int, int) const;
    bool checkCycleLimit() const {return this->m_cycle_enabled->isChecked();}
    void get_units(int arr[3]) const;
    QStringList getIconNames() const;
    //Do we convert the time to ms?
    virtual bool convert() const {return true;}
protected:
    QGridLayout* layout;
    TimerConfig* parentConfig;

    //Segment Lengths
    //Study Length
    QLabel* studyLabel;
    QLineEdit* study;
    QComboBox* studyUnit;
    QComboBox* study_icon;

    //Short Break Length
    QLabel* short_break_label;
    QLineEdit* short_break;
    QComboBox*  short_break_unit;
    QComboBox* short_break_icon;


    //Long Break Length
    QLabel* long_break_label;
    QLineEdit* long_break;
    QComboBox* long_break_unit;
    QComboBox* long_break_icon;

    //Max Pomodoros
    QLabel* p_per_c_label;
    QLineEdit* p_per_c;

    //Max Cycles
    QLabel* m_cycle_label;
    QLineEdit* m_cycle;
    QLabel* m_cycle_enabled_label;
    QRadioButton* m_cycle_enabled;
    QComboBox* complete_icon;

    //Init the units in the combo box:
    QComboBox* setupUnitBox(QComboBox*);
    QPushButton* help;

    //The list used to find the icon's name.
    QList<icon_preview*> icons;

    //Function used to populate said list.
    void populate_icon_list(QComboBox *icons);

    //QAction used to add new icons to the list.
    QPushButton* new_image;
    void set_box_selection(PomodoroTimer *parent_timer, QComboBox* menu, int status);
    void set_box_selection(QString icon, QComboBox* menu);
    void update_icons();
private slots:
    void retrieve_help();
    void load_new_image();
};

//Contains Methods from the "Notification Editor Class"
class NotificationEditor : public QWidget {
    Q_OBJECT
public:
    NotificationEditor(TimerConfig *parent);
    virtual void setPlaceholders(PomodoroTimer* parentTimer);
    virtual void setPlaceholders(const QJsonObject*);
    void getTitleInputs(QString (&src)[6]) const;
    void getMessageInputs(QString (&src)[6]) const;
protected:
    //Layout Handler
    QGridLayout* layout;
    //Parent Configuration
    TimerConfig* parentConfig;
    /*The index of the arrays correlates to the status.
    session_started_label=[0]   study_complete_label=[1]
    break_complete_label=[2]    pomodoro_cycle_complete_label=[3]
    session_complete_label=[4]  session_restart_label=[5] */
    //The Thing that the field edits.
    QLabel* items[6];
    QLabel* labels[6];
    QLineEdit* title_inputs[6];
    QPushButton* message_body_setters[6];
    QString message_inputs[6];
    QString init_message_inputs[6];
    QPushButton* help;
protected slots:
    void retrieve_help();
private slots:
    void edit_body();
};
#endif // TIMERCONFIG_H
