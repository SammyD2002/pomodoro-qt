#ifndef TIMERCONFIG_H
#define TIMERCONFIG_H
#include "widgets.h"
#include "pomodoro_ui.h"
class QDialog;
class QPushButton;
class QLineEdit;
class PomodoroTimer;
class QRadioButton;
class TimerConfig : public QDialog
{
    Q_OBJECT

public:
    TimerConfig(PomodoroTimer*);
    void setPlaceholders();

protected:
    void closeEvent(QCloseEvent *event) override;
signals:
    void study_updated(int); //Sent when config was updated
    void break_s_updated(int);
    void break_l_updated(int);
    void m_pomodoros_updated(int);
    void m_cycles_updated(int);
    void cycle_limit_toggled(bool);

private:
    //Move these to seperate menu option.
    //QPushButton* apply_preset;
    //QComboBox* presets;
    //QPushButton* save_preset;
    //QPushButton* remove_preset;
    PomodoroTimer* parentTimer;
    QDialog* configWindow;
    QPushButton* abort;
    QPushButton* conf_reset;
    QPushButton* conf_apply;
    QGridLayout* layout;
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
    //Return integer in ms if the user input is valid.
    bool input_is_int(QString, bool);
    QString ms_to_min(int);
    int time_valid(QString, int);
    int cycles_valid(QString);

private slots:
    //void savePreset(QString);
    //void loadPreset(QString);
    void submit();
    void submit_and_restart();
    //void cancel();
};

#endif // TIMERCONFIG_H
