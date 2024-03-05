#include "pomodoro_ui.h"
//#include "ui_pomodoro_ui.h"

PomodoroUI::PomodoroUI(QWidget *parent, bool notify, bool log_stdout): QWidget(parent){
    //Main Window Visual Elements + Layout
    this->toggle = new QPushButton("Start");
    this->main_timer = new QTimer(this);
    this->clock = new QLabel("");
    layout = new QGridLayout(this);
    layout->addWidget(toggle, 1, 0);
    layout->addWidget(clock, 2, 0); //-1 should put this on the right side of the window.

    //Main Window Context Menu

    //Initialize the Pomodoro Cycle & set log_stdout and notify appropriatly.
    this->cycle = new PomodoroTimer(main_timer, 60, 60, 90, 2, 4, log_stdout);
    this->log_stdout = log_stdout;
    this->notify = notify;

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
    // Signal

    //Signal and Slot Connections
    connect(this->toggle, SIGNAL (clicked()), this, SLOT (toggle_pressed()));
    connect(this->cycle, SIGNAL (segment_changed(int)), this, SLOT (update_segment(int)));
    connect(cycle, &PomodoroTimer::timer_toggled, this, &PomodoroUI::toggled);
    if (notify){ //Only connect these sockets if notifications are enabled.
        connect(cycle, &PomodoroTimer::segment_changed, this, &PomodoroUI::notify_session);
    }
    //System tray activation signal handler.
    connect(this->tray, &QSystemTrayIcon::activated, this, &PomodoroUI::window_toggle);
    connect(loop_timer, &QTimer::timeout, this, &PomodoroUI::update_timer_display);

    //Start the main & event loop timers.
    this->cycle->initCycle(false);
    loop_timer->start(1000);
}

PomodoroUI::~PomodoroUI()
{
    delete this->cycle;
}

void PomodoroUI::SetupMenus(){
    //The tray context menu
    QMenu* tray_actions = new QMenu(this);
    QAction* toggle_window = new QAction(tr("&Show Window"), this);
    connect(toggle_window, &QAction::triggered, this, &PomodoroUI::update_visible);
    this->tray_menu_items->append(toggle_window);
    QAction* toggle_timer = new QAction(tr("&Pause Timer"), this);
    connect(toggle_timer, &QAction::triggered, this, &PomodoroUI::toggle_pressed);
    this->tray_menu_items->append(toggle_timer);
    QAction* exit_program = new QAction(tr("&Exit"), this);
    connect(exit_program, &QAction::triggered, this, &PomodoroUI::quitting);
    this->tray_menu_items->append(exit_program);
    tray_actions->addActions(*(this->tray_menu_items));
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
    QString ttip = this->toggle->text();
    ttip.append(QString::fromStdString("\nTime Left in Segment: "));
    ttip.append(this->clock->text());
    ttip.append(QString::fromStdString("\nCurrent Pomodoro: " + this->cycle->get_c_pom_str()));
    ttip.append(QString::fromStdString("\nCycles Complete: " + this->cycle->get_c_cycle_str()));
    this->tray->setToolTip(ttip);

}
//Slots:
void PomodoroUI::notify_session(int status){
    //Find and send the applicable notification.
    QString noteTitle;
    QString noteMessage;
    //QIcon *icon;
    //QSystemTrayIcon::Information;
    //int timeout = 100000
    switch(status){
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
            noteTitle = QString::fromStdString("Study Session Complete");
            noteMessage = QString::fromStdString("Congratulations! Hope you got a lot done!");
            break;
        case 5:
            noteTitle = QString::fromStdString("Restarting Study Session");
            noteMessage = QString::fromStdString("Time to get some more work done!");
            break;
        };
    this->tray->showMessage(noteTitle, noteMessage);
}
//The integer recieved signals what state we transferred to.
//0 = Session started, 1 = study -> short break, 2 = study -> long break,
//3 = x break -> study, 4 = session over, 5 = session restart.
void PomodoroUI::update_segment(int status){
    //Update various icons.
    if (this->log_stdout)
        std::cout << "Updating segment..." << std::endl;
    if (status == 0 || status == 3 || status == 5){
        this->status[0] = "Studying";
        this->status[1] = "[Paused] Studying";
        this->tray->setIcon(this->study_icon);
    }
    else if (status == 4)
        this->status[0] = this->status[1] = "Restart";
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
    QTime remTime = ZERO_TIME->addMSecs((this->main_timer->remainingTime() / 1000) * 1000);
    if(this->log_stdout)
        std::cout << qPrintable(remTime.toString("hh:mm:ss")) << std::endl;
    this->clock->setText(remTime.toString("hh:mm:ss"));
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
    ttip.append(QString::fromStdString("\nTime Left in Segment: "));
    ttip.append(this->clock->text());
    ttip.append(QString::fromStdString("\nCurrent Pomodoro: " + this->cycle->get_c_pom_str()));
    ttip.append(QString::fromStdString("\nCycles Complete: " + this->cycle->get_c_cycle_str()));
    this->tray->setToolTip(ttip);
    loop_timer->start(1000);
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
