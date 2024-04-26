/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#include "log_handler.h"
log_handler* log_handler::instance = nullptr;
const QString log_handler::log_fmt = QString("(%{time process})%{if-critical}<[%{type}]>%{endif}") +
                                            QString("%{if-fatal}<[*%{type}*]>%{endif}") +
                                            QString("%{if-warning}<%{type}>%{endif}") +
                                            QString("%{if-info}[%{type}]%{endif}") +
                                            QString("%{if-debug}{%{type}}%{endif}") +
                                            QString(" - %{function}: %{message}");
void log_handler::setup(ofstream *log_file, bool to_stdout, QObject *parent){
    if(log_handler::instance == nullptr){
        log_handler::instance = new log_handler(log_file, to_stdout, parent);
        qInstallMessageHandler(log_handler::handle_message);
    }
}

bool log_handler::started(){
    return instance != nullptr;
}

void log_handler::load_file(ofstream* log_file){
    if(instance == nullptr){
        log_handler::setup(log_file, false);
    }
    if(instance->log_file != nullptr)
        instance->unload_file();
    instance->log_file = instance->setup_file_worker(log_file);
    instance->file_log_thread.start();
}

void log_handler::load_cout(){
    if(instance == nullptr){
        log_handler::setup(nullptr, true);
    }
    if (instance->log_stdout != nullptr)
        instance->unload_cout();
    instance->log_stdout = instance->setup_worker(&cout);
    instance->cout_log_thread.start();
}

void log_handler::unload_cout(){
    log_handler::instance->disconnect_worker(log_handler::instance->log_stdout);
    instance->cout_log_thread.wait();
    instance->log_stdout->deleteLater();
    instance->log_stdout = nullptr;
}

void log_handler::unload_file(){
    log_handler::instance->disconnect_worker(log_handler::instance->log_file);
    instance->file_log_thread.wait();
    instance->log_file->deleteLater();
    instance->log_file = nullptr;
}

void log_handler::disconnect_worker(log_worker * worker){
    disconnect(this, &log_handler::write, worker, &log_worker::write);
    disconnect(this, &log_handler::exitting, worker, &log_worker::cleanup);
    QMetaObject::invokeMethod(worker, &log_worker::cleanup, Qt::QueuedConnection);
}

void log_handler::stop(){
    delete log_handler::instance;
}

log_handler::~log_handler(){
    emit this->exitting();
    this->file_log_thread.wait();
    this->cout_log_thread.wait();
    if(this->log_file != nullptr)
        this->log_file->deleteLater();
    if(this->log_stdout != nullptr)
        this->log_stdout->deleteLater();
}
log_handler::log_handler(ofstream* log_file, bool to_stdout, QObject* parent) : QObject(parent){
    if (to_stdout){
        this->log_stdout = setup_worker(&cout);
        this->cout_log_thread.start();
    }
    else
        this->log_stdout = nullptr;
    if (log_file != nullptr){
        this->log_file = setup_file_worker(log_file);
        this->file_log_thread.start();
    }
    else
        this->log_file = nullptr;
}
log_worker* log_handler::setup_worker(ostream *out){
    log_worker *worker = new log_worker(out, &cout_log_thread);
    worker->moveToThread(&cout_log_thread);
    this->make_connections(worker);
    return worker;
}
void log_handler::make_connections(log_worker * worker){
    connect(this, &log_handler::write, worker, &log_worker::write, Qt::QueuedConnection);
    connect(this, &log_handler::exitting, worker, &log_worker::cleanup, Qt::QueuedConnection);
}

log_worker* log_handler::setup_file_worker(ofstream *out){
    log_worker *worker = new file_log_worker(out, &file_log_thread);
    worker->moveToThread(&file_log_thread);
    this->make_connections(worker);
    return worker;
}

void log_worker::cleanup(){
    this->parent_thread->quit();
    return; //The return instruction is required?, otherwise the program just hangs here.
}
