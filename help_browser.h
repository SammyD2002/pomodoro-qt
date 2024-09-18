/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#ifndef HELP_BROWSER_H
#define HELP_BROWSER_H
//Skip the rest of the widgets.
#include <QHelpEngineCore>
#include <QUrl>
#include <QtHelp/QHelpLink>
#include <QList>
#include <QTextBrowser>
#include <QDialog>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
//This is an attempt to create a static help browser class.
struct QHelpLink;
class QHelpEngine;
class QTextBrowser;
class help_browser : QObject
{
    Q_OBJECT
public:
    //static method to load a help file.
    static void load_help(QString component);
    //Updates the parent widget of the viewer to allow for it to be used during the modal timer config dialog.
private:
    //Default constructor to prevent object from being created outside of the setup_help function.
    help_browser() {}
    //member functions
    static bool setup_help();
    //Load file from QUrl
    static bool setup_viewer(QUrl help_link);
    static bool cache_documentation(QString cache_dir);
    //member attributes
    //Class with method to override the loading of help docs.
    class help_viewer : public QTextBrowser{
    public:
        help_viewer(QWidget* parent = nullptr);
    private:
        QVariant loadResource(int type, const QUrl &name) override;
    };
    static QHelpEngineCore* help;
    static help_viewer* viewer;
    //Need to implement method to override the way the textbrowser loads the help docs.
};
//REMINDER: Static members & functions must be defined in the .cpp file to avoid linker errors.
#endif // HELP_BROWSER_H
