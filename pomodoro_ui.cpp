/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 * TODO: Create Notifier object to handle notifications.
 */
#include "pomodoro_ui.h"
//#include "ui_pomodoro_ui.h"
PomodoroUI::PomodoroUI(PresetManager* preset_manager, QJsonObject* starting_preset, QWidget *parent): QWidget(parent){
//Timer Setup
    //Event loops
    this->log_stdout = false;
    this->main_timer = new QTimer(this);
    this->loop_timer = new QTimer(this);
    //Setup the preset manager object.
    this->preset_manager = preset_manager;
    this->preset_manager->setParent(this);
    //Initialize the Pomodoro Cycle & set log_stdout and notify appropriatly.
    try{
        this->cycle = new PomodoroTimer(main_timer, starting_preset, log_stdout, this);
    }
    catch (PresetManager::preset_error &ex){
        delete this->cycle;
        qCritical("%s%s", ex.what(), " Reverting to default settings...");
        int units[3] = {1,1,1};
        this->cycle = new PomodoroTimer(main_timer, 25.0, 5.0, 15.0, units, 2, 4, log_stdout, true, this);
    }
//Main Window Visual Elements + Layout
    this->toggle = new QPushButton("Start");
    this->clock = new QLabel("");
    this->pc_status = new QLabel("");
    layout = new QGridLayout(this);
    layout->addWidget(toggle, 1, 0, 1, 2);
    layout->addWidget(clock, 2, 0); //-1 should put this on the right side of the window.
    layout->addWidget(pc_status, 2, 1);
    this->log_stdout = false;
    this->notify = true;
    this->config = new TimerConfig(this->cycle);
    //System Tray Visual Elements
    this->study_icon = QIcon (QPixmap(QString::fromStdString(":icons/book.svg")));
    this->breaktime_icon = QIcon (QPixmap(QString::fromStdString(":icons/smiley.svg")));
    this->tray = new QSystemTrayIcon(study_icon, this);
    this->tray->show(); //Reveal the icon in the system tray.
    this->tray_menu_items = new QList<QAction*>();
    //Configure Menus
    QMenu* presetMenu = new QMenu(tr("Presets"));
    QMenu* create_preset = presetMenu->addMenu(tr("&Create Preset"));
    QAction* save_current = new QAction(tr("From &Current Settings..."));
    create_preset->addAction(save_current);
    QAction* save_default = new QAction(tr("From &Default Settings..."));
    create_preset->addAction(save_default);
    this->preset_menus[0] = presetMenu->addMenu(tr("&Load Preset"));
    this->preset_menus[1] = presetMenu->addMenu(tr("&Remove Preset"));
    this->preset_menus[2] = presetMenu->addMenu(tr("&Edit Preset"));
    this->preset_menus[3] = presetMenu->addMenu(tr("Re&name Preset"));
    this->preset_menus[4] = presetMenu->addMenu(tr("Co&py Preset to Default Settings"));
    this->preset_menus[5] = create_preset->addMenu(tr("From &Existing Preset"));
    this->preset_manager->populate_preset_menu_entries(this->preset_menus);
    this->SetupMenus();
    //Main Window Context Menu
    this->top_bar = new QMenuBar(this);
    this->top_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QMenu* timer = this->top_bar->addMenu("Timer");
    timer->addAction(this->tray_menu_items->at(1)); //Pause from tray menu
    timer->addAction(this->tray_menu_items->at(2)); //Reset Segment
    timer->addAction(this->tray_menu_items->at(3)); //Reset Session
    timer->addSeparator();
    QAction* edit = new QAction(tr("&Edit Timer..."), this);
    timer->addAction(edit); //Edit Paramaters...
    this->top_bar->addMenu(presetMenu);
    QMenu* windowMenu = this->top_bar->addMenu("Window");
    windowMenu->addAction(this->tray_menu_items->at(0));
    QAction* help = new QAction(tr("&Help..."), this);
    windowMenu->addAction(help);
    windowMenu->addAction(this->tray_menu_items->at(4));
    this->top_bar->setNativeMenuBar(true);
    this->layout->setMenuBar(this->top_bar);
//Signal and Slot Connections
    //Setup connections activated when the timer is started/stopped.
    connect(this->toggle, SIGNAL (clicked()), this, SLOT (toggle_pressed()));
    connect(cycle, &PomodoroTimer::timer_toggled, this, &PomodoroUI::toggled);
    //Add connections activated when the event loop or the main timer expires.
    connect(loop_timer, &QTimer::timeout, this, &PomodoroUI::update_timer_display);
    connect(this->cycle, SIGNAL (segment_changed(int)), this, SLOT (update_segment(int)));
    if (notify){ //Only connect these sockets if notifications are enabled.
        connect(cycle, &PomodoroTimer::segment_changed, this, &PomodoroUI::notify_session);
    }
    //Connect the preset menu methods:
    connect(this->preset_menus[0], &QMenu::triggered, this, &PomodoroUI::attempt_preset_load);
    connect(create_preset, &QMenu::triggered, this, &PomodoroUI::settings_to_preset);
    connect(this->preset_menus[1], &QMenu::triggered, this, &PomodoroUI::attempt_preset_remove);
    connect(this->preset_menus[2], &QMenu::triggered, this, &PomodoroUI::attempt_preset_edit);
    connect(this->preset_manager, &PresetManager::presetLoaded, this->cycle, &PomodoroTimer::applyPreset);
    connect(this->preset_menus[3], &QMenu::triggered, this, &PomodoroUI::rename_preset);
    connect(this->preset_menus[4], &QMenu::triggered, this, &PomodoroUI::attempt_update_default);
    //Connect methods called for presets being added/removed.
    connect(this->preset_manager, &PresetManager::preset_added, this, &PomodoroUI::preset_added);
    connect(this->preset_manager, &PresetManager::preset_removed, this, &PomodoroUI::preset_removed);
    //System tray activation signal handler.
    connect(this->tray, &QSystemTrayIcon::activated, this, &PomodoroUI::window_toggle);
    //Connect le help menu
    connect(help, &QAction::triggered, this, &PomodoroUI::retrieve_help);
    //Connect editor configs:
    connect(edit, &QAction::triggered, this, &PomodoroUI::start_config);
    connect(this->config, &TimerConfig::config_complete, this, &PomodoroUI::finish_config);
    //Start the main & event loop timers.
    this->cycle->initCycle(false);
    this->loop_timer->start(this->LEN_LOOP);
}
//Destructor that destroys the timer and preset manager objects.
PomodoroUI::~PomodoroUI()
{
    delete this->cycle;
    delete this->preset_manager;
}
//Initializes the tray icon's menu.
void PomodoroUI::SetupMenus(){
    //The tray context menu
    QMenu* tray_actions = new QMenu(this);
    //Toggle window [0]
    QAction* toggle_window = new QAction(tr("&Show Window"), this);
    connect(toggle_window, &QAction::triggered, this, &PomodoroUI::update_visible);
    this->tray_menu_items->append(toggle_window);
    tray_actions->addAction(toggle_window);
    tray_actions->addSeparator();
    //Pause Timer [1]
    QAction* toggle_timer = new QAction(tr("&Pause Timer"), this);
    connect(toggle_timer, &QAction::triggered, this, &PomodoroUI::toggle_pressed);
    this->tray_menu_items->append(toggle_timer);
    tray_actions->addAction(toggle_timer);
    //Restart Segment [2]
    QAction* segment_restart = new QAction(tr("Restart Se&gment"), this);
    connect(segment_restart, &QAction::triggered, this->cycle, &PomodoroTimer::ResetSegment);
    this->tray_menu_items->append(segment_restart);
    tray_actions->addAction(segment_restart);
    //Restart Session [3]
    QAction* session_restart = new QAction(tr("Restart &Session"), this);
    connect(session_restart, &QAction::triggered, this->cycle, &PomodoroTimer::initCycle);
    this->tray_menu_items->append(session_restart);
    tray_actions->addAction(session_restart);
    tray_actions->addSeparator();
    //Load Preset [NOT IN ARRAY]. Simply added to the tray's context menu.
    tray_actions->addMenu(this->preset_menus[0]);
    tray_actions->addSeparator();
    //Exit Program [4]
    QAction* exit_program = new QAction(tr("&Exit"), this);
    connect(exit_program, &QAction::triggered, this, &PomodoroUI::quitting);
    this->tray_menu_items->append(exit_program);
    tray_actions->addAction(exit_program);
    //tray_actions->addActions(*(this->tray_menu_items));
    this->tray->setContextMenu(tray_actions);
    //Calling the destructor should quit the app.
    //connect(this, &PomodoroUI::quitting, this, &PomodoroUI::~PomodoroUI);
}
//Toggle the button's text and stop/start and the timer that updates the countdown.
void PomodoroUI::toggled(bool paused){
    if(paused){
        this->toggle->setText(QString::fromStdString(this->status[0]));
        this->tray_menu_items->at(1)->setText(tr("&Pause Timer"));
        loop_timer->start(this->LEN_LOOP);
    }
    else{
        this->toggle->setText(QString::fromStdString(this->status[1]));
        this->tray_menu_items->at(1)->setText(tr("&Resume Timer"));
        loop_timer->stop();
    }
    //Reset the tray icon tooltip appropriatly.
    this->UpdateTrayTooltip();
}
//Slots:
//Triggered by signal from the PomodoroTimer object.
//The integer recieved signals what state we transferred to.
//0 = Session started, 1 = study -> short break, 2 = study -> long break,
//3 = x break -> study, 4 = session over, 5 = session restart.
void PomodoroUI::update_segment(int status){
    //Update various icons.
    qDebug("Updating segment...");
    if(status == 0 || status || 5)
        this->loop_timer->start(this->LEN_LOOP); //Restart the loop timer if not running.
    if (status == 0 || status == 3 || status == 5){
        this->status[0] = "Studying";
        this->status[1] = "[Paused] Studying";
        this->tray->setIcon(this->study_icon);
        this->tray_menu_items->at(1)->setText(tr("&Pause Timer"));
    }
    else if (status == 4){
        this->status[0] = this->status[1] = "Restart";
        this->loop_timer->stop();
    }
    else if (status == 1){
        this->status[0] = "On Short Break";
        this->status[1] = "[Paused] On Short Break";
        this->tray->setIcon(this->breaktime_icon);
    }
    else{
        this->status[0] = "On Long Break";
        this->status[1] = "[Paused] On Long Break";
        this->tray->setIcon(this->breaktime_icon);
    }
    this->toggle->setText(QString::fromStdString(this->status[0]));
    this->UpdateTrayTooltip();
}
//Called every second to update the timer's display.
void PomodoroUI::update_timer_display(){
    QDateTime remTime = this->cycle->ZERO_TIME->addMSecs((this->main_timer->remainingTime() / 1000) * 1000);
    QString output = remTime.toString("hh:mm:ss");
    if (this->cycle->ZERO_TIME->date().daysTo(remTime.date()) >= 1)
        output = QString::number(this->cycle->ZERO_TIME->date().daysTo(remTime.date())) + ":" + output;
    //qDebug("%s", output.toStdString().c_str());
    this->clock->setText(QString("Time Left in Segment:\n").append(output));
    //Set the tooltip for the tray icon.
    this->UpdateTrayTooltip();
}
//Updates the information displayed on the tooltip
void PomodoroUI::UpdateTrayTooltip(){
    //Set the tooltip for the tray icon.
    QString ttip = this->toggle->text();
    ttip.append((this->clock->text()).append("\n").prepend("\n"));
    QString cycle_info;
    cycle_info.append("Current Pomodoro: " + this->cycle->get_c_pom_str());
    cycle_info.append("\nCurrent Cycle: " + this->cycle->get_c_cycle_str());
    this->pc_status->setText(cycle_info);
    ttip.append(cycle_info);
    this->tray->setToolTip(ttip);
    //loop_timer->start(this->LEN_LOOP);
}
//Sends notification to the session if we are notifying.
void PomodoroUI::notify_session(int status){
    //Retrieve the notification information from the PomodoroTimer object, and send it with the tray object.
    this->tray->showMessage(this->cycle->getMessageTitle(status), this->cycle->getMessageBody(status));
}
//Called when pause button is pressed.
void PomodoroUI::toggle_pressed(){
    this->cycle->toggleTimer();
}
//Right click = QSystemTrayIcon::Context = 1
void PomodoroUI::window_toggle(QSystemTrayIcon::ActivationReason reason){
    if(reason != 1)
        update_visible();
}
//Re-implementation of close event. Used to minimize to tray when closing but not quitting.
void PomodoroUI::closeEvent(QCloseEvent *event){
    if(event->spontaneous() && this->tray->isVisible()){
        this->update_visible();
        if(this->notify && !this->warned_tray){
            this->tray->showMessage("Minimzed to tray","Right click on icon and select exit to close");
            this->warned_tray = true;
        }
        event->ignore();
    }
}
//Updates the tray menu's Show/Hide Window option appropriatly when the window is hidden/unhidden.
void PomodoroUI::update_visible(){
    if (this->isVisible()){
        this->tray_menu_items->at(0)->setText(tr("&Show Window"));
        this->hide();
    }
    else{
        this->tray_menu_items->at(0)->setText(tr("Hi&de Window"));
        this->show();
    }
}
//Called to prepare enviornment and then opens the config.
void PomodoroUI::start_config(){
    this->cycle->pauseTimer();
    //Hide the tray icon to avoid timer tampering.
    this->tray->hide();
    this->config->setup();
}
//Called when config window is closed.
void PomodoroUI::finish_config(){
    this->cycle->resumeTimer();
    this->tray->show();
}
//Slot used to quit the program from a menu option.
void PomodoroUI::quitting(){
    qInfo("%s", "Exitting...");
    QCoreApplication::quit();
}
//Called to prompt user to name a new preset from the current settings.
void PomodoroUI::settings_to_preset(QAction * entry){
    QJsonObject curr_settings;
    if (entry->text() == tr("From &Current Settings..."))
        curr_settings = this->cycle->getPresetJson("");
    else if (entry->text() == tr("From &Default Settings..."))
        curr_settings = *(this->preset_manager->getDefaultPreset());
    else
        curr_settings = *(this->preset_manager->getPreset(entry->text()));
    try{
        PresetManager::validate_preset(&curr_settings);
    }
    catch (PresetManager::preset_error &err){
        qCritical("%s", err.what());
        return;
    }
    bool ok;
    QString text;
    do{
        text = QInputDialog::getText(this, tr("Create New Preset"),
                                         tr("Preset Name:"), QLineEdit::Normal,
                                         "", &ok);
        if (!ok)
            break;
        ok = !(text.trimmed().isEmpty());
        if(ok){
            curr_settings["preset_name"] = text;
            if (!this->preset_manager->create_preset(curr_settings)){
                this->prompt_confirmation("Preset " + text.trimmed() + " exists.", "Overwrite it?", ok);
                if (ok)
                    this->preset_manager->create_preset(curr_settings, true);
            }
        }
    } while (ok == false);
    if(ok)
        this->preset_manager->writePresetFile();
}
//Called to prompt the user to give a new name for an existing preset.
void PomodoroUI::rename_preset(QAction* event){
    int i = this->preset_manager->findPreset(event->text().trimmed());
    if (i < 0)
        return;
    const QJsonObject* old_settings = this->preset_manager->getPreset(i);
    QJsonObject curr_settings(*old_settings);
    bool ok;
    QString text;
    QString label = "New Preset Name:";
    QString base_label = label;
    do{
        text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                     label, QLineEdit::Normal,
                                     curr_settings["preset_name"].toString(), &ok);
        if (!ok){ //Exits if user canceled.
            break;
        }
        ok = !(text.trimmed().isEmpty());
        if (ok && text.trimmed() == (*old_settings)["preset_name"].toString()){
            ok = false;
            label = base_label + "\n(Please input a NEW name.)";
        }
        else{
            label = base_label;
            if (ok){
                curr_settings["preset_name"] = text.trimmed();
                if (!this->preset_manager->rename_preset(i, curr_settings, false)){
                    this->prompt_confirmation("Preset " + text.trimmed() + " exists.", "Overwrite it?", ok);
                    if (ok)
                        this->preset_manager->rename_preset(i, curr_settings, true);
                }
            }
        }
    } while (ok == false);
    if (ok){
        this->preset_manager->writePresetFile();
    }
    delete old_settings;
}
//Is the QAction::menu() unique per sender? Yes, but not relevant here.
//These functions are called when a preset is added/remove, and adds/removes the appropriate menu option.
void PomodoroUI::preset_added(QAction *set[6]){
    for (int i = 0; i < 6; i++)
        this->preset_menus[i]->addAction(set[i]);
}

void PomodoroUI::preset_removed(QAction *set[6]){
    for (int i = 0; i < 6; i++)
        this->preset_menus[i]->removeAction(set[i]);
}
//Attempts to load the preset specified by the user in a menu.
//Checks if the default preset was selected by comparing the triggered action to the known default entry in the menu.
//If the preset doesn't exist, which should never be the case, nothing happens.
void PomodoroUI::attempt_preset_load(QAction* entry){
    QList<QAction*> entryList = this->preset_menus[0]->actions();
    try{
        if(entry == entryList[0]){
            qDebug("Loading default preset...");
            if (this->preset_manager->load_default_preset())
                this->cycle->ResetSession();
            return;
        }
        qDebug("%s %s", "Loading preset", entry->text().trimmed().toStdString().c_str());
        if(this->preset_manager->loadPreset(entry->text().trimmed()))
            this->cycle->ResetSession();
    }
    catch (PresetManager::preset_error &ex){
        qCritical("%s",ex.what());
    }
}
//Attempts to remove the preset specified by the user in a menu.
//If the preset doesn't exist, which should never be the case, nothing happens.
void PomodoroUI::attempt_preset_remove(QAction* entry){
    if (!(this->preset_manager->removePreset(entry->text().trimmed()))){
        qCritical("%s", QString("Failed to remove entry " + entry->text().trimmed()).toStdString().c_str());
    }
}

//Starts the preset editor with the user selected preset. Does not stop timer while doing this.
//If the preset doesn't exist, which should never be the case, nothing happens.
void PomodoroUI::attempt_preset_edit(QAction* entry){
    QList<QAction*> entryList = this->preset_menus[2]->actions();
    PresetEditor* editor;
    try{
        editor = new PresetEditor(this->cycle, entry->text().trimmed(), this->preset_manager, this, entry == entryList.first());
        connect(editor, &PresetEditor::request_overwrite, this, &PomodoroUI::prompt_confirmation);
        editor->exec();
        editor->deleteLater();
    }
    catch (PresetManager::preset_error &ex){
        qCritical("%s", ex.what());
    }
}
//Copies the selected preset to the default entry.
//If the preset doesn't exist, which should never be the case, nothing happens.
void PomodoroUI::attempt_update_default(QAction* entry){
	try{
        if (!(this->preset_manager->update_default_preset(entry->text().trimmed())))
            qDebug("Default Preset was not updated, but appeared to be valid.");
	}
	catch (PresetManager::preset_error &ex){
        qCritical("%s%s", ex.what(), ". The Default preset was not updated.");
	}
}
//Creates a dialog titled <Title> with the Message <Message>.
//They are given two options, <accept> or <reject>. The result is saved to <ok>.
void PomodoroUI::prompt_confirmation(QString Title, QString Message, bool &result, QString accept, QString reject){
    //Set up the vars and initialize the box.
    QMessageBox confirm;
    confirm.setText(Title);
    confirm.setInformativeText(Message);
    QPushButton* button_accept = confirm.addButton(accept, QMessageBox::YesRole);
    QPushButton* button_reject = confirm.addButton(reject, QMessageBox::NoRole);
    confirm.exec();
    //"Return" true if the accept button was hit, false otherwise.
    result = confirm.clickedButton() == button_accept;
}
//Displays the program's help.
void PomodoroUI::retrieve_help(){
    help_browser::load_help(QString("PomodoroUI"), this);
}
