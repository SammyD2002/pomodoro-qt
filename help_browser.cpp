/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#include "help_browser.h"
#include <iostream>
//Initialize pointer to help browser instance to nullptr.
QHelpEngineCore* help_browser::help = nullptr;
help_browser::help_viewer* help_browser::viewer = nullptr;

//Methods for help_viewer class:
help_browser::help_viewer::help_viewer(QWidget* parent) : QTextBrowser(parent) {}

//Sourced from example at https://code.qt.io/cgit/qt/qttools.git/tree/examples/help/contextsensitivehelp?h=6.7
QVariant help_browser::help_viewer::loadResource(int type, const QUrl &name){
    QByteArray ba;
    if (type < 4 && help_browser::help) {
        QUrl url(name);
        if (name.isRelative())
            url = source().resolved(url);
        ba = help_browser::help->fileData(url);
    }
    return ba;
}

//Loads help document for <component>
void help_browser::load_help(QString component){
    if (help_browser::setup_help()){
        QList<QHelpLink> links = help_browser::help->documentsForIdentifier("Pomodoro::" + component, "");
        if (links.count())
            help_browser::setup_viewer(links.first().url);
        else{
            return;
        }
    }
    else{
        delete help_browser::help;
        help_browser::help = NULL;
    }

}
/*
Loading Help:
1. Write help cached here to the cache location
2. Create a new help engine targeting the newly created help file.
*/
bool help_browser::setup_help(){
    if (help_browser::help == NULL){
        if (help_browser::cache_documentation(QStandardPaths::writableLocation(QStandardPaths::CacheLocation))){
            help_browser::help = new QHelpEngineCore(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/doc/doc.qhc");
        }
        else{
            return false;
        }
        bool setup = help_browser::help->setupData(); //Returns the value if the data was setup correctly.
        //print list of documents in help:
        QStringList docs = help_browser::help->registeredDocumentations();
        for (auto i = docs.cbegin(); i != docs.cend(); i++){
            //std::cout << qPrintable(*i) << std::endl;
        }
        return setup;
    }
    return true;
}

bool help_browser::cache_documentation(QString cache_dir){
    //Create QDir object to allow for path creation.
    QDir dir_manager("");
    return (dir_manager.mkpath(cache_dir + "/doc")) &&
           (QFile::exists(cache_dir + "/doc/doc.qhc") || QFile::copy(":doc.qhc", QString(cache_dir + "/doc/doc.qhc"))) &&
           (QFile::exists(cache_dir + "/doc/doc.qch") || QFile::copy(":doc.qch", QString(cache_dir + "/doc/doc.qch")));
}

//Load file from QUrl, or default index if url passed = NULL.
bool help_browser::setup_viewer(QUrl help_link){
    if (!help_browser::viewer)
        help_browser::viewer = new help_browser::help_viewer();
    help_browser::viewer->setFixedSize(1000,500); //Set window dimensions.
    help_browser::viewer->setSource(help_link);
    help_browser::viewer->show();
    return true;
    //TODO: implement default index loading.
}
