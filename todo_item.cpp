/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#include "todo_list.h"

todo_item::todo_item(QWidget * parent) : QWidget(parent), pomodoros_elapsed(0) {
    this->setLayout((this->layout = new QGridLayout()));
    //Autocomplete task label
    this->layout->addWidget((this->p_rem_switch = new QRadioButton()), 4, 0, 1, 1, Qt::AlignLeft);
    this->layout->addWidget(new QLabel(QStringLiteral("Automatically mark complete after")), 4, 1, 1, 1, Qt::AlignRight);
    this->layout->addWidget((this->p_rem_entry = new QLineEdit()), 4, 2, 1, 1, Qt::AlignRight);
    this->layout->addWidget((this->p_rem_display = new QLabel(QStringLiteral("pomodoros. Pomodoros Elapsed: 0"))), 4, 3, 1, 1, Qt::AlignLeft);
    this->layout->addWidget((this->desc = new QTextEdit()), 1, 0, 3, 4);
    this->p_rem_switch->setChecked(false);
    this->p_rem_entry->setEnabled(false);
    QIntValidator* int_bounds = new QIntValidator();
    int_bounds->setBottom(1);
    int_bounds->setTop(99);
    this->p_rem_entry->setValidator(int_bounds);
    this->p_rem_entry->setText("");
    this->desc->setPlaceholderText("Task Description...");
    connect(this->p_rem_switch, &QRadioButton::clicked, this->p_rem_entry, &QLineEdit::setEnabled);
}

todo_item::todo_item(QJsonObject task, QWidget* parent) : todo_item(parent) {
    //Get the task title from the json object:
    QJsonValue title_json = task.value("title");
    if(!title_json.isUndefined())
        this->title = title_json.toString();
    else{
        this->title = "<TITLE MISSING>";
    }
    QJsonValue desc_json = task.value("description");
    if(!desc_json.isUndefined())
        this->desc->setText(desc_json.toString());
    else{
        this->desc->setText("<DESCRIPTION MISSING>");
    }
    QJsonValue rem_json = task.value("pomodoros");
    if(!rem_json.isUndefined() && rem_json.isDouble()){
        int rem = rem_json.toInt();
        if(rem > 0){
            this->p_rem_switch->setChecked(true);
            this->p_rem_entry->setText(QString::number(rem));
            this->p_rem_entry->setEnabled(true);
        }
    }
    QJsonValue elapsed_json = task.value("elapsed");
    this->pomodoros_elapsed = elapsed_json.toInt(0);
    this->construct_elapsed_label();
}

bool todo_item::pomodoro_complete(){
    if(this->title.isEmpty())
        return false;
    this->pomodoros_elapsed++;
    if(this->p_rem_switch->isChecked()){
        if(!this->p_rem_entry->text().isEmpty() && this->pomodoros_elapsed >= this->p_rem_entry->text().toInt())
            return true;
    }
    this->construct_elapsed_label();
    return false;
}

void todo_item::construct_elapsed_label(){
    QString label = QString::number(this->pomodoros_elapsed);
    label.prepend(QStringLiteral("pomodoros. Pomodoros Elapsed: "));
    this->p_rem_display->setText(label);
}

QJsonObject todo_item::to_json() const{
    QJsonObject task_info;
    task_info.insert("title", this->title);
    task_info.insert("description", this->desc->toPlainText());
    task_info.insert("pomodoros", ((this->p_rem_switch->isChecked()) ? this->p_rem_entry->text().toInt() : -1));
    task_info.insert("elapsed", this->pomodoros_elapsed);
    return task_info;
}
