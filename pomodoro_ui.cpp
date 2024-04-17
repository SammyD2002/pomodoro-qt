#include "pomodoro_ui.h"
//#include "ui_pomodoro_ui.h"
//Define the help pointer.

//why
PomodoroUI::PomodoroUI(QWidget *parent): QWidget(parent){
    this->preset_manager = new PresetManager(QString(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)), this);
    //Main Window Visual Elements + Layout
    this->toggle = new QPushButton("Start");
    this->main_timer = new QTimer(this);
    this->loop_timer = new QTimer(this);
    this->clock = new QLabel("");
    this->pc_status = new QLabel("");
    layout = new QGridLayout(this);
    //layout->SetMinimumSize(2,2);
    layout->addWidget(toggle, 1, 0, 1, 2);
    layout->addWidget(clock, 2, 0); //-1 should put this on the right side of the window.
    layout->addWidget(pc_status, 2, 1);

    //Initialize the Pomodoro Cycle & set log_stdout and notify appropriatly.
    this->log_stdout = false;
    this->notify = true;
    try{
        this->cycle = new PomodoroTimer(main_timer, this->preset_manager->getDefaultPreset(), log_stdout, this);
    }
    catch (std::invalid_argument &ex){
        delete this->cycle;
        std::cout << "Json was not parsed. Reverting to default settings..." << std::endl;
        int units[3] = {1,1,1};
        this->cycle = new PomodoroTimer(main_timer, 25.0, 5.0, 15.0, units, 2, 4, log_stdout, true, this);
    }
    this->config = new TimerConfig(this->cycle);

    //System Tray Visual Elements
    //this->study_icon = QIcon ("/vol/sharedstorage/Software/Projects/Personal/qt/pomodoro/icons/book.svg");
    //this->breaktime_icon = QIcon ("/vol/sharedstorage/Software/Projects/Personal/qt/pomodoro/icons/smiley.svg");
    this->study_icon = QIcon (QPixmap(QString::fromStdString(":icons/book.svg")));
    this->breaktime_icon = QIcon (QPixmap(QString::fromStdString(":icons/smiley.svg")));
    this->tray = new QSystemTrayIcon(study_icon, this);
    this->tray->show(); //Reveal the icon in the system tray.
    this->tray_menu_items = new QList<QAction*>();
    //Configure Menus
    this->SetupMenus();
    //Main Window Context Menu
    this->top_bar = new QMenuBar(this);
    this->top_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //this->layout->addWidget(top_bar, 0, 0);
    QMenu* timer = this->top_bar->addMenu("Timer");
    timer->addAction(this->tray_menu_items->at(1)); //Pause from tray menu
    timer->addAction(this->tray_menu_items->at(2)); //Reset Segment
    timer->addAction(this->tray_menu_items->at(3)); //Reset Session
    timer->addSeparator();
    QAction* edit = new QAction(tr("&Edit Timer..."), this);
    timer->addAction(edit); //Edit Paramaters...
    QMenu* presetMenu = this->top_bar->addMenu("Presets");
    QAction* save_current = new QAction(tr("&Save Current Settings..."), this);
    this->load_preset_menu = presetMenu->addMenu(tr("&Load Preset"));
    this->del_preset_menu = presetMenu->addMenu(tr("&Remove Preset"));
    this->edit_preset_menu = presetMenu->addMenu(tr("&Edit Preset"));
    this->rename_preset_menu = presetMenu->addMenu(tr("Re&name Preset"));
    this->new_default_preset_menu = presetMenu->addMenu(tr("&Copy Preset to Default Settings"));
    this->preset_manager->populate_preset_menu_entries(load_preset_menu, del_preset_menu, edit_preset_menu, rename_preset_menu, new_default_preset_menu);
    presetMenu->addAction(save_current);
    QMenu* windowMenu = this->top_bar->addMenu("Window");
    windowMenu->addAction(this->tray_menu_items->at(0));
    QAction* help = new QAction(tr("&Help..."), this);
    windowMenu->addAction(help);
    windowMenu->addAction(this->tray_menu_items->at(4));
    //QMenu presets = this->top_bar->addMenu("Presets");
    //QMenu window = this->top_bar->addMenu("Window");
    this->top_bar->setNativeMenuBar(true);
    this->layout->setMenuBar(this->top_bar);
    //this->resize(layout::size);

    //Signal and Slot Connections
    connect(edit, &QAction::triggered, this, &PomodoroUI::start_config);
    connect(this->config, &TimerConfig::config_complete, this, &PomodoroUI::finish_config);
    connect(this->toggle, SIGNAL (clicked()), this, SLOT (toggle_pressed()));
    connect(this->cycle, SIGNAL (segment_changed(int)), this, SLOT (update_segment(int)));
    connect(cycle, &PomodoroTimer::timer_toggled, this, &PomodoroUI::toggled);
    if (notify){ //Only connect these sockets if notifications are enabled.
        connect(cycle, &PomodoroTimer::segment_changed, this, &PomodoroUI::notify_session);
    }
    //Connect various preset menu methods:
    connect(this->load_preset_menu, &QMenu::triggered, this, &PomodoroUI::attempt_preset_load, Qt::UniqueConnection);
    connect(save_current, &QAction::triggered, this, &PomodoroUI::settings_to_preset);
    connect(this->del_preset_menu, &QMenu::triggered, this, &PomodoroUI::attempt_preset_remove, Qt::UniqueConnection);
    connect(this->edit_preset_menu, &QMenu::triggered, this, &PomodoroUI::attempt_preset_edit, Qt::UniqueConnection);
    connect(this->preset_manager, &PresetManager::presetLoaded, this->cycle, &PomodoroTimer::applyPreset);
    connect(this->rename_preset_menu, &QMenu::triggered, this, &PomodoroUI::rename_preset);
    connect(this->new_default_preset_menu, &QMenu::triggered, this, &PomodoroUI::attempt_update_default);

    //Connect methods called for presets being added/removed. Qt::UniqueConnection makes it so one menu doesn't trigger the other.
    connect(this->preset_manager, &PresetManager::preset_added, this, &PomodoroUI::preset_added, Qt::UniqueConnection);
    connect(this->preset_manager, &PresetManager::preset_removed, this, &PomodoroUI::preset_removed, Qt::UniqueConnection);

    //System tray activation signal handler.
    connect(this->tray, &QSystemTrayIcon::activated, this, &PomodoroUI::window_toggle);
    connect(loop_timer, &QTimer::timeout, this, &PomodoroUI::update_timer_display);
    //Connect le help menu
    connect(help, &QAction::triggered, this, &PomodoroUI::retrieve_help);


    //Connect editor configs:
    //this->connectConfigSignals();
    //Start the main & event loop timers.
    this->cycle->initCycle(false);
    //loop_timer->start(1000);
}
/*
PomodoroUI::PomodoroUI(bool log_stdout): PomodoroUI::PomodoroUI(nullptr) {
    this->log_stdout = log_stdout;
    this->cycle->toggle_log_stdout(log_stdout);
}*/
PomodoroUI::~PomodoroUI()
{
    delete this->cycle;
    delete this->preset_manager;
}

void PomodoroUI::retrieve_help(){
    help_browser::load_help(QString("PomodoroUI"), this);
}

/*
void PomodoroUI::connectConfigSignals(){
    connect(this->config, &TimerConfig::study_updated, this, &PomodoroUI::update_study);
    connect(this->config, &TimerConfig::break_s_updated, this, &PomodoroUI::update_break_short);
    connect(this->config, &TimerConfig::break_l_updated, this, &PomodoroUI::update_break_long);
    connect(this->config, &TimerConfig::m_pomodoros_updated, this, &PomodoroUI::update_max_pomodoros);
    connect(this->config, &TimerConfig::m_cycles_updated, this, &PomodoroUI::update_max_cycles);
}
*/
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
    QAction* segment_restart = new QAction(tr("&Restart Segment"), this);
    connect(segment_restart, &QAction::triggered, this->cycle, &PomodoroTimer::ResetSegment);
    this->tray_menu_items->append(segment_restart);
    tray_actions->addAction(segment_restart);
    //Restart Session [3]
    QAction* session_restart = new QAction(tr("&Restart Session"), this);
    connect(session_restart, &QAction::triggered, this->cycle, &PomodoroTimer::initCycle);
    this->tray_menu_items->append(session_restart);
    tray_actions->addAction(session_restart);
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

void PomodoroUI::toggle_pressed(){
    this->cycle->toggleTimer();
}

void PomodoroUI::toggled(bool paused){
    if(paused){
        this->toggle->setText(QString::fromStdString(this->status[0]));
        this->tray_menu_items->at(1)->setText("Pause Timer");
        loop_timer->start(1000);
    }
    else{
        this->toggle->setText(QString::fromStdString(this->status[1]));
        this->tray_menu_items->at(1)->setText("Resume Timer");
        loop_timer->stop();
    }
    //Reset the tray icon tooltip appropriatly.
    this->UpdateTrayTooltip();

}
//Slots:

void PomodoroUI::notify_session(int status){
    //Find and send the applicable notification.
    //QString noteTitle;
    //QString noteMessage;
    //QIcon *icon;
    //QSystemTrayIcon::Information;
    //int timeout = 100000
    /*switch(status){
    //Initial startup
        case 0:
            noteTitle = QString::fromStdString("Starting Study Session");
            noteMessage = QString::fromStdString("Good luck!");
            break;
        case 1:
            noteTitle = QString::fromStdString("Study Segment Complete");
            noteMessage = QString::fromStdString(("Nice job out there. You have completed " + cycle->get_c_pom_str() + " pomodoros.\nEnjoy your short break!"));
            break;
        case 2:
            noteTitle = QString::fromStdString("Study Cycle Complete");
            noteMessage = QString::fromStdString(("Congratulations! You have completed " + cycle->get_c_pom_str() + " pomodoros, and have earned your self a long break!"));
            break;
        case 3:
            noteTitle = QString::fromStdString("Short Break Complete");
            noteMessage = QString::fromStdString("Hope you enjoyed the break! Now, GET BACK TO WORK!");
            break;
        case 4:
            noteTitle = QString::fromStdString("Study Session Co1mplete");
            noteMessage = QString::fromStdString("Congratulations! Hope you got a lot done!");
            break;
        case 5:
            noteTitle = QString::fromStdString("Restarting Study Session");
            noteMessage = QString::fromStdString("Time to get some more work done!");
            break;
        };*/

    this->tray->showMessage(this->cycle->getMessageTitle(status), this->cycle->getMessageBody(status));
}
//The integer recieved signals what state we transferred to.
//0 = Session started, 1 = study -> short break, 2 = study -> long break,
//3 = x break -> study, 4 = session over, 5 = session restart.
void PomodoroUI::update_segment(int status){
    //Update various icons.
    if (this->log_stdout)
        std::cout << "Updating segment..." << std::endl;
    if(status == 0 || status || 5)
        this->loop_timer->start(1000); //Restart the loop timer if not running.
    if (status == 0 || status == 3 || status == 5){
        this->status[0] = "Studying";
        this->status[1] = "[Paused] Studying";
        this->tray->setIcon(this->study_icon);
        this->tray_menu_items->at(1)->setText("Pause Timer");
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
void PomodoroUI::update_timer_display(){
    QDateTime remTime = this->cycle->ZERO_TIME->addMSecs((this->main_timer->remainingTime() / 1000) * 1000);
    QString output = remTime.toString("hh:mm:ss");
    if (this->cycle->ZERO_TIME->date().daysTo(remTime.date()) >= 1)
        output = QString::number(this->cycle->ZERO_TIME->date().daysTo(remTime.date())) + ":" + output;
    if(this->log_stdout)
        std::cout << qPrintable(output) << std::endl;
    this->clock->setText(QString("Time Left in Segment:\n").append(output));
    //Set the tooltip for the tray icon.
    this->UpdateTrayTooltip();
}

//Right click = QSystemTrayIcon::Context = 1
void PomodoroUI::window_toggle(QSystemTrayIcon::ActivationReason reason){
    if(reason != 1)
        update_visible();
}

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
    //loop_timer->start(1000);
}

void PomodoroUI::update_visible(){
    if (this->isVisible()){
        this->tray_menu_items->at(0)->setText("Show Window");
        this->hide();
    }
    else{
        this->tray_menu_items->at(0)->setText("Hide Window");
        this->show();
    }
}

void PomodoroUI::quitting(){
    QCoreApplication::quit();
}

//Called to prepare enviornment and then opens the config.
void PomodoroUI::start_config(){
    this->cycle->pauseTimer();
    //Hide the tray icon to avoid timer tampering.
    this->tray->hide();
    this->config->setup();
}

//Re-implementation of close event:
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
/*
void PomodoroUI::update_study(int new_val){
    this->cycle->adjustSegment(1, new_val);
}
void PomodoroUI::update_break_short(int new_val){
    this->cycle->adjustSegment(2, new_val);
}
void PomodoroUI::update_break_long(int new_val){
    this->cycle->adjustSegment(3, new_val);
}
void PomodoroUI::update_max_pomodoros(int new_val){
    this->cycle->adjustSegment(4, new_val);
}
void PomodoroUI::update_max_cycles(int new_val){
    this->cycle->adjustSegment(5, new_val);
}

void PomodoroUI::update_cycle_limit(bool enabled){
    this->cycle->adjustSegment(6, !enabled);
}
*/

//Called to prompt user for the new preset's name.
void PomodoroUI::settings_to_preset(){
    QJsonObject curr_settings = this->cycle->getPresetJson("");
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
            if (!this->preset_manager->update_preset(curr_settings)){
                this->prompt_confirmation("Preset " + text.trimmed() + " exists.", "Overwrite it?", ok);
                if (ok)
                    this->preset_manager->update_preset(curr_settings, true);
            }
        }

    } while (ok == false);
    if(ok)
        this->preset_manager->writePresetFile();
}

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

//Called when config window is closed.
void PomodoroUI::finish_config(){
    this->cycle->resumeTimer();
    this->tray->show();
}
//Is the QAction::menu() unique per sender?
//Public Slots:
void PomodoroUI::preset_added(QAction* load, QAction* del, QAction* edit, QAction* ren, QAction* def){
    this->load_preset_menu->addAction(load);
    this->del_preset_menu->addAction(del);
    this->edit_preset_menu->addAction(edit);
    this->rename_preset_menu->addAction(ren);
    this->new_default_preset_menu->addAction(def);
}
void PomodoroUI::preset_removed(QAction* load, QAction* del, QAction* edit, QAction* ren, QAction* def){
    this->load_preset_menu->removeAction(load);
    this->del_preset_menu->removeAction(del);
    this->edit_preset_menu->removeAction(edit);
    this->rename_preset_menu->removeAction(ren);
    this->new_default_preset_menu->removeAction(def);
}

void PomodoroUI::attempt_preset_load(QAction* entry){
    QList<QAction*> entryList = this->load_preset_menu->actions();
    if(entry == entryList[0]){
        if (this->preset_manager->load_default_preset())
            this->cycle->ResetSession();
        return;
    }

    if(this->preset_manager->loadPreset(entry->text().trimmed()))
        this->cycle->ResetSession();
}
void PomodoroUI::attempt_preset_remove(QAction* entry){
    if (!(this->preset_manager->removePreset(entry->text().trimmed()))){
        if(this->log_stdout)
            std::cout << "Failed to remove entry " << qPrintable(entry->text().trimmed()) << std::endl;
    }
}

void PomodoroUI::attempt_preset_edit(QAction* entry){
    PresetEditor* editor = new PresetEditor(this->cycle, entry->text().trimmed(), this->preset_manager, this);
    connect(editor, &PresetEditor::request_overwrite, this, &PomodoroUI::prompt_confirmation);
    editor->exec();
    delete editor;
}
void PomodoroUI::attempt_update_default(QAction* entry){
    if (!(this->preset_manager->update_default_preset(entry->text().trimmed())))
        std::cout << "Default Preset was not updated." <<  std::endl;

}


