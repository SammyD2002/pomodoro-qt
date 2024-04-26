#include <cstring>
#include "preset_manager.h"
using namespace std;
//preset_error
//const char* PresetManager::preset_error::issue_bridge = " for the preset ".c_str();
PresetManager::preset_error::preset_error(string name) : runtime_error("Preset " + name){
    this->preset_name = name.c_str();
    this->preset_attribute = nullptr;
    this->preset_issue = ("Preset " + name).c_str();
}

PresetManager::preset_error::preset_error(string name, json_value_error err) : runtime_error("Preset " + name + " is " + err.what()){
    this->preset_name = name.c_str();
    this->preset_attribute = nullptr;
    this->preset_issue = ("Preset " + name + " is " + err.what()).c_str();
}

PresetManager::preset_error::preset_error(std::string name, std::string attribute, json_value_error err) : runtime_error("Preset " + name + "/" + attribute + " is " + err.what()){
    this->preset_name = name.c_str();
    this->preset_attribute = attribute.c_str();
    this->preset_issue = ("Preset " + name + "/" + attribute + " is " + err.what()).c_str();
}
/*
const char* PresetManager::preset_error::what() const noexcept override{
    return this->preset_issue;
}
*/
//json_value_error
PresetManager::json_value_error::json_value_error(string name) : runtime_error(name){
    this->issue = name.c_str();
}
/*
const char* PresetManager::json_value_error::what() const noexcept override{
    return this->issue;
}*/
