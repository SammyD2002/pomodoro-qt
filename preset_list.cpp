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
    for (int i = 0; i < this->preset_delete_actions.length(); i++){
        if (this->preset_delete_actions[i]->text() == ps_name.trimmed()){
            emit this->preset_removed(preset_load_actions[i], preset_delete_actions[i], preset_edit_actions[i], preset_rename_actions[i], preset_new_default_actions[i]);
            this->preset_load_actions.remove(i);
            this->preset_delete_actions.remove(i);
            this->preset_edit_actions.remove(i);
            this->preset_rename_actions.remove(i);
            this->preset_new_default_actions.remove(i);
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
    this->preset_load_actions[old]->setText(new_name);
    this->preset_delete_actions[old]->setText(new_name);
    this->preset_edit_actions[old]->setText(new_name);
    this->preset_rename_actions[old]->setText(new_name);
    return true;
}
//Find index of the updated preset, and call the more robust update_preset function.
bool PresetManager::update_preset(QJsonObject preset, bool overwrite){
    return this->update_preset(preset, this->findPreset(preset["preset_name"].toString()), overwrite);
}

//The input is validated and confirmed in the editor function(s).
bool PresetManager::update_preset(QJsonObject preset, int index, bool overwrite){
    if(index >= 0){
        if(overwrite){
            this->presets->replace(index, preset);
            return true;
        } else
            return false;
    }
    else{
        this->presets->append(preset);
        this->preset_load_actions.append(new QAction(this->presets->last().toObject()["preset_name"].toString()));
        this->preset_delete_actions.append(new QAction(this->presets->last().toObject()["preset_name"].toString()));
        this->preset_edit_actions.append(new QAction(this->presets->last().toObject()["preset_name"].toString()));
        this->preset_rename_actions.append(new QAction(this->presets->last().toObject()["preset_name"].toString()));
        this->preset_new_default_actions.append(new QAction(this->presets->last().toObject()["preset_name"].toString()));
        emit this->preset_added(this->preset_load_actions.last(), this->preset_delete_actions.last(), this->preset_edit_actions.last(), this->preset_rename_actions.last(), this->preset_new_default_actions.last());
        return true;
    }
}
