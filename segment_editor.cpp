#include "timerconfig.h"

//Segment Editor Constructor:
SegmentEditor::SegmentEditor(TimerConfig *parent) : QWidget(parent){
    this->layout = new QGridLayout(this);
    this->parentConfig =  parent; //For some reason I couldn't get the cast parentWidget() thing to work properly, so this will do for now.
    //Study Editor
    this->studyLabel = new QLabel(tr("Study Segment Length:"));
    layout->addWidget(this->studyLabel, 0, 0);
    this->study = new QLineEdit(this);
    layout->addWidget(this->study, 0, 1);
    this->studyUnit = this->setupUnitBox(new QComboBox(this));
    layout->addWidget(this->studyUnit, 0, 2);

    //Short Break Editor
    this->short_break_label = new QLabel(tr("Short Break Length:"));
    layout->addWidget(this->short_break_label, 1, 0);
    this->short_break = new QLineEdit(this);
    layout->addWidget(this->short_break, 1, 1);
    this->short_break_unit = this->setupUnitBox(new QComboBox(this));
    layout->addWidget(this->short_break_unit, 1, 2);

    //Long Break Editor
    this->long_break_label = new QLabel(tr("Long Break Length"));
    layout->addWidget(this->long_break_label, 2, 0);
    this->long_break = new QLineEdit(this);
    layout->addWidget(this->long_break, 2, 1);
    this->long_break_unit = this->setupUnitBox(new QComboBox(this));
    layout->addWidget(this->long_break_unit, 2, 2);

    //Pomodoros per Cycle Editor
    this->p_per_c_label = new QLabel(tr("Pomodoros per Cycle:"));
    layout->addWidget(this->p_per_c_label, 0, 3);
    this->p_per_c = new QLineEdit(this);
    layout->addWidget(this->p_per_c, 0, 4);

    //Totals Session Cycles Editor
    this->m_cycle_label = new QLabel(tr("Cycles per Session"));
    layout->addWidget(this->m_cycle_label, 1, 3);
    this->m_cycle = new QLineEdit(this);
    layout->addWidget(this->m_cycle, 1, 4);
    this->m_cycle_enabled_label = new QLabel(tr("Don't Limit Cycles"));
    layout->addWidget(this->m_cycle_enabled_label, 2, 3);
    this->m_cycle_enabled = new QRadioButton();
    layout->addWidget(this->m_cycle_enabled, 2, 4);

    //Help button:
    this->help = new QPushButton("Help");
    layout->addWidget(this->help, 3, 0);

    //Connect the radio button to the thing that disables/enables the limit box. Should send isChecked().
    connect(m_cycle_enabled, &QRadioButton::toggled, this->m_cycle, &QLineEdit::setDisabled);

    //Connect the help button to the retrieve_help function.
    connect(this->help, &QPushButton::pressed, this, &SegmentEditor::retrieve_help);
}

QString SegmentEditor::ms_to_unit(int ms, int unit){
    float new_time_f = static_cast<float>(ms);
    switch(unit){ //Convert nums to smaller value until ms is reached.
    case 0: //Seconds
        new_time_f /= 1000; //Seconds to ms: ms = 1/1000 seconds -> 1000ms = 1s
        break;
    case 1: //min
        new_time_f /= 60000; //Minutes to Seconds: min = (ms / 60 * 1000) -> 60000ms = 1m
        break;
    case 2: //hours
        new_time_f /= 3600000; //Hours to Minutes = (ms / (60 * 60 * 1000) -> 3600000ms = 1hr
        break;
    };
    //float min = static_cast<float>(ms) / 1000; //ms->s
    //min /= 60; //s->min
    return QString::fromStdString(std::to_string(new_time_f));
}

//Init options in the unit selection box.
QComboBox* SegmentEditor::setupUnitBox(QComboBox* units){
    units->addItem(tr("Seconds")); //Index = 0
    units->addItem(tr("Minutes")); //Index = 1
    units->addItem(tr("Hours")); //Index = 2
    units->setCurrentIndex(1);
    return units;
}

void SegmentEditor::setPlaceholders(PomodoroTimer* parentTimer){
    this->study->setText(this->ms_to_unit(parentTimer->get_len_study_int(), this->studyUnit->currentIndex()));
    this->short_break->setText(this->ms_to_unit(parentTimer->get_len_break_s_int(), this->short_break_unit->currentIndex()));
    this->long_break->setText(this->ms_to_unit(parentTimer->get_len_break_l_int(), this->long_break_unit->currentIndex()));
    this->p_per_c->setText(parentTimer->get_m_pom_str());
    this->m_cycle->setText(parentTimer->get_m_cycle_str());
}

void SegmentEditor::getInputs(double (&settings_arr)[5], bool convert){
    settings_arr[0] = this->parentConfig->time_valid(this->study->text(), this->studyUnit->currentIndex(), convert);
    settings_arr[1] = this->parentConfig->time_valid(this->short_break->text(), this->short_break_unit->currentIndex(), convert);
    settings_arr[2] = this->parentConfig->time_valid(this->long_break->text(), this->long_break_unit->currentIndex(), convert);
    settings_arr[3] = static_cast<double>(this->parentConfig->cycles_valid(this->p_per_c->text()));
    settings_arr[4] = static_cast<double>(this->parentConfig->cycles_valid(this->m_cycle->text()));
}

void SegmentEditor::get_units(int arr[3]){
    arr[0] = this->studyUnit->currentIndex();
    arr[1] = this->short_break_unit->currentIndex();
    arr[2] = this->long_break_unit->currentIndex();
}


void SegmentEditor::retrieve_help(){
    help_browser::load_help("SegmentEditor", this->parentConfig);
}
