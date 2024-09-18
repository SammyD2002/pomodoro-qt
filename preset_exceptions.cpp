/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#include <cstring>
#include "preset_manager.h"
using namespace std;
//preset_error
//const char* PresetManager::preset_error::issue_bridge = " for the preset ".c_str();
//Creates a preset error specifying a preset by name.
PresetManager::preset_error::preset_error(string name) : runtime_error("Preset " + name){
    this->preset_name = name.c_str();
    this->preset_attribute = nullptr;
    this->preset_issue = ("Preset " + name).c_str();
}

//Creates a preset_error specifying a name & a json_value_error with the problem attribute and issue.
PresetManager::preset_error::preset_error(string name, json_value_error err) : runtime_error("Preset " + name + " is " + err.what()){
    this->preset_name = name.c_str();
    this->preset_attribute = nullptr;
    this->preset_issue = ("Preset " + name + " is " + err.what()).c_str();
}

//Creates a preset_error by specifying a name and attribute and gets the attribute and issue from a json_value_error.
PresetManager::preset_error::preset_error(std::string name, std::string attribute, json_value_error err) : runtime_error("Preset " + name + "/" + attribute + " is " + err.what()){
    this->preset_name = name.c_str();
    this->preset_attribute = attribute.c_str();
    this->preset_issue = ("Preset " + name + "/" + attribute + " is " + err.what()).c_str();
}

//json_value_error: Thrown if there is an issue with the attribute <name>
PresetManager::json_value_error::json_value_error(string name) : runtime_error(name){
    this->issue = name.c_str();
}

