/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#include "pomodoro_ui.h"
#include "log_handler.h"
#include <QApplication>

#ifdef QT_NO_QDEBUG
#define DEF_LOG_FILE QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/pomodoro.log"
#else
#define DEF_LOG_FILE QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/pomodoro.debug.log"
#endif

//#ifndef QT_DEBUG
//#define QT_LOGGING_RULES "*.debug=false\n"
//#endif

using namespace std;
ofstream* load_log_file(QString log_path){
    if (log_path.isEmpty())
        log_path = DEF_LOG_FILE;
    ofstream* file = new ofstream(log_path.toStdString(), ios::app);
    if(file->fail()){
        if(log_path != DEF_LOG_FILE){
            delete file;
            throw invalid_argument((stringstream() << "The specified log file '" << qPrintable(log_path) << "'could not be opened. Exitting...").str().c_str());
        }
        else{
            qWarning("The default log file could not be opened. Starting without log...");
            return file;
        }
    }
    qDebug("File loaded.");
    return file;

}

string check_set(bool v){
    return v ? "set" : "not set";
}

bool open_log(ofstream* out=nullptr, bool to_stdout=false){
    //ofstream should at least be attempted to be opened in append mode prior.
    if (out != nullptr){
        if(!(out->fail())){
            log_handler::setup(out, to_stdout);
            return true;
        }
        else{
            delete out;
            log_handler::setup(nullptr, to_stdout);
            return false;
        }
    }
    else{
        log_handler::setup(nullptr, to_stdout);
        return true;
    }

}

QJsonObject* find_starting_preset(PresetManager* manager, QString preset_name){
    QJsonObject* alt = manager->getPreset(preset_name);
    if(alt == nullptr){
        throw(invalid_argument((stringstream() << "ERROR: The Preset '" << qPrintable(preset_name) << "' was not found.").str().c_str()));
    }
    else
        return alt;
}

PresetManager* handle_manager_start(QString def, QString pre, ofstream* log_f, bool to_stdout){
    if (QFileInfo(pre).isDir())
        throw invalid_argument((stringstream() << "ERROR: '" << qPrintable(pre) << "' is a directory.").str().c_str());
    else if (QFileInfo(def).isDir())
        throw invalid_argument((stringstream() << "ERROR: '" << qPrintable(def) << "' is a directory.").str().c_str());
    log_handler::setup(log_f, to_stdout);
    return new PresetManager(def, pre);
}

//QList<QCommandLineOption>
void add_options(QCommandLineParser *argp){
    //QList<QCommandLineOption> opts;
    //Specify the preset to load.
    argp->addPositionalArgument("Preset", "Specify the preset to load at startup", "<PRESET_NAME>");
    //Prints the log to stdout in addition to the log file.
    QStringList to_stdout_opts = QStringList() << "o" << "to-stdout";
    QCommandLineOption to_stdout(to_stdout_opts, "Print application log to stdout");
    //opts.append(to_stdout);
    //Prints the log exclusivly to stdout.
    QCommandLineOption ex_to_stdout("O", "Print application log ONLY to stdout");
    //Specify the file with the default preset.
    QCommandLineOption def_preset_file("d", "Specify the file with the default preset.", "def_preset_file", QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/default.json");
    //Specifies the file with the other presets.
    QCommandLineOption preset_file("p", "Specify the file with the extra presets.", "preset_file", QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/presets.json");
    //Specify the config directory
    QStringList preset_dir_opts = QStringList() << "P" << "D";
    QCommandLineOption preset_dir(preset_dir_opts, "Specify the directory with the preset file.", "preset_dir");
    //Specifies the log file.
    QCommandLineOption log_file("l", "Specify log file location.", "log_file", DEF_LOG_FILE);
    //Adds the -v version option.
    argp->addVersionOption();
    //Adds the -h,--help,&-? help options
    QStringList help_opts = QStringList() << "h" << "help" << "?";
    QCommandLineOption help(help_opts, "Displays help on commandline options.");
    qDebug("%s %s","-o is", check_set(argp->addOption(to_stdout)).c_str());
    qDebug("%s %s","-O is", check_set(argp->addOption(ex_to_stdout)).c_str());
    qDebug("%s %s", "-d is", check_set(argp->addOption(def_preset_file)).c_str());
    qDebug("%s %s", "-p is", check_set(argp->addOption(preset_file)).c_str());
    qDebug("%s %s", "-P/-D is", check_set(argp->addOption(preset_dir)).c_str());
    qDebug("%s %s", "-l is", check_set(argp->addOption(log_file)).c_str());
    qDebug("%s %s", "The help options -h/--help/-? are", check_set(argp->addOption(help)).c_str());
}

//returns true if the program can continue, false otherwise.
QJsonObject* process_options(QApplication *a, QCommandLineParser *argp, PresetManager *&man){
    try{
        ofstream* log_f = nullptr;
        bool to_stdout = false;
        add_options(argp);
        argp->parse(a->arguments());
        qDebug("Arguments Processed.");
        //This is handled by process().
        //Look for syntax errors, conflicting options, help/version, and missing files first.
        if(argp->isSet("h")){
            argp->showHelp(0);
        }
        if(argp->isSet("v")){
            log_handler::stop();
            argp->showVersion();
        }
        QStringList preset = argp->positionalArguments();
        if(argp->unknownOptionNames().length() == 1){
            throw invalid_argument((stringstream() << "ERROR: '" << qPrintable(argp->unknownOptionNames().join(',')) << "' is not a recognized option.").str().c_str());
        }
        else if(argp->unknownOptionNames().length() > 1)
            throw invalid_argument((stringstream() << "ERROR: '" << qPrintable(argp->unknownOptionNames().join(',')) << "' are not recognized options.").str().c_str());
        if(argp->positionalArguments().length() > 1){
            throw invalid_argument("ERROR: Too many arguments. (More than one preset was specified.)");
        }
        if((argp->isSet("P") || argp->isSet("D")) && (argp->isSet("p") || argp->isSet("d")))
            throw invalid_argument("ERROR: -p/-d & -P/-D conflict and should not be used together.");
        if(argp->isSet("l") && argp->isSet("O")){ //These options conflict.
            throw invalid_argument("-l & -P conflict and should not be used together.\nTo print to a custom log file & stdout, use -l & -p.");
        }
        if(!argp->isSet("O"))
            log_f = load_log_file(argp->value("l"));
        else
            qDebug("-O passed, not opening a log file...");
        if(argp->isSet("o") || argp->isSet("O"))
            to_stdout = true;
        //Next, start the log handler before attempting to load the preset file.
        if(argp->isSet("P") || argp->isSet("D")){
            if(QFileInfo(argp->value("P")).isFile())
                throw invalid_argument((stringstream() << "ERROR: '" << qPrintable(argp->value("P")) << "' is a file.").str().c_str());
            log_handler::setup(log_f, to_stdout);
            man = new PresetManager(nullptr, argp->value("P"));
        }
        else{
            man = handle_manager_start(argp->value("d"), argp->value("p"), log_f, to_stdout);
        }
        if(preset.length() == 1)
            return find_starting_preset(man, preset[0]);
        else
            return man->getDefaultPreset();
    }
    catch (invalid_argument &err){
        if (log_handler::started())
            log_handler::stop();
        printf("%s\n", err.what());
        //qCritical("%s", qPrintable(argp->helpText()));
        if(man != nullptr)
            delete man;
        argp->showHelp(1);
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCommandLineParser argp;
#ifdef QT_NO_DEBUG
    QLoggingCategory::setFilterRules("*.debug=false\n");
#endif
    qSetMessagePattern(log_handler::log_fmt);
    PresetManager* man = nullptr;
    QJsonObject* starting_preset = process_options(&a, &argp, man);
    qInfo("Starting...");
    PomodoroUI w(man, starting_preset);
    //w.show();
    int exit_code = a.exec();
    log_handler::stop();
    return exit_code;
}
