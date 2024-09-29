#include "todo_list.h"
todo_history::todo_history(todo_list* parent) : QWidget(nullptr){
    QPushButton* requeue_button = new QPushButton("Re-Enqueue Task");
    this->setLayout(&layout);
    this->layout.addWidget(&this->task_select, 0, 0, 4, 1, Qt::AlignLeft);
    this->layout.addWidget(&this->task_desc, 0, 1, 4, 4);
    this->layout.addWidget(requeue_button, 4, 0, 1, 1);
    this->layout.addWidget(&this->task_constraints, 4, 1, 1, -1);
    this->layout.addWidget(&this->task_duration, 5, 1, 1, -1);
    this->layout.addWidget(&this->task_time, 6, 1, 1, -1);
    this->task_desc.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->task_constraints.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->task_duration.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(requeue_button, &QPushButton::pressed, this, &todo_history::prep_requeue);
    connect(parent, &todo_list::task_completed, this, &todo_history::task_completed);
    connect(&this->task_select, &QListWidget::currentRowChanged, this, &todo_history::item_changed);
}

void todo_history::task_completed(todo_item* task){
    qint64 time = QDateTime::currentMSecsSinceEpoch();
    QJsonObject task_info;
    task_info.insert(QString::number(time), task->to_json());
    QListWidgetItem* new_task = new QListWidgetItem(task->get_title());
    new_task->setData(Qt::UserRole, QVariant::fromValue(task_info));
    this->task_select.addItem(new_task);
}

void todo_history::item_changed(int row){
    QListWidgetItem* item = this->task_select.item(row);
    QJsonObject info = item->data(Qt::UserRole).toJsonObject();
    QVariant time = QVariant::fromValue(info.keys().at(0));
    QDateTime d = QDateTime::fromMSecsSinceEpoch(qvariant_cast<qint64>(time));
    QJsonObject task = info.value(time.toString()).toObject();
    this->task_time.setText("Completion Date: " + d.toLocalTime().toString("ddd MMMM d yyyy - hh:mm:ss"));
    this->task_desc.setText(task.value("description").toString(""));
    int allocation = task.value("pomodoros").toInt();
    QString allocation_str = "Pomodoros Allocated: ";
    if(allocation == -1)
        allocation_str.append("∞");
    else
        allocation_str.append(QString::number(allocation));
    this->task_constraints.setText(allocation_str);
    this->task_duration.setText("Task Duration: " + QString::number(task.value("elapsed").toInt()) + " pomodoors");
}

void todo_history::prep_requeue(){
    QVariant item = this->task_select.currentItem()->data(Qt::UserRole);
    QJsonObject historic = item.toJsonObject();
    QString date = historic.keys().at(0);
    QJsonObject task(historic.value(date).toObject());
    emit this->requeue_task(task);
}
