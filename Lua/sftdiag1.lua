--
--  Copyright (c) Scott Simpson
--
-- 	This program is free software: you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation, either version 3 of the License, or
--  (at your option) any later version.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  A copy of the GNU General Public License is available at <http://www.gnu.org/licenses/>.
--    

local debugLabelWidth = 90
local debugRowHeight = 7
local debugColWidth = 106

initialize()

local function printFrSkyData(col, row, label, name)
    local val = getValue(name)
	local x = (col - 1) * debugColWidth
    local y = row * debugRowHeight - 6
    lcd.drawText(x, y, label, SMLSIZE)
    if val ~= nil then
      lcd.drawText(x + debugLabelWidth - 20, y, val, SMLSIZE)
    end
end

local function printExtensionData(col, row, label, name)
    local val = extensionValue[name]
	local x = (col - 1) * debugColWidth
    local y = row * debugRowHeight - 6
    lcd.drawText(x, y, label, SMLSIZE)
    if val ~= nil then
      lcd.drawText(x + debugLabelWidth - 20, y, val, SMLSIZE)
    end
end

local function printGpsData(col, row, label, name)
    local val = gpsData[name]
	local x = (col - 1) * debugColWidth
    local y = row * debugRowHeight - 6
    lcd.drawText(x, y, label, SMLSIZE)
    if val ~= nil then
      lcd.drawText(x + debugLabelWidth - 20, y, val, SMLSIZE)
    end
end

local function background() 
end

local function run(event)
	lcd.clear()

    checkForExtensionMessage()
    
	printExtensionData(1, 1, "sequence", "sequence")
	printExtensionData(1, 2, "roll_angle", "roll_angle")
	printExtensionData(1, 3, "pitch_angle", "pitch_angle")
	printExtensionData(1, 4, "hdop", "hdop")
	printExtensionData(1, 5, "fix_type", "fix_type")
	printExtensionData(1, 6, "sats_visible", "sats_visible")
	printExtensionData(1, 7, "base_mode", "base_mode")
	printExtensionData(1, 8, "custom_mode", "custom_mode")

	printExtensionData(2, 1, "bar_altitude", "bar_altitude")
	printExtensionData(2, 2, "ranger_dist", "ranger_dist")
	printExtensionData(2, 3, "batt_remain", "batt_remain")	 
	printExtensionData(2, 4, "cur_consumed", "cur_consumed")	    
end

return {run=run}