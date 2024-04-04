#include "timerconfig.h"
NotificationEditor::NotificationEditor(TimerConfig *parent) : QWidget(parent){
    //Initialize the layout
    this->layout = new QGridLayout(this);
    //Set the parent configuration window variable.
    this->parentConfig = parent;

    //Inititalize and set the locations of labels and line edits.
    //Session Started
    this->labels[0] = new QLabel("Session Started:");
    this->title_inputs[0] = new QLineEdit();
    this->layout->addWidget(labels[0], 0, 0);
    this->layout->addWidget(title_inputs[0], 0, 1, 1, -1); //-1 Column Span fills the remaining space.

    //Study Segment Complete
    this->labels[1] = new QLabel("Study Segment Completed");
    this->title_inputs[1] = new QLineEdit();
    this->layout->addWidget(this->labels[1], 1, 0);
    this->layout->addWidget(this->title_inputs[1], 1, 1, 1, -1);

    //Break Complete
    this->labels[2] = new QLabel("Break Completed");
    this->title_inputs[2] = new QLineEdit();
    this->layout->addWidget(this->labels[2], 2, 0);
    this->layout->addWidget(this->title_inputs[2], 2, 1, 1, -1);

    //Pomodoro Cycle Complete
    this->labels[3] = new QLabel("Cycle Completed");
    this->title_inputs[3] = new QLineEdit();
    this->layout->addWidget(this->labels[3], 3, 0);
    this->layout->addWidget(this->title_inputs[3], 3, 1, 1, -1);

    //Session Complete
    this->labels[4] = new QLabel("Session Completed");
    this->title_inputs[4] = new QLineEdit();
    this->layout->addWidget(this->labels[4], 4, 0);
    this->layout->addWidget(this->title_inputs[4], 4, 1, 1, -1);


    //Session Restarted
    this->labels[5] = new QLabel("Session Restarted");
    this->title_inputs[5] = new QLineEdit();
    this->layout->addWidget(this->labels[5], 5, 0);
    this->layout->addWidget(this->title_inputs[5], 5, 1, 1, -1);

    //Help button:
    this->help = new QPushButton("Help");
    this->layout->addWidget(this->help, 6, 0);
}

void NotificationEditor::setPlaceholders(PomodoroTimer* parentTimer){
    for(int i = 0; i < 6; i++)
        this->title_inputs[i]->setText(parentTimer->getMessageTitleTemplate(i));
}

void MessageEditor::setPlaceholders(PomodoroTimer *parentTimer){
    for (int i = 0; i < 6; i++)
        this->title_inputs[i]->setText(parentTimer->getMessageBodyTemplate(i));
}

void NotificationEditor::getTitleInputs(QString (&src)[6]) const{
    src[0] = this->title_inputs[0]->text();
    src[1] = this->title_inputs[1]->text();
    src[2] = this->title_inputs[2]->text();
    src[3] = this->title_inputs[3]->text();
    src[4] = this->title_inputs[4]->text();
    src[5] = this->title_inputs[5]->text();
}

void NotificationEditor::retrieve_help(){
    help_browser::load_help("NotificationEditor", this->parentConfig);
}
