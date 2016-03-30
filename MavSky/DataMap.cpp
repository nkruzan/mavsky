//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  A copy of the GNU General Public License is available at <http://www.gnu.org/licenses/>.
//  
#include <EEPROM.h>
#include "MavSky.h"
#include "DataMap.h"
#include "MavLinkData.h"
#include "MavConsole.h"

extern MavLinkData *mav;
extern MavConsole *console;

uint8_t   default_program[] = {
  4,

  32, 0, 0, 0, 0, 0, 0,   
  32, 1, 0, 1, 1, 0, 1,   
  32, 2, 0, 2, 2, 0, 2,   
  32, 3, 0, 3, 3, 0, 3,   
  32, 4, 0, 4, 4, 0, 4,   
  32, 5, 0, 5, 5, 0, 5,   
  32, 6, 0, 6, 6, 0, 6,   
  32, 7, 0, 7, 7, 0, 7,   

  1, 0, 0, 0x20, 0, 0,   
  48, 0, 
  48, 1, 
  48, 2, 
  48, 3, 
  48, 4, 
  48, 5, 
  48, 6, 
  48, 7
};

DataMap::DataMap() {
  source_name[MAP_SOURCE_RANGEFINDER_DISTANCE] = (char*)"rangefinder_distance";
  source_name[MAP_SOURCE_BAR_ALTITUDE] = (char*)"bar_altitude";

  target_name[MAP_TARGET_VARIO_ALTITUDE] = (char*)"vario_altitude";
  
  for(uint8_t i=0; i<MAP_TARGET_COUNT; i++) {
    source_for_target[i] = EEPROM.read(EEPROM_ADDR_MAP_BEGIN + i*2);
    int8_t scale_power = EEPROM.read(EEPROM_ADDR_MAP_BEGIN + i*2 + 1);
    scale_for_target[i] = powf(10.0, scale_power);    
  }
  if(EEPROM.read(EEPROM_ADDR_VERSION) != EEPROM_INIT_VALUE_213) {
    write_factory_settings();
    EEPROM.write(EEPROM_ADDR_VERSION, EEPROM_INIT_VALUE_213);
  }
}

void DataMap::write_factory_settings() {
  EEPROM.write(EEPROM_ADDR_FRSKY_VFAS_ENABLE, 1);
  add_map((char*)"bar_altitude", (char*)"vario_altitude", (char*)"100.0");  
  add_map((char*)"climb_rate", (char*)"vario_vertical_speed", (char*)"100.0");  

  EEPROM.write(EEPROM_LED_CODE_SIZE, (sizeof(default_program) << 8) & 0xff);
  EEPROM.write(EEPROM_LED_CODE_SIZE + 1, sizeof(default_program) & 0xff);
  for(uint16_t i=0; i<sizeof(default_program); i++) {
    EEPROM.write(EEPROM_LED_CODE_BASE + i, default_program[i]);
  }  
}

void DataMap::console_map(char* p) {        // todo - don't need this param
  char* p1 = strtok(NULL, " ");
  if(p1 == NULL) {  
    console_dump_map();
  } else { 
    char* p2 = strtok(NULL, " ");
    if(p2 == NULL) { 
      console->console_print("Command requires at least two parameters.  Please retry\r\n");
    } else {
      char* p3 = strtok(NULL, " ");
      if(p3 == NULL) {
        if(!add_map(p1, p2, (char*)"1.0")) { 
          console->console_print("Command was not understood.  Please check syntax and retry\r\n"); 
        } else {
          console->console_print("%s is now mapped to %s with a scale of 1.0\r\n", p1, p2); 
        }
      } else { 
        if(!add_map(p1, p2, p3)) {
          console->console_print("Command was not understood.  Please check syntax and retry\r\n");           
        } else {
          console->console_print("%s is now mapped to %s with a scale of %s\r\n", p1, p2, p3);    
        }
      }
    }
  }
}

void DataMap::console_dump_map() {
   for(uint8_t target_index=0; target_index<MAP_TARGET_COUNT; target_index++) {
     uint8_t source_index = source_for_target[target_index];
     console->console_print("%s -> %s (scale:%f)\r\n", source_name[source_index], target_name[target_index], scale_for_target[target_index]);
  } 
}

uint8_t DataMap::add_map(char *source_name, char* target_name, char* scale_string) {
  int8_t source_index = get_source_index(source_name);
  int8_t target_index = get_target_index(target_name);
  int8_t scale_power = get_scale_power(scale_string);
  if(source_index < 0 || target_index < 0) {
    return 0;
  } else {
    source_for_target[target_index] = source_index;
    EEPROM.write(EEPROM_ADDR_MAP_BEGIN + target_index*2, source_index);  
    scale_for_target[target_index] = powf(10.0, scale_power); 
    EEPROM.write(EEPROM_ADDR_MAP_BEGIN + target_index*2 + 1, scale_power);
    return 1;
  }
}

int32_t DataMap::get_mapped_data(uint8_t target_index) {
  uint8_t source_index = source_for_target[target_index];
  int32_t raw_value = get_raw_source_value(source_index);
  float scale_value = scale_for_target[target_index];
  int32_t scaled_value = (int32_t)lroundf((float)raw_value * scale_value);
  return scaled_value; 
}

int8_t DataMap::get_source_index(char* name) {
  for(uint8_t i=0; i<MAP_SOURCE_COUNT; i++) {
    if(strcmp(source_name[i], name) == 0) {
      return i;
    }
  }
  return -1;
}

int8_t DataMap::get_target_index(char* name) {
  for(uint8_t i=0; i<MAP_TARGET_COUNT; i++) {
    if(strcmp(target_name[i], name) == 0) {
      return i;
    }
  }
  return -1;
}

int8_t DataMap::get_scale_power(char* scale_string) {
  float scale = atof(scale_string);
  float power_float = log10f(scale);
  int8_t power_int = (int8_t)lroundf(power_float);
  return power_int;
}

int32_t DataMap::get_raw_source_value(uint16_t index) {
  switch(index) {
    case MAP_SOURCE_RANGEFINDER_DISTANCE:
      return mav->rangefinder_distance;
    case MAP_SOURCE_BAR_ALTITUDE:
      return (int32_t)mav->bar_altitude;     
    default:
      break;
  }
  return 0;
}


