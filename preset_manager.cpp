#include "preset_manager.h"
#include <fstream>


PresetManager::Preset* PresetManager::insert(Preset* current, Preset* newPreset){
    if (!current){
        return newPreset;
    }
    if (current == newPreset){
        delete newPreset;
        return NULL;
    }
    else if (current < newPreset){
        PresetManager::Preset* tmp = this->insert(current->getLeaf_l(), newPreset);
        if(tmp){
            current->setLeaf_l(tmp);
            return this->balance(current);
        }
        else
            return NULL;
    }
    else{
        PresetManager::Preset* tmp = this->insert(current->getLeaf_r(), newPreset);
        if(tmp){
            current->setLeaf_r(tmp);
            return this->balance(current);
        }
        else
            return NULL;
    }
}

PresetManager::Preset* PresetManager::balance(Preset* current){
    int weight = this->getWeight();
    if(weight > 1){
        if(current->getLeaf_l() && current->getLeaf_l()->getWeight() > 0)
            current = this->rotate_r(current);
        else
            current = this->rotate_lr(current);
    }
    else if (weight < -1){
        if(current->getLeaf_r && current->getLeaf_r()->getWeight() > 0)
            current = this->rotate_rl(current);
        else
            current = this->rotate_l(current);
    }
}
PresetManager::Preset* PresetManager::rotate_l(PresetManager::Preset* trunk){

}
PresetManager::Preset* PresetManager::rotate_r(PresetManager::Preset *trunk){

}
PresetManager::Preset* PresetManager::rotate_rl(PresetManager::Preset *trunk){

}
PresetManager::Preset* PresetManager::rotate_lr(PresetManager::Preset *trunk){

}
QJsonDocument* PresetManager::readPresetFile(){
    QJsonDocument* presets = NULL;
    ifstream infile(this->getPresetPath());
    if (!infile.fail()){
        QString in_str;
        while (!infile.eof())
            in_str += infile.readline();
        infile.close();
        presets = new QJsonDocument(QJsonDocument::fromJson(in_str));
    }
    return presets;
}

bool PresetManager::writePresetFile(){
    ofstream outfile(this->getPresetPath());
    if (!outfile.fail()){
        out << qPrintable(QJsonDocument::toJson()) << endl;
        return true;
    }
    else
        return false;
}
