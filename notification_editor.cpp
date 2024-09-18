/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#include "timerconfig.h"
NotificationEditor::NotificationEditor(TimerConfig *parent) : QWidget(parent){
    //Initialize the layout
    this->layout = new QGridLayout(this);
    //Set the parent configuration window variable.
    this->parentConfig = parent;
    //Items:
    this->items[0] = new QLabel(QStringLiteral("Notification"));
    this->layout->addWidget(this->items[0], 0, 0, 1, 1);
    this->items[1] = new QLabel(QStringLiteral("Title"));
    this->layout->addWidget(this->items[1], 0, 1, 1, 1);
    this->items[2] = new QLabel(QStringLiteral("Body"));
    this->layout->addWidget(this->items[2], 0, 5, 1, 1);

    //Inititalize and set the locations of labels and line edits.
    //Session Started
    this->labels[0] = new QLabel(QStringLiteral("Session Started"));
    this->title_inputs[0] = new QLineEdit();
    this->layout->addWidget(labels[0], 1, 0);
    this->layout->addWidget(title_inputs[0], 1, 1, 1, 3); //-1 Column Span fills the remaining space.

    //Study Segment Complete
    this->labels[1] = new QLabel(QStringLiteral("Study Segment Completed"));
    this->title_inputs[1] = new QLineEdit();
    this->layout->addWidget(this->labels[1], 2, 0);
    this->layout->addWidget(this->title_inputs[1], 2, 1, 1, 3);

    //Break Complete
    this->labels[2] = new QLabel(QStringLiteral("Break Completed"));
    this->title_inputs[2] = new QLineEdit();
    this->layout->addWidget(this->labels[2], 3, 0);
    this->layout->addWidget(this->title_inputs[2], 3, 1, 1, 3);

    //Pomodoro Cycle Complete
    this->labels[3] = new QLabel(QStringLiteral("Cycle Completed"));
    this->title_inputs[3] = new QLineEdit();
    this->layout->addWidget(this->labels[3], 4, 0);
    this->layout->addWidget(this->title_inputs[3], 4, 1, 1, 3);

    //Session Complete
    this->labels[4] = new QLabel(QStringLiteral("Session Completed"));
    this->title_inputs[4] = new QLineEdit();
    this->layout->addWidget(this->labels[4], 5, 0);
    this->layout->addWidget(this->title_inputs[4], 5, 1, 1, 3);


    //Session Restarted
    this->labels[5] = new QLabel(QStringLiteral("Session Restarted"));
    this->title_inputs[5] = new QLineEdit();
    this->layout->addWidget(this->labels[5], 6, 0);
    this->layout->addWidget(this->title_inputs[5], 6, 1, 1, 3);

    //Help button:
    this->help = new QPushButton(QStringLiteral("Help"));
    this->layout->addWidget(this->help, 7, 0);

    //Message Input Buttons
    for(int i = 0; i < 6; i++){
        this->message_body_setters[i] = new QPushButton(QStringLiteral("Edit"), this);
        this->layout->addWidget(this->message_body_setters[i], i+1, 5);
        connect(this->message_body_setters[i], &QPushButton::clicked, this, &NotificationEditor::edit_body);
    }
}

void NotificationEditor::setPlaceholders(PomodoroTimer* parentTimer){
    for(int i = 0; i < 6; i++){
        this->message_inputs[i] = this->init_message_inputs[i] = parentTimer->getMessageBodyTemplate(i);
        this->title_inputs[i]->setText(parentTimer->getMessageTitleTemplate(i));
    }
}

void NotificationEditor::edit_body(){
    for (int i = 0; i < 6; i++){
        if(this->message_body_setters[i] == QObject::sender()){
            bool ok;
            QString tmp_text = QInputDialog::getMultiLineText(this, this->labels[i]->text().append(" - Notification Body"), "Notification Body:", this->message_inputs[i], &ok);
            if(ok && !tmp_text.isEmpty()){
                this->message_inputs[i] = tmp_text;
            }
        }
    }
}


void NotificationEditor::getTitleInputs(QString (&src)[6]) const{
    src[0] = this->title_inputs[0]->text();
    src[1] = this->title_inputs[1]->text();
    src[2] = this->title_inputs[2]->text();
    src[3] = this->title_inputs[3]->text();
    src[4] = this->title_inputs[4]->text();
    src[5] = this->title_inputs[5]->text();
}

void NotificationEditor::getMessageInputs(QString (&src)[6]) const{
    src[0] = this->message_inputs[0];
    src[1] = this->message_inputs[1];
    src[2] = this->message_inputs[2];
    src[3] = this->message_inputs[3];
    src[4] = this->message_inputs[4];
    src[5] = this->message_inputs[5];
}

void NotificationEditor::retrieve_help(){
    help_browser::load_help(QStringLiteral("NotificationEditor"));
}
