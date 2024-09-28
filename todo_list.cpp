/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#include "todo_list.h"
todo_list* todo_list::instance = nullptr;
todo_list* todo_list::get_todo_list(){return (instance == nullptr) ? (instance = new todo_list()) : instance;}
//TODO: Add manage queue and completed tasks options.
todo_list::todo_list(QWidget *parent): QWidget{parent}
{
    this->setLayout((this->layout = new QGridLayout()));
    this->layout->addWidget((this->task_select = new QComboBox()), 0, 0, 1, 2);
    QPushButton* rem_task = new QPushButton("Remove Task");
    this->layout->addWidget(rem_task, 0, 2, 1, 1);
    QPushButton* new_task = new QPushButton("New Task");
    this->layout->addWidget(new_task, 0, 3, 1, 1);
    this->layout->addWidget((this->task_display = new QStackedWidget()), 1, 0, 2, 4);
    complete = new QPushButton("Mark Complete");
    this->layout->addWidget(complete, 3, 0, 1, 1);
    this->completion_lable = new QLabel("Tasks completed: 0");
    this->layout->addWidget(this->completion_lable, 3, 1, 1, 1);
    connect(this->task_select, &QComboBox::currentIndexChanged, this, &todo_list::current_task_updated);
    complete->setDisabled(true);
    //Connect buttons
    connect(new_task, &QPushButton::pressed, this, &todo_list::insert_task);
    connect(rem_task, &QPushButton::pressed, this, &todo_list::remove_task);
    connect(complete, &QPushButton::pressed, this, &todo_list::task_complete);
    //Connect application exit to write_file.
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &todo_list::write_task_file);
    if (!(this->task_file_loaded = this->load_task_file()) || this->task_select->count() == 0)
        this->insert_task();
}

todo_item* todo_list::get_task(int index) const {
    if(index >= this->task_select->count() || index < 0 || this->task_select->itemData(index).isNull())
        return nullptr;
    return qvariant_cast<todo_item*>(this->task_select->itemData(index));
}
//Pass nullptr to add an empty task.
void todo_list::insert_task_json(todo_item* item){
    if(item == nullptr)
        this->insert_empty(0);
    else{
        this->task_display->addWidget(item);
        this->task_select->addItem(item->get_title(), QVariant::fromValue(item));
    }
    this->task_select->setCurrentIndex(0);
    if(!this->task_select->isEditable()){
        this->task_select->setEditable(true);
        connect(this->task_select->lineEdit(), &QLineEdit::editingFinished, this, &todo_list::task_title_updated);
        this->task_select->lineEdit()->setPlaceholderText("Task Name...");
        this->complete->setEnabled(true);
        task_select->setCompleter(nullptr);
    }
}

void todo_list::remove_task(){
    if(this->task_select->count() == 0)
        return;
    else if(this->task_select->count() == 1){
        this->insert_empty();
    }
    todo_item* item = this->get_current_task();
    this->task_display->removeWidget(item);
    this->task_select->removeItem(this->task_select->currentIndex());
    delete item;
    this->task_select->setCurrentIndex(0);
}

void todo_list::task_title_updated(){
    QString text = this->task_select->lineEdit()->text();
    int index = this->task_select->findText(text, Qt::MatchExactly);
    if(index == -1){
        this->current_item->set_title(text);
        this->task_select->setItemText(this->task_select->currentIndex(), text);
    }
    else if (index != this->task_select->currentIndex()){
        QString err = QString::fromStdString("The task '" + text.toStdString() + "' already exists.");
        QMessageBox::critical(this,"Task Already Exists", err, "Ok");
        this->task_select->setCurrentText(this->current_item->get_title());
    }
}

void todo_list::current_task_updated(int index){
    QVariant item_variant = this->task_select->itemData(index);
    if(!item_variant.isNull()){
        todo_item* new_item = qvariant_cast<todo_item*>(item_variant);
        this->task_display->setCurrentWidget(new_item);
        this->current_item = new_item;
    }
}

void todo_list::pomodoro_complete(int status){
    todo_item* curr_task = this->get_current_task();
    if(status >= 3 && curr_task != nullptr && curr_task->pomodoro_complete())
        this->task_complete();
}

void todo_list::task_complete(){
    if(!this->task_select->currentText().isEmpty()){
        this->num_completed_tasks++;
        this->remove_task();
        this->completion_lable->setText("Tasks completed: " + QString::number(this->num_completed_tasks));
    }
}

bool todo_list::load_task_file(){
    QFile task_file(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/tasks.json");
    if(task_file.exists() && QFileInfo(task_file).isReadable()){
        task_file.open(QIODevice::ReadOnly);
        QJsonParseError err;
        QJsonDocument task_json = QJsonDocument::fromJson(task_file.readAll(), &err);
        if(err.error != QJsonParseError::NoError){
            qWarning("%s", ("Could not parse task file. " + err.errorString().toStdString()).c_str());
            return false;
        }
        else if(!task_json.isArray()){
            qWarning("Task file is valid JSON but is not an array.");
        }
        QJsonArray task_arr = task_json.array();
        for(QJsonArray::const_iterator i = task_arr.cbegin(); i != task_arr.cend(); i++){
            this->insert_task_json(new todo_item(i->toObject(), this));
        }
        task_file.close();
        return true;
    }
    return !task_file.exists();
}

bool todo_list::write_task_file(){
    if(!this->task_file_loaded)
        return true;
    QFile task_file(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/tasks.json");
    if(!task_file.exists() || QFileInfo(task_file).isWritable()){
        task_file.open(QIODevice::WriteOnly);
        QJsonArray tasks;
        if(!this->task_select->currentText().isEmpty()){
            todo_item* curr = this->get_current_task();
            if (curr != nullptr)
                tasks.append(curr->to_json());
        }
        for(int i = 0; i < this->task_select->count(); i++){
            if(!this->task_select->itemText(i).isEmpty() && i != this->task_select->currentIndex()){
                QVariant v = this->task_select->itemData(i);
                todo_item* item = qvariant_cast<todo_item*>(v);
                if(item != nullptr)
                    tasks.append(item->to_json());
            }
        }
        QJsonDocument j;
        j.setArray(tasks);
        QByteArray task_data = j.toJson();
        task_file.write(task_data);
        task_file.close();
        return true;
    }
    return false;
}


