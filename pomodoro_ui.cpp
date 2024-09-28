/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 * TODO: Create Notifier object to handle notifications.
 */
#include "pomodoro_ui.h"
//#include "ui_pomodoro_ui.h"
PomodoroUI::PomodoroUI(PresetManager* preset_manager, QJsonObject* starting_preset, QWidget *parent): QWidget(parent){
    //Set the window title
    this->setWindowTitle("Pomodoro Status");
//Timer Setup
    //Event loops
    //this->main_timer = new QTimer(this);
    this->loop_timer = new QTimer(this);
    //Setup the preset manager object.
    this->preset_manager = preset_manager;
    this->preset_manager->setParent(this);
    //Initialize the Pomodoro Cycle & set log_stdout and notify appropriatly.
    try{
        this->cycle = new PomodoroTimer(starting_preset, this);
    }
    catch (PresetManager::preset_error &ex){
        delete this->cycle;
        qCritical("%s %s", ex.what(), "Reverting to default settings...");
        int units[3] = {1,1,1};
        this->cycle = new PomodoroTimer(25.0, 5.0, 15.0, units, 2, 4, true, this);
    }
//Main Window Visual Elements + Layout
    this->toggle = new QPushButton("Start");
    this->clock = new QLabel("");
    this->pc_status = new QLabel("");
    layout = new QGridLayout(this);
    layout->addWidget(toggle, 1, 0, 1, 2);
    layout->addWidget(clock, 2, 0); //-1 should put this on the right side of the window.
    layout->addWidget(pc_status, 2, 1);
    this->notify = true;
    this->config = new TimerConfig(this->cycle);
    //System Tray Visual Elements
    this->tray = new QSystemTrayIcon(*(this->preset_manager->construct_tray_icon(0, this->cycle->get_icon_name(0))), this);
    this->tray->show(); //Reveal the icon in the system tray.
    this->tray_menu_items = new QList<QAction*>();
    //Configure Menus
    //Configure Preset Options
    QMenu* presetMenu = new QMenu(tr("Presets"));
    QMenu* create_preset = presetMenu->addMenu(tr("&Create Preset")); //The Create Preset Submenu
    this->preset_menus[0] = presetMenu->addMenu(tr("&Load Preset"));
    this->preset_menus[1] = presetMenu->addMenu(tr("&Remove Preset"));
    this->preset_menus[2] = presetMenu->addMenu(tr("&Edit Preset"));
    this->preset_menus[3] = presetMenu->addMenu(tr("Re&name Preset"));
    this->preset_menus[4] = presetMenu->addMenu(tr("Co&py Preset to Default Settings"));
    //Configure Preset Create Preset submenu.
    this->preset_menus[5] = create_preset->addMenu(tr("From &Existing Preset"));
    QAction* save_current = new QAction(tr("From &Current Settings..."));
    create_preset->addAction(save_current);
    QAction* save_default = new QAction(tr("From &Default Settings..."));
    create_preset->addAction(save_default);
    QAction* from_scratch = new QAction(tr("From &Scratch..."));
    create_preset->addAction(from_scratch);
    this->preset_manager->populate_preset_menu_entries(this->preset_menus);
    this->SetupMenus();
    //Main Window Context Menu
    this->top_bar = new QMenuBar(this);
    this->top_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QMenu* timer = this->top_bar->addMenu("Timer");
    timer->addAction(this->tray_menu_items->at(1)); //Pause from tray menu
    timer->addAction(this->tray_menu_items->at(2)); //Skip Pomodoro
    timer->addAction(this->tray_menu_items->at(3)); //Reset Segment
    timer->addAction(this->tray_menu_items->at(4)); //Reset Session
    timer->addSeparator();
    QAction* edit = new QAction(tr("&Edit Timer..."), this);
    timer->addAction(edit); //Edit Paramaters...
    this->top_bar->addMenu(presetMenu);
    QMenu* windowMenu = this->top_bar->addMenu("Window");
    windowMenu->addAction(this->tray_menu_items->at(0));
    QAction* help = new QAction(tr("&Help..."), this);
    windowMenu->addAction(help);
    windowMenu->addAction(this->tray_menu_items->at(5)); //Exit
    this->top_bar->setNativeMenuBar(true);
    this->layout->setMenuBar(this->top_bar);
    //Add the task manager.
    this->layout->addWidget(todo_list::get_todo_list(), 3, 0, 3, -1);
//Signal and Slot Connections
    //Setup connections activated when the timer is started/stopped.
    connect(this->toggle, SIGNAL (clicked()), this, SLOT (toggle_pressed()));
    connect(cycle, &PomodoroTimer::timer_toggled, this, &PomodoroUI::toggled);
    //Add connections activated when the event loop or the main timer expires.
    connect(loop_timer, &QTimer::timeout, this, &PomodoroUI::update_timer_display);
    connect(this->cycle, SIGNAL (segment_changed(int)), this, SLOT (update_segment(int)));
    connect(this->cycle, SIGNAL (segment_changed(int)), todo_list::get_todo_list(), SLOT (pomodoro_complete(int)));
    if (notify){ //Only connect these sockets if notifications are enabled. After segment_changed for ordering purposes.
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
    //Skip current Pomodoro [2]
    QAction* skip_pomodoro = new QAction(tr("S&kip Pomodoro"));
    connect(skip_pomodoro, &QAction::triggered, this->cycle, &PomodoroTimer::skip_pomodoro);
    this->tray_menu_items->append(skip_pomodoro);
    tray_actions->addAction(skip_pomodoro);
    //Restart Segment [3]
    QAction* segment_restart = new QAction(tr("Restart Se&gment"), this);
    connect(segment_restart, &QAction::triggered, this->cycle, &PomodoroTimer::ResetSegment);
    this->tray_menu_items->append(segment_restart);
    tray_actions->addAction(segment_restart);
    //Restart Session [4]
    QAction* session_restart = new QAction(tr("Restart &Session"), this);
    connect(session_restart, &QAction::triggered, this->cycle, &PomodoroTimer::initCycle);
    this->tray_menu_items->append(session_restart);
    tray_actions->addAction(session_restart);
    tray_actions->addSeparator();
    //Load Preset [NOT IN ARRAY]. Simply added to the tray's context menu.
    tray_actions->addMenu(this->preset_menus[0]);
    tray_actions->addSeparator();
    //Exit Program [5]
    QAction* exit_program = new QAction(tr("&Exit"), this);
    connect(exit_program, &QAction::triggered, this, &PomodoroUI::quitting);
    this->tray_menu_items->append(exit_program);
    tray_actions->addAction(exit_program);
    //Timer Display [6], but at start of tray_actions.
    QAction* timer_display = new QAction(this);
    timer_display->setDisabled(true);
    this->tray_menu_items->append(timer_display);
    tray_actions->insertAction(this->tray_menu_items->at(0), timer_display);
    //tray_actions->addActions(*(this->tray_menu_items));
    this->tray->setContextMenu(tray_actions);
}

//Destructor that destroys the timer and preset manager objects.
PomodoroUI::~PomodoroUI()
{
    delete this->cycle;
    delete this->preset_manager;
}

//These functions handle pausing/resuming.
//Called when pause button is pressed.
void PomodoroUI::toggle_pressed(){
    this->cycle->toggleTimer();
}

//Toggle the button's text and stop/start and the timer that updates the countdown.
//bool paused: If the timer was paused BEFORE the button was pressed, set to true.
void PomodoroUI::toggled(bool paused){
    QString time = this->tray_menu_items->at(6)->text();
    if(paused){
        time.replace(this->status[3], this->status[2]);
        this->toggle->setText(this->status[0]);
        this->tray_menu_items->at(6)->setText(time);
        this->tray_menu_items->at(1)->setText(tr("&Pause Timer"));
        loop_timer->start(this->LEN_LOOP);
    }
    else{
        time.replace(this->status[2], this->status[3]);
        this->toggle->setText(this->status[1]);
        this->tray_menu_items->at(1)->setText(tr("&Resume Timer"));
        this->tray_menu_items->at(6)->setText(time);
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
    if(status == 0 || status == 5)
        this->loop_timer->start(this->LEN_LOOP); //Restart the loop timer if not running.
    if (status == 0 || status == 3 || status == 5){
        this->status[0] = "Studying";
        this->status[1] = "[Paused] Studying";
        this->status[2] = "Studying: ";
        this->status[3] = "Studying [⏸]: ";
        //this->tray->setIcon(this->study_icon);
        this->tray_menu_items->at(1)->setText(tr("&Pause Timer"));
    }
    else if (status == 4){
        this->status[0] = this->status[1] = "Restart";
        this->status[2] = this->status[3] = "Complete";
        this->loop_timer->stop();
    }
    else if (status == 1){
        this->status[0] = "On Short Break";
        this->status[1] = "[Paused] On Short Break";
        this->status[2] = "S. Break: ";
        this->status[3] = "S. Break [⏸]: ";
        //this->tray->setIcon(this->breaktime_icon);
    }
    else{
        this->status[0] = "On Long Break";
        this->status[1] = "[Paused] On Long Break";
        this->status[2] = "L. Break: ";
        this->status[3] = "L. Break [⏸]: ";
        //this->tray->setIcon(this->breaktime_icon);
    }
    const QIcon* updated_icon = this->preset_manager->construct_tray_icon(this->cycle->getCurrentSegment(), this->cycle->get_icon_name(this->cycle->getCurrentSegment()));
    if (updated_icon != nullptr)
        this->tray->setIcon(*updated_icon);
    this->toggle->setText(QString(this->status[0]));
    this->UpdateTrayTooltip();
}

//Called every second to update the timer's display.
void PomodoroUI::update_timer_display(){
    QString output = this->cycle->getTimeRemaining();
    //qDebug("%s", output.toStdString().c_str());
    if(!output.isEmpty()){
        this->clock->setText(QString("Time Left in Segment:\n").append(output));
        //Set the tooltip for the tray icon.
        this->tray_menu_items->at(6)->setText(this->status[2] + output);
    }
    this->UpdateTrayTooltip();
    const QIcon* updated_icon = this->preset_manager->construct_tray_icon(this->cycle->getCurrentSegment(), this->cycle->get_icon_name(this->cycle->getCurrentSegment()), this->cycle->getPercentElapsed());
    if(updated_icon != nullptr)
        this->tray->setIcon(*updated_icon);
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
    if (this->tray->toolTip() != ttip)
        this->tray->setToolTip(ttip);
}

//Sends notification to the session if we are notifying.
void PomodoroUI::notify_session(int status){
    //Retrieve the notification information from the PomodoroTimer object, and send it with the tray object.
    this->tray->showMessage(this->cycle->getMessageTitle(status), this->cycle->getMessageBody(status), this->tray->icon());
}

//Disables all tray menu actions and gets it to block signals. Used to prevent interference with dialog windows.
void PomodoroUI::set_tray_enabled(bool enabled){
    QList<QAction *> actions = this->tray->contextMenu()->actions();
    for(QList<QAction *>::iterator i = actions.begin(); i != actions.end(); i++)
        (*i)->setEnabled(enabled);
    this->tray->blockSignals(!enabled);
}

//Functions for timer configuration.
//Called to prepare enviornment and then opens the config.
void PomodoroUI::start_config(){
    this->cycle->pauseTimer();
    //Disable the tray to prevent tampering whist editing the timer.
    this->set_tray_enabled(false);
    this->config->setup();
}

//Called when config window is closed.
void PomodoroUI::finish_config(){
    this->cycle->resumeTimer();
    //Editing is done. Reenable the tray.
    this->set_tray_enabled(true);
}

//Slot used to quit the program from a menu option.
void PomodoroUI::quitting(){
    qInfo("%s", "Exitting...");
    QCoreApplication::quit();
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

//Called to prompt user to name and configure a new preset from some pre-exsisting settings.
void PomodoroUI::settings_to_preset(QAction * entry){
    QJsonObject* curr_settings;
    if (entry->text() == tr("From &Current Settings..."))
        curr_settings = new QJsonObject(this->cycle->getPresetJson());
    else if (entry->text() == tr("From &Default Settings..."))
        curr_settings = this->preset_manager->getDefaultPreset();
    else if (entry->text() == tr("From &Scratch..."))
        curr_settings = new QJsonObject(*PresetManager::DEFAULT_PRESET);
    else
        curr_settings = this->preset_manager->getPreset(entry->text());
    curr_settings->insert(QStringLiteral("preset_name"), QStringLiteral(""));
    try{
        PresetEditor* editor = new PresetEditor(this->cycle, curr_settings, this->preset_manager, this);
        connect(editor, &PresetEditor::request_overwrite, this, &PomodoroUI::prompt_confirmation);
        editor->exec();
        editor->deleteLater();
    }
    catch (PresetManager::preset_error &ex){
        qCritical("%s", ex.what());
    }
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
    QString base_label = "New Preset Name:";
    QString label = base_label;
    this->set_tray_enabled(false);
    do{
        text = QInputDialog::getText(this, QStringLiteral("Rename Preset"),
                                     label, QLineEdit::Normal,
                                     curr_settings["preset_name"].toString(), &ok);
        if (!ok){ //Exits if user canceled.
            break;
        }
        if (text.trimmed() == old_settings->value("preset_name").toString()){
            ok = false;
            label = base_label + "\n(Please input a NEW name.)";
            continue;
        }
        label = base_label;
        if ((ok = !(text.trimmed().isEmpty()))){
            curr_settings["preset_name"] = text.trimmed();
            if (!this->preset_manager->rename_preset(i, curr_settings, false)){
                this->prompt_confirmation("Preset " + text.trimmed() + " exists.", "Overwrite it?", ok);
                if (ok)
                    this->preset_manager->rename_preset(i, curr_settings, true);
            }
        }
    } while (ok == false);
    if (ok){
        this->preset_manager->writePresetFile();
    }
    delete old_settings;
    this->set_tray_enabled(true);
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
        editor = PresetEditor::start_edit(this->cycle, entry->text().trimmed(), this->preset_manager, this, entry == entryList.first());
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
    confirm.addButton(reject, QMessageBox::NoRole);
    confirm.exec();
    //"Return" true if the accept button was hit, false otherwise.
    result = confirm.clickedButton() == button_accept;
}

//These functions handle window events that result in it being hidden/shown.
//Right click = QSystemTrayIcon::Context = 1
void PomodoroUI::window_toggle(QSystemTrayIcon::ActivationReason reason){
    if(reason != 1)
        (this->isHidden() || this->isActiveWindow()) ? this->update_visible() : ((this->isMinimized()) ? this->setWindowState(this->windowState() & (~Qt::WindowMinimized | Qt::WindowActive)) : this->activateWindow());
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


//Displays the program's help.
void PomodoroUI::retrieve_help(){
    help_browser::load_help(QStringLiteral("PomodoroUI"));
}
