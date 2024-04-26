/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#ifndef LOG_HANDLER_H
#define LOG_HANDLER_H
#include "widgets.h"
#include <fstream>
#include <iostream>
//The idea:
//1. Check for incoming signals
//2. Add new messages to the queue
//3. Write the oldest message in the queue if it exists
//4. Flush Queue
//2-3 can be handled via queded signal slot connections.
using namespace std;
class log_worker;

class log_handler : public QObject
{
    Q_OBJECT
    //QThread logging_thread;
public:
    static void handle_message(QtMsgType m_type, const QMessageLogContext &context, const QString &message){
        emit instance->write(qFormatLogMessage(m_type, context, message));
        return;
    }
    static const QString log_fmt;
    static bool started();
    static void stop();
    static void load_file(ofstream*);
    static void load_cout();
    static void unload_file();
    static void unload_cout();
    static void setup(ofstream* log_file=nullptr, bool to_stdout=false, QObject* parent=nullptr);
signals:
    void write(QString);
    void exitting(int exit=0);
private:
    static log_handler* instance;
    ~log_handler();
    void make_connections(log_worker*);
    void disconnect_worker(log_worker*);
    QThread file_log_thread;
    QThread cout_log_thread;
    log_handler(ofstream* log_file, bool to_stdout, QObject* parent);
    log_worker* setup_worker(ostream *out);
    log_worker* setup_file_worker(ofstream *out);
    log_worker *log_file;
    log_worker *log_stdout;
};


class log_worker : public QObject{
    Q_OBJECT
public:
    log_worker(ostream* out, QThread* parent_thread) : QObject(nullptr), out(out), parent_thread(parent_thread) {}
public slots:
    void cleanup();
    void write(QString message){*out << qPrintable(message) << endl; out->flush();}
private:
    ostream *out;
    QThread* parent_thread;
};

class file_log_worker : public log_worker{
public:
    file_log_worker(ofstream* out, QThread* parent_thread) : log_worker(out, parent_thread) {}
private:
    ~file_log_worker(){
        cout << "destruction" << endl;
        out->close();
        delete out;
    }
    ofstream *out;
};

#endif // LOG_HANDLER_H
