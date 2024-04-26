/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#include "preset_manager.h"
//This file should handle methods related to searching and modifying the list of presets.
//Find the preset <preset_name>, and return its index.
int PresetManager::findPreset(QString preset_name) const{
    int index = 0;
    for (QJsonArray::const_iterator i = this->presets->cbegin(); i != this->presets->cend(); i++){
        if((*i).toObject()["preset_name"] == preset_name)
            return index;
        else
            index++;
    }
    return -1;
}
//load the preset <ps_name>
bool PresetManager::loadPreset(QString ps_name){
    int index = this->findPreset(ps_name);
    if (index >= 0){
        emit this->presetLoaded(new QJsonObject((*(this->presets))[index].toObject()));
        return true;
    }
    return false;
}
//remove the preset <ps_name>
bool PresetManager::removePreset(QString ps_name){
    int index = this->findPreset(ps_name);
    if(index >= 0){
        this->presets->removeAt(index);
    }
    else{
        return false;
    }
    for (int i = 0; i < this->preset_actions[0].length(); i++){
        if (this->preset_actions[0][i] != nullptr && this->preset_actions[0][i]->text() == ps_name.trimmed()){
            QAction* actions[6];
            for(int j = 0; j < 6; j++)
                actions[j] = preset_actions[j][i];
            emit this->preset_removed(actions);
            for(int j = 0; j < 6; j++)
                this->preset_actions[j].remove(i);
            return true;
        }
    }
    return true;
}
//Replace the object at index <old> with the new_settings passed.
bool PresetManager::rename_preset(int old, QJsonObject new_settings, bool overwrite){
    QString new_name = new_settings["preset_name"].toString();
    if (!overwrite && this->findPreset(new_name) >= 0)
        return false;
    this->presets->replace(old, new_settings);
    this->update_menu_labels(old, new_name);
    return true;
}

void PresetManager::update_menu_labels(int old, QString new_name){
    for(int i = 0; i < 6; i++)
        preset_actions[i][old]->setText(new_name);
}

//The input is validated and confirmed in the editor function(s). index = -1 if it is not present yet.
bool PresetManager::create_preset(QJsonObject preset, bool overwrite){
    return this->create_preset(preset, this->findPreset(preset.value("preset_name").toString()), overwrite);
}

//The input is validated and confirmed in the editor function(s). index = -1 if it is not present yet.
bool PresetManager::create_preset(QJsonObject preset, int index, bool overwrite){
    if(index >= 0){
        if(overwrite){
            this->presets->replace(index, preset); //TODO: update menu entries to the new_name.
            this->update_menu_labels(index, preset["preset_name"].toString());
            return true;
        } else
            return false;
    }
    else{
        this->presets->append(preset);
        QAction* added[6];
        for(int i = 0; i < 6; i++){
            added[i] = new QAction(this->presets->last().toObject()["preset_name"].toString());
            this->preset_actions[i].append(added[i]);
        }
        emit this->preset_added(added);
        return true;
    }
}

//Find index of the updated preset, and call the more robust update_preset function.
bool PresetManager::update_preset(QString old, QJsonObject preset, bool overwrite){
    return this->update_preset(old, preset, this->findPreset(preset["preset_name"].toString()), overwrite);
}
//The input is validated and confirmed in the editor function(s). index = -1 if it is not present yet.
bool PresetManager::update_preset(QString old, QJsonObject preset, int index, bool overwrite){
    int old_i = this->findPreset(old);
    if(!overwrite && index >= 0 && old_i != index){
        return false;
    }
    else{
        this->presets->replace(old_i, preset); //TODO: update menu entries to the new_name.
        this->update_menu_labels(old_i, preset["preset_name"].toString());
        //emit this->preset_removed(preset_load_actions[old_i], preset_delete_actions[old_i], preset_edit_actions[old_i], preset_rename_actions[old_i], preset_new_default_actions[old_i]);
        //emit this->preset_added(this->preset_load_actions[old_i], this->preset_delete_actions[old_i], this->preset_edit_actions[old_i], this->preset_rename_actions[old_i], this->preset_new_default_actions[old_i]);
        return true;
    }
}
