#ifndef TIMERCONFIG_H
#define TIMERCONFIG_H
#include "widgets.h"
#include "pomodoro_ui.h"
class QDialog;
class QPushButton;
class QLineEdit;
class PomodoroTimer;
class QRadioButton;
class QTabWidget;
class SegmentEditor;
class NotificationEditor;

//The dialog contatining the tabbar and the confirm/cancel buttons.
//Also contains methods to apply changes.
class TimerConfig : public QDialog
{
    Q_OBJECT

public:
    TimerConfig(PomodoroTimer*);
    void setup();
    int time_valid(QString, int);
    int cycles_valid(QString);


protected:
    void closeEvent(QCloseEvent *event) override;
    void reject() override; //This is called by the esc key.
    //Put utility functions used for verifying data HERE.
signals:
    /*
    void study_updated(int); //Sent when config was updated
    void break_s_updated(int);
    void break_l_updated(int);
    void m_pomodoros_updated(int);
    void m_cycles_updated(int);
    void cycle_limit_toggled(bool);
    */
    void segment_updated(int, int);
    void titles_updated(bool updated[6], QString updates[6]);
    void messages_updated(bool updated[6], QString updates[6]);
    void config_complete();

private:
    //Move these to seperate menu option.
    //QPushButton* apply_preset;
    //QComboBox* presets;
    //QPushButton* save_preset;
    //QPushButton* remove_preset;
    PomodoroTimer* parentTimer;
    QDialog* configWindow;
    QTabWidget* configTabBar;
    QPushButton* abort;
    QPushButton* conf_reset;
    QPushButton* conf_apply;
    QGridLayout* layout;

    SegmentEditor* s_edit;
    NotificationEditor* title_edit;
    NotificationEditor* message_edit;
    //Message Editors:

    //Apply settings to running timer.
    virtual void apply_changes(int new_vals[5], QString (&new_titles)[6], QString(&new_messages)[6]);
    //Return integer in ms if the user input is valid.
    bool input_is_int(QString, bool);
    QDateTime input_is_formatted_time(QString, int);

private slots:
    //void savePreset(QString);
    //void loadPreset(QString);
    bool submit();
    void submit_and_restart();
    //void cancel();
};

//Contains methods from the "Segment Editor Class"
class SegmentEditor : public QWidget {
    Q_OBJECT
public:
    SegmentEditor(TimerConfig *parent); //Forces TimerConfig's parent to be set to a TimerConfig object that isn't a nullptr.
    void setPlaceholders(PomodoroTimer* parentTimer);
    void getInputs(int (&settings_arr)[5]);
    //Converts ms to min for the placeholders
    QString ms_to_unit(int, int);
    bool checkCycleLimit(){return this->m_cycle_enabled->isChecked();}
private:
    QGridLayout* layout;
    TimerConfig* parentConfig;

    //Segment Lengths    
    //Study Length
    QLabel* studyLabel;
    QLineEdit* study;
    QComboBox* studyUnit;

    //Short Break Length
    QLabel* short_break_label;
    QLineEdit* short_break;
    QComboBox*  short_break_unit;

    //Long Break Length
    QLabel* long_break_label;
    QLineEdit* long_break;
    QComboBox* long_break_unit;

    //Max Pomodoros
    QLabel* p_per_c_label;
    QLineEdit* p_per_c;

    //Max Cycles
    QLabel* m_cycle_label;
    QLineEdit* m_cycle;
    QLabel* m_cycle_enabled_label;
    QRadioButton* m_cycle_enabled;

    //Init the units in the combo box:
    QComboBox* setupUnitBox(QComboBox*);
    QPushButton* help;
private slots:
    void retrieve_help();
};

//Contains Methods from the "Notification Editor Class"
class NotificationEditor : public QWidget {
    Q_OBJECT
public:
    NotificationEditor(TimerConfig *parent);
    virtual void setPlaceholders(PomodoroTimer* parentTimer);
    void getTitleInputs(QString (&src)[6]) const;
protected:
    //Layout Handler
    QGridLayout* layout;
    //Parent Configuration
    TimerConfig* parentConfig;

    //The index of the arrays correlates to the status.
    QLabel* labels[6];
    QLineEdit* title_inputs[6];
    QPushButton* help;
protected slots:
    void retrieve_help();
    /*
    //Labels and inputs
    QLabel* session_started_label; [0]
    QLineEdit* session_started_title;

    QLabel* study_complete_label; [1]
    QLineEdit* study_complete_title;

    QLabel* break_complete_label; [2]
    QLineEdit* break_complete_title;

    //Pomodoro Cycle Complete
    QLabel* pomodoro_cycle_complete_label; [3]
    QLineEdit* pomodoro_cycle_complete_title;

    //Session Complete
    QLabel* session_complete_label; [4]
    QLineEdit* session_complete_title;

    //Session Restarted
    QLabel* session_restart_label; [5]
    QLineEdit* session_restart_title;
    */
};
//Created child class of notification editor for messages.
class MessageEditor : public NotificationEditor{
public:
    MessageEditor(TimerConfig* parent) : NotificationEditor(parent){}
    virtual void setPlaceholders(PomodoroTimer* parentTimer) override;
};

#endif // TIMERCONFIG_H
