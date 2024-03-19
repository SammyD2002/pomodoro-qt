#include "preset_manager.h"
//Tree Balancing Functions:
int PresetManager::Preset::getWeight(){
    return PresetManager::Preset::getTreeHeight(this->preset_l) - PresetManager::Preset::getTreeHeight(this->preset_r);
}

int PresetManager::Preset::getTreeHeight(){
    int height = 0;
    int l_height = 0;
    int r_height = 0;
    if(this->preset_l){
        l_height = this->preset_l->getTreeHeight();
    }
    return height;
}
//Comparision operators
bool PresetManager::Preset::operator==(const PresetManager::Preset& rPreset){
    return this->name == rPreset.getName();
}
bool PresetManager::Preset::operator!=(const PresetManager::Preset &rPreset){
    return !(this->operator ==(rPreset));
}
bool PresetManager::Preset::operator>(const PresetManager::Preset& rPreset){
    return this->name > rPreset.getName();
}
bool PresetManager::Preset::operator<(const PresetManager::Preset& rPreset){
    return this->name < rPreset.getName();
}
bool PresetManager::Preset::operator<=(const PresetManager::Preset& rPreset){
    return (this->operator <(rPreset) || this->operator ==(rPreset));
}
bool PresetManager::Preset::operator>=(const PresetManager::Preset &rPreset){
    return (this->operator >(rPreset) || this->operator ==(rPreset));
}
