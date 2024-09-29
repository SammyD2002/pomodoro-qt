/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#ifndef TODO_LIST_H
#define TODO_LIST_H
#include "widgets.h"
class todo_history;
class todo_item : public QWidget
{
    Q_OBJECT
public:
    explicit todo_item(QWidget *parent = nullptr);
    explicit todo_item(QJsonObject task, QWidget* parent = nullptr);
    explicit todo_item(todo_item &base);
    QJsonObject to_json() const;
    QString get_title() const {return this->title;}
    QString get_desc() const {return this->desc->toPlainText();}
    int get_rem_pomodoros() const {return (this->p_rem_switch->isChecked()) ? this->p_rem_entry->text().toInt() : -1;} //Return -1 if set to manually be completed.
    bool pomodoro_complete();
    void set_title(QString new_title){this->title = new_title;}
    bool isEmpty(){return this->title.isEmpty() && this->desc->toPlainText().isEmpty() && (!this->p_rem_switch->isChecked() || this->p_rem_entry->text().isEmpty());}
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

private:
    //Singleton things
    static todo_list* instance;
    explicit todo_list(QWidget *parent = nullptr);

    //Other private attributes
    QGridLayout* layout;
    //QComboBox* task_select;
    QListWidget* task_select;
    QStackedWidget* task_display;
    QLabel* completion_lable;
    todo_item* current_item = nullptr;
    todo_history* hist;
    int num_completed_tasks = 0;
    bool task_file_loaded;
    QPushButton* complete;
    QListWidgetItem* insert_empty(int index = -1){
        todo_item* blank = new todo_item(this);
        this->task_display->addWidget(blank);
        int i = (index < 0) ? this->task_select->count() : index;
        QListWidgetItem* item = new QListWidgetItem("");
        item->setData(Qt::UserRole, QVariant::fromValue(blank));
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        this->task_select->insertItem(i, item); //Was index earlier.
        return item;
    }
    todo_item* get_task(QListWidgetItem*) const;
    todo_item* get_task(int index=0) const;
    todo_item* get_current_task() const {return this->get_task(this->task_select->currentRow());}
    bool load_task_file();
signals:
    void queue_edit(QListWidgetItem* item);
    void task_completed(todo_item* task);

private slots:
    void current_task_updated(int index);
    QListWidgetItem* insert_task_json(todo_item *item = nullptr);
    void insert_task(){
        QListWidgetItem* item;
        todo_item* t = (this->task_select->count() > 0) ? this->get_task(0) : nullptr;
        if (!(t != nullptr && t->isEmpty()))
            item = this->insert_empty(0);
        else
            item = this->task_select->item(0);
        this->task_select->setCurrentRow(0);
        this->task_select->editItem(item);
    }
    void requeue_task(QJsonObject &t){
        t.insert("elapsed", 0);
        this->insert_task_json(new todo_item(t, this));
    }
    void remove_task();
    void task_title_updated(QListWidgetItem* item);
    void task_complete();
    bool write_task_file(); //Only connected if the task file is not present or loaded.
    void start_edit(QListWidgetItem* item);
};

class todo_history : public QWidget
{
    Q_OBJECT
public:
    explicit todo_history(todo_list *parent=nullptr);
    int get_completed_tasks() const {return this->task_select.count();}
signals:
    void requeue_task(QJsonObject &task);
private:
    QGridLayout layout;
    QListWidget task_select;
    QTextBrowser task_desc;
    QLabel task_time = QLabel("Completion Time: ");
    QLabel task_constraints = QLabel("Pomodoros Allocated: ");
    QLabel task_duration = QLabel("Task Duration: ");
    //Store the completion dates as a string here.
    QMap<QListWidgetItem*, qint64> completion_dates;
private slots:
    void task_completed(todo_item *task);
    void item_changed(int row);
    void prep_requeue();
};

#endif // TODO_LIST_H
