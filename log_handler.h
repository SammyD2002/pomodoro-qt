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
/* This class creates threads that are used to log to a stream somewhere.
 *  - This can be stdout, a file, or another ostream type object.
 */
using namespace std;
class log_worker;

/* log_handler class:
 * Runs on main thread and queues log messages to be written via the log_worker objects created here.
 * Can log to a file and/or stdout.
 */
class log_handler : public QObject
{
    Q_OBJECT
public:
    //Message handler to send log messages to each log_worker instance.
    static void handle_message(QtMsgType m_type, const QMessageLogContext &context, const QString &message){
        emit instance->write(qFormatLogMessage(m_type, context, message));
        return;
    }
    //The way messages are formatted when logged.
    static const QString log_fmt;
    //Static methods to manage the state of the main log_handler instance.
    static bool started(); //Checks if instance is active.
    static void stop(); //Stops the active instance by deleting it, and sets the pointer to nullptr.
    static void load_file(ofstream*); //Loads a log file.
    static void load_cout(); //Makes application log to stdout.
    static void unload_file(); //Stops logging to the active file.
    static void unload_cout(); //Stops logging to stdout.
    static void setup(ofstream* log_file=nullptr, bool to_stdout=false, QObject* parent=nullptr); //Starts the log_handler instance.
signals:
    void write(QString); //Makes all log_handler instances write a message.
    void exitting(int exit=0); //Makes all log_handler instances stop.
private:
    static log_handler* instance; //Main log handler instance.
    ~log_handler();
    void make_connections(log_worker*); //Sets up connectinos between the log_handler and a log_worker.
    void disconnect_worker(log_worker*); //Removes connections between the log_handler and a log_worker.
    QThread file_log_thread; //The thread logging to a file.
    QThread cout_log_thread; //The thread logging to stdout.
    log_handler(ofstream* log_file, bool to_stdout, QObject* parent); //Private constructor used to setup the main instance.
    log_worker* setup_worker(ostream *out); //Sets up a log_worker instance on out.
    log_worker* setup_file_worker(ofstream *out); //Sets up a file_log_instance on out.
    log_worker *log_file; //The object on file_log_thread that logs to a file.
    log_worker *log_stdout; //The object on cout_log_thread that logs to stdout.
};

//Put on a seperate thread and pushes recieved log messages to their destination.
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

//A variant of log_worker that logs to a file and closes it when the object is destroyed.
class file_log_worker : public log_worker{
public:
    file_log_worker(ofstream* out, QThread* parent_thread) : log_worker(out, parent_thread) {}
private:
    ~file_log_worker(){
        out->close();
        delete out;
    }
    ofstream *out;
};

#endif // LOG_HANDLER_H
