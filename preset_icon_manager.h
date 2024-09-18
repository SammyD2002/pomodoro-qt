/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#ifndef PRESET_ICON_MANAGER_H
#define PRESET_ICON_MANAGER_H
#include "widgets.h"
#include "icon_builder.h"
#include "preset_manager.h"
struct icon_preview{
    QString icon_name;
    const QIcon* icon_base;
};

class PresetIconManager : public QObject
{
Q_OBJECT
public:
    PresetIconManager(int partitions, QObject* parent = nullptr);
    ~PresetIconManager();
    bool add_icon(const QIcon &icon, QString preset, int urgency = -1){return this->add_icon(new const QIcon(icon), preset, urgency);}
    bool add_icon(const QIcon* icon, QString preset, int urgency = -1); //Returns false if icon is present and has all icons built. Urgency is where to insert the icon's name into the list.
    const QIcon *get_icon(QString icon_name, int complete);
    bool isAdded(QString filename) const {return this->preset_icons.constFind(filename) != this->preset_icons.constEnd();}
    QList<icon_preview*> get_all_bases();
private:
    int sets_complete = 0;
    int icon_partitions;
    int last_percent_applied = -1;
    QString last_icon_applied = QStringLiteral("");
    icon_builder* builder = nullptr;
    QMap<QString, QList<const QIcon*>> preset_icons;
    QStringList to_build;
    void start_set();
private slots:
    void icon_built(const QIcon*, QString); //Append the newly built icon to the list.
    void set_complete(); //Stop the builder thread, set it to be deleted later, and start the next icon set's thread.
};

#endif // PRESET_ICON_MANAGER_H
