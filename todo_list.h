/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#ifndef TODO_LIST_H
#define TODO_LIST_H

#include "widgets.h"

class todo_item : public QWidget
{
    Q_OBJECT
public:
    explicit todo_item(QWidget * parent = nullptr);
    explicit todo_item(QJsonObject task, QWidget* parent = nullptr);
    QJsonObject to_json() const;
    QString get_title() const {return this->title;}
    QString get_desc() const {return this->desc->toPlainText();}
    int get_rem_pomodoros() const {return (this->p_rem_switch->isChecked()) ? this->p_rem_entry->text().toInt() : -1;} //Return -1 if set to manually be completed.
    bool pomodoro_complete();
    void set_title(QString new_title){this->title = new_title;}
private:
    QString title; //Set via the drop-down box in todo_list.
    QGridLayout* layout;
    QTextEdit* desc; //Edit task description.
    QRadioButton* p_rem_switch; //Is autocomplete enabled/disabled?
    QLineEdit* p_rem_entry; //Stop after x pomodoros.
    QLabel* p_rem_display = nullptr;
    void construct_elapsed_label();
    int pomodoros_elapsed;
};
Q_DECLARE_METATYPE(todo_item*)

//Singleton
class todo_list : public QWidget
{
    Q_OBJECT
public:
    static todo_list* get_todo_list();
    int get_task_count() const {return this->task_select->count();}
    QJsonObject get_task_info(int index) const{
        todo_item* task = this->get_task(index);
        return (task == nullptr)?  QJsonObject() : task->to_json();
    }
public slots:
    void pomodoro_complete(int status);

signals:

private:
    //Singleton things
    static todo_list* instance;
    explicit todo_list(QWidget *parent = nullptr);

    //Other private attributes
    QGridLayout* layout;
    QComboBox* task_select;
    QStackedWidget* task_display;
    QLabel* completion_lable;
    todo_item* current_item = nullptr;
    int num_completed_tasks = 0;
    bool task_file_loaded;
    QPushButton* complete;
    todo_item* insert_empty(int index = -1){
        todo_item* blank = new todo_item(this);
        this->task_display->addWidget(blank);
        int i = (index < 0) ? this->task_select->count() : index;
        this->task_select->insertItem(i, "", QVariant::fromValue(blank)); //Was index earlier.
        return blank;
    }
    todo_item* get_task(int index=0) const;
    todo_item* get_current_task() const {return this->get_task(this->task_select->currentIndex());}
    bool load_task_file();
private slots:
    void current_task_updated(int index);
    void insert_task_json(todo_item *item = nullptr);
    void insert_task(){this->insert_task_json();}
    void remove_task();
    void task_title_updated();
    void task_complete();
    bool write_task_file(); //Only connected if the task file is not present or loaded.
};
#endif // TODO_LIST_H
