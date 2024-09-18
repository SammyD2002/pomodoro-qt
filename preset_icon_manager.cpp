/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#include "preset_icon_manager.h"
/*This class manages icons loaded into the program as follows:
 * 1. Init with no icons, only add them via a function. (Ran at program startup for defaults).
 *  - Map icons to filename. Requires list of icons to be changed to QMap to list of icons.
 *  - Icons added to some form of queue?
 * 2. Send const Preset Object with status and complete in get_icon().
 *  - Requires icon paths to be added as a string array to be added to the json object.
 */
PresetIconManager::PresetIconManager(int partitions, QObject *parent) : QObject(parent), icon_partitions(partitions) {}

PresetIconManager::~PresetIconManager(){
    //Force early exit of builder thread if running.
    if(this->builder != nullptr){
        this->builder->exit();
        this->builder->deleteLater();
    }
    for (QMap<QString, QList<const QIcon*>>::iterator i = this->preset_icons.begin(); i != this->preset_icons.end(); i++){
        QList<const QIcon*> icon_set = i.value();
        for(QList<const QIcon*>::iterator j = icon_set.begin(); j != icon_set.end(); j++)
            delete *j;
    }
}
//Returns false if icon is present and has all icons built, or is being built and urgency = -1.
//Urgency is where to insert the icon's name into the list.
bool PresetIconManager::add_icon(const QIcon *icon, QString preset, int urgency){
    bool enqueued = this->to_build.contains(preset);
    //If the preset hasn't been added to the manager before now, add it.
    if(!this->isAdded(preset)){
        this->preset_icons[preset].insert(0, icon);
        enqueued = this->to_build.contains(preset);
    }
    //Otherwise, if the icon has already been added to the manager, and...
    //Has been built already or...
    //Is queued to be built & urgency is -1.
    else if(!enqueued || urgency == -1){
        return false;
    }
    //If enqueued and the urgency was specified, remove all icons from the queue.
    if (enqueued){
        this->to_build.removeAll(preset);
    }
    //Enqueue at the end of the queue if urgency = -1 or is larger than the list.
    if(urgency == -1 || urgency > this->to_build.length())
        this->to_build.append(preset);
    //Otherwise, insert it at the end of the list.
    else
        this->to_build.insert(urgency, preset);
    //If the builder is a nullptr, it is not active, and start_set needs to be ran manually.
    if(this->builder == nullptr)
        this->start_set();
    return true;

}

//status = 0,3,5 set the study icon, 1 & 2 set the break icon. 4 Is the end of the cycle, and doesn't alter the icon.
const QIcon *  PresetIconManager::get_icon(QString icon_name, int complete) { //Complete = the percent complete
    //built/part = x / 100 -> built * 100 = part(x) -> built * 100 / partitions
    //%/100 = x/part -> part * % = 100x
    int percent_partition = (complete * this->icon_partitions) / 100;
    //If we are re-applying the same icon for the same %, return nullptr.
    if(this->last_percent_applied == percent_partition && icon_name == last_icon_applied)
        return nullptr;
    //Make sure the icon actually exists.
    QMap<QString, QList<const QIcon*>>::const_iterator icon_image_list = this->preset_icons.constFind(icon_name);
    if(icon_image_list == this->preset_icons.cend())
        throw PresetManager::preset_error((std::stringstream() << "BUG: " << qPrintable(icon_name) << " was not loaded for some reason.").str().c_str());
    QList<const QIcon*> icon_images = icon_image_list.value();
    if(percent_partition > icon_images.length() - 1)
        percent_partition = icon_images.length() - 1;
    qInfo("%s", (std::stringstream() << "Icon: " << qPrintable(icon_name) << ", Percent: " << complete << ", Icon Number: " << percent_partition).str().c_str());
    this->last_percent_applied = percent_partition;
    this->last_icon_applied = icon_name;
    const QIcon* const ic = icon_images[percent_partition];
    return ic;
}

QList<icon_preview *> PresetIconManager::get_all_bases(){
    QList<icon_preview*> bases;
    for(QMap<QString, QList<const QIcon*>>::const_iterator i = this->preset_icons.constBegin(); i != this->preset_icons.constEnd(); i++){
        bases.append(new icon_preview);
        bases.last()->icon_name = i.key();
        bases.last()->icon_base = i.value().at(0);
    }
    return bases;
}

void PresetIconManager::start_set(){
    if(!this->to_build.isEmpty()){
        QString icon_path = this->to_build.takeFirst();
        this->builder = new icon_builder(this->preset_icons[icon_path][0], icon_path, this->icon_partitions);
        connect(this->builder, &icon_builder::icon_built, this, &PresetIconManager::icon_built, Qt::QueuedConnection);
        connect(this->builder, &icon_builder::set_complete, this, &PresetIconManager::set_complete, Qt::QueuedConnection);
        qDebug("%s", (std::stringstream() << "Building set for icon with path " << qPrintable(icon_path) << " across " << this->icon_partitions << " partitions").str().c_str());
        this->builder->start();
    }
}

void PresetIconManager::icon_built(const QIcon *varient, QString icon_name){
    this->preset_icons[icon_name].append(varient);
}

void PresetIconManager::set_complete(){
        qDebug("Done building set.");
    this->builder = nullptr;
    this->start_set(); //Emptyness checked in start_set().
}
