#ifndef HELP_BROWSER_H
#define HELP_BROWSER_H
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
    static void load_help(QString component, QWidget* parent);
    //Updates the parent widget of the viewer to allow for it to be used during the modal timer config dialog.
private:
    //member functions
    static bool setup_help();
    //Load file from QUrl
    static bool setup_viewer(QUrl help_link, QWidget* parent);
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
