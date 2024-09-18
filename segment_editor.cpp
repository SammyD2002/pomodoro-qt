/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#include "timerconfig.h"

//Segment Editor Constructor:
SegmentEditor::SegmentEditor(TimerConfig *parent) : QWidget(parent){
    this->layout = new QGridLayout(this);
    this->parentConfig =  parent; //For some reason I couldn't get the cast parentWidget() thing to work properly, so this will do for now.
    //Labels
    this->layout->addWidget(new QLabel(QStringLiteral("Segment")), 0, 0, Qt::AlignTop);
    this->layout->addWidget(new QLabel(QStringLiteral("Length")), 0, 1, Qt::AlignTop);
    this->layout->addWidget(new QLabel(QStringLiteral("Unit")), 0, 2, Qt::AlignTop);
    //Study Editor
    this->studyLabel = new QLabel(QStringLiteral("Study Segment Length:"));
    layout->addWidget(this->studyLabel, 1, 0, Qt::AlignTop);
    this->study = new QLineEdit(this);
    layout->addWidget(this->study, 1, 1, Qt::AlignTop);
    this->studyUnit = this->setupUnitBox(new QComboBox(this));
    layout->addWidget(this->studyUnit, 1, 2, Qt::AlignTop);

    //Short Break Editor
    this->short_break_label = new QLabel(QStringLiteral("Short Break Length:"));
    layout->addWidget(this->short_break_label, 2, 0, Qt::AlignTop);
    this->short_break = new QLineEdit(this);
    layout->addWidget(this->short_break, 2, 1, Qt::AlignTop);
    this->short_break_unit = this->setupUnitBox(new QComboBox(this));
    layout->addWidget(this->short_break_unit, 2, 2, Qt::AlignTop);

    //Long Break Editor
    this->long_break_label = new QLabel(QStringLiteral("Long Break Length:"));
    layout->addWidget(this->long_break_label, 3, 0, Qt::AlignTop);
    this->long_break = new QLineEdit(this);
    layout->addWidget(this->long_break, 3, 1, Qt::AlignTop);
    this->long_break_unit = this->setupUnitBox(new QComboBox(this));
    layout->addWidget(this->long_break_unit, 3, 2, Qt::AlignTop);

    //Seperator Frame below cycle length fields.
    QFrame* sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameStyle(QFrame::Plain); //Effectivly turns this into a simple spacer.
    layout->addWidget(sep, 4, 0, 1, -1);

    //Pomodoros per Cycle Editor
    this->p_per_c_label = new QLabel(QStringLiteral("Pomodoros per Cycle"));
    layout->addWidget(this->p_per_c_label, 5, 0, Qt::AlignTop);
    this->p_per_c = new QLineEdit(this);
    layout->addWidget(this->p_per_c, 6, 0, Qt::AlignTop);

    //Totals Session Cycles Editor
    this->m_cycle_label = new QLabel(QStringLiteral("Cycles per Session"));
    layout->addWidget(this->m_cycle_label, 5, 1, Qt::AlignTop);
    this->m_cycle = new QLineEdit(this);
    layout->addWidget(this->m_cycle, 6, 1, Qt::AlignTop);
    this->m_cycle_enabled_label = new QLabel(QStringLiteral("Disable Cycle Limit"));
    layout->addWidget(this->m_cycle_enabled_label, 5, 2, Qt::AlignTop);
    this->m_cycle_enabled = new QRadioButton();
    layout->addWidget(this->m_cycle_enabled, 6, 2, Qt::AlignTop);

    layout->addWidget((this->study_icon = new QComboBox()), 1, 3, Qt::AlignTop);
    layout->addWidget((this->short_break_icon = new QComboBox()), 2, 3, Qt::AlignTop);
    layout->addWidget((this->long_break_icon = new QComboBox()), 3, 3, Qt::AlignTop);
    //Complete Icon Image Label:
    layout->addWidget(new QLabel(QStringLiteral("Session Complete Icon")), 5, 3, Qt::AlignTop);
    layout->addWidget((this->complete_icon = new QComboBox()), 6, 3, Qt::AlignTop);

    //Help button:
    this->help = new QPushButton(QStringLiteral("Help"));
    layout->addWidget(this->help, 7, 0, Qt::AlignBottom);


    //New Image Button
    this->new_image = new QPushButton(QStringLiteral("Import Icon from File..."));
    layout->addWidget(this->new_image, 7, 3, Qt::AlignTop);

    //Connect the radio button to the thing that disables/enables the limit box. Should send isChecked().
    connect(m_cycle_enabled, &QRadioButton::toggled, this->m_cycle, &QLineEdit::setDisabled);

    //Connect the help button to the retrieve_help function.
    connect(this->help, &QPushButton::pressed, this, &SegmentEditor::retrieve_help);

    //Connect the selection QAction to the get new image function.
    connect(this->new_image, &QPushButton::pressed, this, &SegmentEditor::load_new_image);
}

//Prompts the user to select a file to load as a new icon.
void SegmentEditor::load_new_image(){
    QString filename = QFileDialog::getOpenFileName();
    if (filename.isNull())
        return;
    try{
        const QIcon* ic = PresetManager::load_icon_file(filename);
        if(ic != nullptr){ //Will be nullptr if already added.
            icon_preview* ip = new icon_preview();
            ip->icon_base = ic;
            ip->icon_name = filename;
            this->icons.append(ip);
            this->study_icon->addItem(*(this->icons.last()->icon_base), "");
            this->short_break_icon->addItem(*(this->icons.last()->icon_base), "");
            this->long_break_icon->addItem(*(this->icons.last()->icon_base), "");
            this->complete_icon->addItem(*(this->icons.last()->icon_base), "");
        }
        else{
            qInfo("%s", (filename.toStdString() + " appears to already be imported.").c_str());
        }
    }
    catch (std::invalid_argument &err){
            QMessageBox::critical(this, QStringLiteral("Icon Import Error"), err.what());
    }
}

QString SegmentEditor::ms_to_unit(int ms, int unit) const{
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
    units->addItem(QStringLiteral("Seconds")); //Index = 0
    units->addItem(QStringLiteral("Minutes")); //Index = 1
    units->addItem(QStringLiteral("Hours")); //Index = 2
    units->setCurrentIndex(1);
    return units;
}

//Populates a new QComboBox with the available icons.
void SegmentEditor::populate_icon_list(QComboBox* icons){
    for(QList<icon_preview*>::const_iterator i = this->icons.constBegin(); i != this->icons.constEnd(); i++){
        icons->addItem(*((*i)->icon_base), "");
    }
}

//Sets placeholder values for input fields based on the timer's current settings.
void SegmentEditor::setPlaceholders(PomodoroTimer* parentTimer){
    this->study->setText(this->ms_to_unit(parentTimer->get_len_study_int(), this->studyUnit->currentIndex()));
    this->short_break->setText(this->ms_to_unit(parentTimer->get_len_break_s_int(), this->short_break_unit->currentIndex()));
    this->long_break->setText(this->ms_to_unit(parentTimer->get_len_break_l_int(), this->long_break_unit->currentIndex()));
    this->p_per_c->setText(parentTimer->get_m_pom_str());
    this->m_cycle->setText(parentTimer->get_m_cycle_str());
    this->update_icons();
    this->set_box_selection(parentTimer, this->study_icon, 0);
    this->set_box_selection(parentTimer, this->short_break_icon, 1);
    this->set_box_selection(parentTimer, this->long_break_icon, 2);
    this->set_box_selection(parentTimer, this->complete_icon, 3);
}

void SegmentEditor::update_icons(){
    //Clear all QComboBoxes with Icons.
    this->study_icon->clear();
    this->short_break_icon->clear();
    this->long_break_icon->clear();
    this->complete_icon->clear();
    //Icon Selection boxes
    while(!this->icons.isEmpty())
        delete this->icons.takeFirst();
    this->icons = PresetManager::icon_varient_manager->get_all_bases();
    //Move book and smiley to start of list so that things actually match up.
    for(int j = 0; j < this->icons.length(); j++){
        QString message = "Icon at index " + QString::number(j) + ": " + this->icons[j]->icon_name;
        qDebug(message.toStdString().c_str());
        if(this->icons.at(j)->icon_name == ":icons/book.svg"){
            icon_preview* item = this->icons[j];
            this->icons[j] = this->icons[0];
            this->icons[0] = item;
        }
        else if (this->icons.at(j)->icon_name == ":icons/smiley.svg"){
            icon_preview* item = this->icons[j];
            this->icons[j] = this->icons[1];
            this->icons[1] = item;
        }
    }
}
//Gets the icon in use by the timer for a given status and confirms it is not empty. Default is used when icon is empty.
void SegmentEditor::set_box_selection(PomodoroTimer* parent_timer, QComboBox* menu, int status){
    QString icon = parent_timer->get_icon_name(status);
    qDebug(("Searching for " + icon.toStdString() + "...").c_str());
    if(icon.isEmpty()){
        qInfo("%s %i %s", "Icon for status", status, "is empty. Falling back to default values...");
        QString* def_icons = PresetManager::getPresetStringArray(PresetManager::DEFAULT_PRESET->value("icon_paths"), 4);
        icon = def_icons[status];
        delete [] def_icons;
    }
    this->set_box_selection(icon, menu);
}

//Locates the icon in use by the timer in the appropriate QComboBox and sets it to the current index.
void SegmentEditor::set_box_selection(QString icon, QComboBox* menu){
    this->populate_icon_list(menu);
    for(int j = 0; j < this->icons.length(); j++){
        if(this->icons[j]->icon_name == icon){
            qDebug("%s %i", "Found at index", j);
            qDebug(("Icon Name: " + icons[j]->icon_name.toStdString()).c_str());
            menu->setCurrentIndex(j);
            break;
        }
    }
}

//Gets user inputs for time values by segment and places them into the settings array.
//If convert is true, converts them to ms.
void SegmentEditor::getInputs(double (&settings_arr)[5], bool convert) const{
    settings_arr[0] = this->parentConfig->time_valid(this->study->text(), this->studyUnit->currentIndex(), convert);
    settings_arr[1] = this->parentConfig->time_valid(this->short_break->text(), this->short_break_unit->currentIndex(), convert);
    settings_arr[2] = this->parentConfig->time_valid(this->long_break->text(), this->long_break_unit->currentIndex(), convert);
    settings_arr[3] = static_cast<double>(this->parentConfig->cycles_valid(this->p_per_c->text()));
    settings_arr[4] = static_cast<double>(this->parentConfig->cycles_valid(this->m_cycle->text()));
}

//Gets the units selected in each Unit Field and puts them into arr.
void SegmentEditor::get_units(int arr[3]) const{
    arr[0] = this->studyUnit->currentIndex();
    arr[1] = this->short_break_unit->currentIndex();
    arr[2] = this->long_break_unit->currentIndex();
}

//Gets the selected icon for each box icon box and finds the appropriate icon name.
QStringList SegmentEditor::getIconNames() const
{
    QStringList new_icons;
    icon_preview* study = this->icons.at(this->study_icon->currentIndex());
    icon_preview* break_s = this->icons.at(this->short_break_icon->currentIndex());
    icon_preview* break_l = this->icons.at(this->long_break_icon->currentIndex());
    icon_preview* done = this->icons.at(this->complete_icon->currentIndex());
    if (study != nullptr && break_s != nullptr && break_l != nullptr && done != nullptr)
        new_icons << study->icon_name << break_s->icon_name << break_l->icon_name << done->icon_name;
    return new_icons;

}

//Gets application help
void SegmentEditor::retrieve_help(){
    help_browser::load_help(QStringLiteral("SegmentEditor"));
}
