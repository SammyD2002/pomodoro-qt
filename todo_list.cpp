/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#include "todo_list.h"
todo_list* todo_list::instance = nullptr;
todo_list* todo_list::get_todo_list(){return (instance == nullptr) ? (instance = new todo_list()) : instance;}
//TODO: Add manage queue and completed tasks options.
todo_list::todo_list(QWidget *parent): QWidget(parent)
{
    this->setLayout((this->layout = new QGridLayout()));
    this->layout->addWidget((this->task_select = new QListWidget()), 0, 0, 4, 1);
    QPushButton* new_task = new QPushButton("New Task");
    this->layout->addWidget(new_task, 4, 0, 1, 1);
    QPushButton* rem_task = new QPushButton("Remove Task");
    this->layout->addWidget(rem_task, 5, 0, 1, 1);
    this->layout->addWidget((this->task_display = new QStackedWidget()), 0, 2, 4, 2);
    complete = new QPushButton("Mark Complete");
    this->layout->addWidget(complete, 4, 1, 1, -1);
    this->completion_lable = new QLabel("Tasks completed: 0");
    this->layout->addWidget(this->completion_lable, 5, 3, 1, 1);
    this->task_select->setDragDropMode(QAbstractItemView::InternalMove);
    QPushButton* b = new QPushButton("Load task history");
    this->layout->addWidget(b, 5, 1, 1, 2);
    this->hist = new todo_history(this);
    connect(hist, &todo_history::requeue_task, this, &todo_list::requeue_task);
    connect(this->task_select, &QListWidget::currentRowChanged, this, &todo_list::current_task_updated);
    //Connect buttons
    connect(new_task, &QPushButton::pressed, this, &todo_list::insert_task);
    connect(rem_task, &QPushButton::pressed, this, &todo_list::remove_task);
    connect(complete, &QPushButton::pressed, this, &todo_list::task_complete);
    //Connect application exit to write_file.
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &todo_list::write_task_file);
    if (!(this->task_file_loaded = this->load_task_file()) || this->task_select->count() == 0)
        this->insert_task_json(nullptr);
    connect(this->task_select, &QListWidget::itemChanged, this, &todo_list::task_title_updated);
    connect(this, &todo_list::queue_edit, this, &todo_list::start_edit, Qt::QueuedConnection);
    connect(b, &QPushButton::pressed, this->hist, &todo_history::show);
}

todo_item* todo_list::get_task(QListWidgetItem* item) const {
    return qvariant_cast<todo_item*>(item->data(Qt::UserRole));
}

todo_item* todo_list::get_task(int index) const {
    if(index >= this->task_select->count() || index < 0 || this->task_select->item(index)->data(Qt::UserRole).isNull())
        return nullptr;
    return this->get_task(task_select->item(index));
}
//Pass nullptr to add an empty task.
QListWidgetItem* todo_list::insert_task_json(todo_item* item){
    QListWidgetItem* i;
    if(item == nullptr)
        i = this->insert_empty(0);
    else{
        this->task_display->addWidget(item);
        i = new QListWidgetItem(item->get_title());
        i->setData(Qt::UserRole, QVariant::fromValue(item));
        i->setFlags(i->flags() | Qt::ItemIsEditable);
        this->task_select->insertItem(0, i);
    }
    this->task_select->setCurrentRow(0);
    return i;
    /*
    if(!this->task_select->isEditable()){
        this->task_select->setEditable(true);
        connect(this->task_select->lineEdit(), &QLineEdit::editingFinished, this, &todo_list::task_title_updated);
        this->task_select->lineEdit()->setPlaceholderText("Task Name...");
        this->complete->setEnabled(true);
        task_select->setCompleter(nullptr);
    }
    */
}

void todo_list::remove_task(){
    if(this->task_select->count() == 0)
        return;
    else if(this->task_select->count() == 1){
        this->insert_empty();
    }
    QListWidgetItem* i = this->task_select->takeItem(this->task_select->currentRow());
    todo_item* item = qvariant_cast<todo_item*>(i->data(Qt::UserRole));
    this->task_display->removeWidget(item);
    delete item;
    delete i;
}

void todo_list::task_title_updated(QListWidgetItem *item){
    QString text = item->text();
    todo_item* entry = this->get_task(item);
    QList<QListWidgetItem*> items = this->task_select->findItems(text, Qt::MatchExactly);
    if(items.length() == 1 && items.contains(item)){
        entry->set_title(text);
    }
    else{
        //item->setText(entry->get_title()); //Set the text back to what it was before.
        QString err = QString::fromStdString("The task '" + text.toStdString() + "' already exists.");
        QMessageBox m;
        m.setWindowTitle("Task Already Exists");
        m.setText(err);
        m.setIcon(QMessageBox::Critical);
        m.setModal(true);
        m.exec();
        emit this->queue_edit(item);
    }
}

void todo_list::current_task_updated(int index){
    QVariant item_variant = this->task_select->item(index)->data(Qt::UserRole);
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
    if(!this->task_select->currentItem()->text().isEmpty()){
        this->num_completed_tasks++;
        emit this->task_completed(this->get_current_task());
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
        for(int i = 0; i < this->task_select->count(); i++){
            // && i != this->task_select->currentRow()
            if(!this->task_select->item(i)->text().isEmpty()){
                QVariant v = this->task_select->item(i)->data(Qt::UserRole);
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

//Wait for control to return to the main loop before restarting editing when a duplicate name is entered.
void todo_list::start_edit(QListWidgetItem* item){
    this->task_select->editItem(item);
}

