--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-diskman>
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0
$Id$
]]--

require "luci.util"
local ver = require "luci.version"

local CMD = {"parted", "mdadm", "blkid", "smartctl", "df", "sgdisk", "btrfs"}

local d = {command ={}}
for _, cmd in ipairs(CMD) do
  local command = luci.sys.exec("/usr/bin/which " .. cmd)
    d.command[cmd] = command:match("^.+"..cmd) or nil
end

local mounts = nixio.fs.readfile("/proc/mounts") or ""
local swaps = nixio.fs.readfile("/proc/swaps") or ""
local df = luci.sys.exec(d.command.df) or ""

function byte_format(byte)
  local suff = {"B", "KB", "MB", "GB", "TB"}
  for i=1, 5 do
    if byte > 1024 and i < 5 then
      byte = byte / 1024
    else
      return string.format("%.2f %s", byte, suff[i]) 
    end 
  end
end

local get_smart_info = function(device)
  local section
  local smart_info = {}
  for _, line in ipairs(luci.util.execl(d.command.smartctl .. " -H -A -i -n standby -f brief /dev/" .. device)) do
    local attrib, val
    if section == 1 then
        attrib, val = line:match "^(.-):%s+(.+)"
    elseif section == 2 and device:match("nvme") then
      attrib, val = line:match("^(.-):%s+(.+)")
      if not smart_info.health then smart_info.health = line:match(".-overall%-health.-: (.+)") end
    elseif section == 2 then
      attrib, val = line:match("^([0-9 ]+)%s+[^ ]+%s+[POSRCK-]+%s+[0-9-]+%s+[0-9-]+%s+[0-9-]+%s+[0-9-]+%s+([0-9-]+)")
      if not smart_info.health then smart_info.health = line:match(".-overall%-health.-: (.+)") end
    else
      attrib = line:match "^=== START OF (.*) SECTION ==="
      if attrib and attrib:match("INFORMATION") then
        section = 1
      elseif attrib and attrib:match("SMART DATA") then
        section = 2
      elseif not smart_info.status then
        val = line:match "^Device is in (.*) mode"
        if val then smart_info.status = val end
      end
    end

    if not attrib then
      if section ~= 2 then section = 0 end
    elseif (attrib == "Power mode is") or
      (attrib == "Power mode was") then
        smart_info.status = val:match("(%S+)")
    -- elseif attrib == "Sector Sizes" then
    --   -- 512 bytes logical, 4096 bytes physical
    --   smart_info.phy_sec = val:match "([0-9]*) bytes physical"
    --   smart_info.logic_sec = val:match "([0-9]*) bytes logical"
    -- elseif attrib == "Sector Size" then
    --   -- 512 bytes logical/physical
    --   smart_info.phy_sec = val:match "([0-9]*)"
    --   smart_info.logic_sec = smart_info.phy_sec
    elseif attrib == "Serial Number" then
      smart_info.sn = val
    elseif attrib == "194" or attrib == "Temperature" then
      smart_info.temp = val:match("(%d+)") .. "°C"
    elseif attrib == "Rotation Rate" then
      smart_info.rota_rate = val
    elseif attrib == "SATA Version is" then
      smart_info.sata_ver = val
    end
  end
  return smart_info
end

local parse_parted_info = function(keys, line)
  -- parse the output of parted command (machine parseable format)
  -- /dev/sda:5860533168s:scsi:512:4096:gpt:ATA ST3000DM001-1ER1:;
  -- 1:34s:2047s:2014s:free;
  -- 1:2048s:1073743872s:1073741825s:ext4:primary:;
  local result = {}
  local values = {}

  for value in line:gmatch("(.-)[:;]") do table.insert(values, value) end
  for i = 1,#keys do
    result[keys[i]] = values[i] or ""
  end
  return result
end

local is_raid_member = function(partition)
  -- check if inuse as raid member
  if nixio.fs.access("/proc/mdstat") then
    for _, result in ipairs(luci.util.execl("grep md /proc/mdstat | sed 's/[][]//g'")) do
      local md, buf
      md, buf = result:match("(md.-):(.+)")
      if buf:match(partition) then
        return "Raid Member: ".. md
      end
    end
  end
  return nil
end

local get_mount_point = function(partition)
  local mount_point, dk_root_dir
  -- if use luci-in-dokcer, exclude the docker overlay mounts
  -- if ver.distname == "LuCI in Docker" then
    local _o, dk = pcall(require,"luci.docker")
    if _o and dk then
      dk_root_dir = dk.new():info().body.DockerRootDir
    end
  -- end

  for m in mounts:gmatch("/dev/"..partition.." ([^ ]*)") do
    if dk_root_dir and m:match(dk_root_dir) then
    else
      mount_point = (mount_point and (mount_point .. " ")  or "") .. m
    end
  end
  if mount_point then return mount_point end
  -- result = luci.sys.exec('cat /proc/mounts | awk \'{if($1=="/dev/'.. partition ..'") print $2}\'')
  -- if result ~= "" then return result end

  if swaps:match("\n/dev/" .. partition .."%s") then return "swap" end
  -- result = luci.sys.exec("cat /proc/swaps | grep /dev/" .. partition)
  -- if result ~= "" then return "swap" end

  return is_raid_member(partition)

end

local get_partition_useage = function(partition)
  if not nixio.fs.access("/dev/"..partition) then return false end
  local useage = df:match("\n/dev/" .. partition .. "%s+%d+%s+%d+%s+%d+%s+([0-9]+)%%%s")
  useage = useage and (useage .. "%") or false
  return useage
end

local get_parted_info = function(device)
  if not device then return end
  local result = {partitions={}}
  local DEVICE_INFO_KEYS = { "path", "size", "type", "logic_sec", "phy_sec", "p_table", "model", "flags" }
  local PARTITION_INFO_KEYS = { "number", "sec_start", "sec_end", "size", "fs", "tag_name", "flags" }
  local partition_temp
  local partitions_temp = {}
  local disk_temp

  for line in luci.util.execi(d.command.parted .. " -s -m /dev/" .. device .. " unit s print free", "r") do
    if line:find("^/dev/"..device..":.+") then
      disk_temp = parse_parted_info(DEVICE_INFO_KEYS, line)
      disk_temp.partitions = {}
      if disk_temp["size"] then
        local length = disk_temp["size"]:gsub("^(%d+)s$", "%1")
        local newsize = tostring(tonumber(length)*tonumber(disk_temp["logic_sec"]))
        disk_temp["size"] = newsize
      end
      if disk_temp["p_table"] == "msdos" then
        disk_temp["p_table"] = "MBR"
      else
        disk_temp["p_table"] = disk_temp["p_table"]:upper()
      end
    elseif line:find("^%d-:.+") then
      partition_temp = parse_parted_info(PARTITION_INFO_KEYS, line)
      -- use human-readable form instead of sector number
      if partition_temp["size"] then
        local length = partition_temp["size"]:gsub("^(%d+)s$", "%1")
        local newsize = (tonumber(length) * tonumber(disk_temp["logic_sec"]))
        partition_temp["size"] = newsize
        partition_temp["size_formated"] = byte_format(newsize)
      end
      partition_temp["number"] = tonumber(partition_temp["number"]) or -1
      if partition_temp["fs"] == "free" then
        partition_temp["number"] = -1
        partition_temp["fs"] = "Free Space"
        partition_temp["name"] = "-"
      elseif device:match("sd") or device:match("sata") then
        partition_temp["name"] = device..partition_temp["number"]
      elseif device:match("mmcblk") or device:match("md") or device:match("nvme") then
        partition_temp["name"] = device.."p"..partition_temp["number"]
      end
      partition_temp["fs"] = partition_temp["fs"] == "" and "raw" or partition_temp["fs"]
      partition_temp["sec_start"] = partition_temp["sec_start"] and partition_temp["sec_start"]:sub(1,-2)
      partition_temp["sec_end"] = partition_temp["sec_end"] and partition_temp["sec_end"]:sub(1,-2)
      partition_temp["mount_point"] = partition_temp["name"]~="-" and get_mount_point(partition_temp["name"]) or "-"
      partition_temp["useage"] = partition_temp["mount_point"]~="-" and get_partition_useage(partition_temp["name"]) or "-"
      -- if disk_temp["p_table"] == "MBR" and (partition_temp["number"] < 4) and (partition_temp["number"] > 0) then
      --   local real_size_sec = tonumber(nixio.fs.readfile("/sys/block/"..device.."/"..partition_temp["name"].."/size")) * tonumber(disk_temp.phy_sec)
      --   if real_size_sec ~= partition_temp["size"] then
      --     disk_temp["extended_partition_index"] = partition_temp["number"]
      --     partition_temp["type"] = "extended"
      --     partition_temp["size"] = real_size_sec
      --     partition_temp["fs"] = "-"
      --     partition_temp["logicals"] = {}
      --   else
      --     partition_temp["type"] = "primary"
      --   end
      -- end

      table.insert(partitions_temp, partition_temp)
    end
  end
  if disk_temp["p_table"] == "MBR" then
    for i, p in ipairs(partitions_temp) do
      if disk_temp["extended_partition_index"] and p["number"] > 4 then
        if tonumber(p["sec_end"]) <= tonumber(partitions_temp[disk_temp["extended_partition_index"]]["sec_end"]) and tonumber(p["sec_start"]) >= tonumber(partitions_temp[disk_temp["extended_partition_index"]]["sec_start"]) then
          p["type"] = "logical"
          table.insert(partitions_temp[disk_temp["extended_partition_index"]]["logicals"], i)
        end
      elseif (p["number"] < 4) and (p["number"] > 0) then
        local real_size_sec = tonumber(nixio.fs.readfile("/sys/block/"..device.."/"..p["name"].."/size")) * tonumber(disk_temp.phy_sec)
        if real_size_sec ~= p["size"] then
          disk_temp["extended_partition_index"] = i
          p["type"] = "extended"
          p["size"] = real_size_sec
          p["fs"] = "-"
          p["logicals"] = {}
        else
          p["type"] = "primary"
        end
      end
    end
  end
  result = disk_temp
  result.partitions = partitions_temp

  return result
end

local mddetail = function(mdpath)
	local detail = {}
	local path = mdpath:match("^/dev/md%d+$")
	if path then
		local mdadm = io.popen(d.command.mdadm .. " --detail "..path, "r")
		for line in mdadm:lines() do
			local key, value = line:match("^%s*(.+) : (.+)")
			if key then
				detail[key] = value
			end
		end
		mdadm:close()
	end
	return detail
end

-- return {{device="", mount_points="", fs="", mount_options="", dump="", pass=""}..}
d.get_mount_points = function()
  local mount, dk_root_dir
  local res = {}
  local h ={"device", "mount_point", "fs", "mount_options", "dump", "pass"}
  local _o, dk = pcall(require,"luci.docker")
  if _o and dk then
    dk_root_dir = dk.new():info().body.DockerRootDir
  end
  for mount in mounts:gmatch("[^\n]+") do
    local device = mount:match("^([^%s]+)%s+.+")
    -- only show /dev/xxx device
    if device and device:match("/dev/") then
      -- not show docker root dir mounts
      if dk_root_dir and mount and mount:match(dk_root_dir) then
      else
        res[#res+1] = {}
        local i = 0
        for v in mount:gmatch("[^%s]+") do
          i = i + 1
          res[#res][h[i]] = v
        end
      end
    end
  end
  return res
end

d.get_disk_info = function(device, wakeup)
  --[[ return:
  {
    path, model, sn, size, size_mounted, flags, type, temp, p_table, logic_sec, phy_sec, sec_size, sata_ver, rota_rate, status, health,
    partitions = {
      1 = { number, name, sec_start, sec_end, size, size_mounted, fs, tag_name, type, flags, mount_point, useage },
      2 = { number, name, sec_start, sec_end, size, size_mounted, fs, tag_name, type, flags, mount_point, useage },
      ...
    }
    --raid devices only
    level, members, members_str
  }
  --]]
  if not device then return end
  local disk_info
  local smart_info = get_smart_info(device)

  -- check if divice is the member of raid
  smart_info["p_table"] = is_raid_member(device..'0')
  -- if status is not active(standby), only check smart_info.
  -- if only weakup == true, weakup the disk and check parted_info.
  if smart_info.status ~= "STANDBY" or wakeup or (smart_info["p_table"] and not smart_info["p_table"]:match("Raid")) or device:match("^md") then
    disk_info = get_parted_info(device)
    disk_info["sec_size"] = disk_info["logic_sec"] .. "/" .. disk_info["phy_sec"]
    disk_info["size_formated"] = byte_format(tonumber(disk_info["size"]))
    -- if status is standby, then get smart_info again
    if smart_info.status ~= "ACTIVE" then smart_info = get_smart_info(device) end
  else
    disk_info = {}
  end

  for k, v in pairs(smart_info) do
    disk_info[k] = v
  end

  if disk_info.type and disk_info.type:match("md") then
    local raid_info = d.list_raid_devices()[disk_info["path"]:match("/dev/(.+)")]
    for k, v in pairs(raid_info) do
      disk_info[k] = v
    end
  end
  return disk_info
end

d.list_raid_devices = function()
  local fs = require "nixio.fs"

  local raid_devices = {}
  if not fs.access("/proc/mdstat") then return raid_devices end
  local mdstat = io.open("/proc/mdstat", "r")
  for line in mdstat:lines() do

    -- md1 : active raid1 sdb2[1] sda2[0]
    -- md127 : active raid5 sdh1[6] sdg1[4] sdf1[3] sde1[2] sdd1[1] sdc1[0]
    local device_info = {}
    local mdpath, list = line:match("^(md%d+) : (.+)")
    if mdpath then
      local members = {}
      for member in string.gmatch(list, "%S+") do
        member_path = member:match("^(%S+)%[%d+%]")
        if member_path then
          member = '/dev/'..member_path
        end
        table.insert(members, member)
      end
      local active = table.remove(members, 1)
      local level = "-"
      if active == "active" then
        level = table.remove(members, 1)
      end

      local size = tonumber(fs.readfile(string.format("/sys/class/block/%s/size", mdpath)))
      local ss = tonumber(fs.readfile(string.format("/sys/class/block/%s/queue/logical_block_size", mdpath)))

      device_info["path"] = "/dev/"..mdpath
      device_info["size"] = size*ss
      device_info["size_formated"] = byte_format(size*ss)
      device_info["active"] = active:upper()
      device_info["level"] = level
      device_info["members"] = members
      device_info["members_str"] = table.concat(members, ", ")

      -- Get more info from output of mdadm --detail
      local detail = mddetail(device_info["path"])
      device_info["status"] = detail["State"]:upper()

      raid_devices[mdpath] = device_info
    end
  end
  mdstat:close()

  return raid_devices
end

-- Collect Devices information
  --[[ return:
  {
    sda={
      path, model, inuse, size_formated,
      partitions={ 
        { name, inuse, size_formated }
        ...
      }
    }
    ..
  }
  --]]
d.list_devices = function()
  local fs = require "nixio.fs"

  -- get all device names (sdX and mmcblkX)
  local target_devnames = {}
  for dev in fs.dir("/dev") do
    if dev:match("^sd[a-z]$")
      or dev:match("^mmcblk%d+$")
      or dev:match("^sata[a-z]$")
      or dev:match("^nvme%d+n%d+$")
      then
      table.insert(target_devnames, dev)
    end
  end

  local devices = {}
  for i, bname in pairs(target_devnames) do
    local device_info = {}
    local device = "/dev/" .. bname
    local size = tonumber(fs.readfile(string.format("/sys/class/block/%s/size", bname)))
    local ss = tonumber(fs.readfile(string.format("/sys/class/block/%s/queue/logical_block_size", bname)))
    local model = fs.readfile(string.format("/sys/class/block/%s/device/model", bname))
    local partitions = {}
    for part in nixio.fs.glob("/sys/block/" .. bname .."/" .. bname .. "*") do
      local pname = nixio.fs.basename(part)
      local psize = byte_format(tonumber(nixio.fs.readfile(part .. "/size"))*ss)
      local mount_point = get_mount_point(pname)
      if mount_point then device_info["inuse"] = true end
      table.insert(partitions, {name = pname, size_formated = psize, inuse = mount_point})
    end

      device_info["path"] = device
      device_info["size_formated"] = byte_format(size*ss)
      device_info["model"] = model
      device_info["partitions"] = partitions
      -- true or false
      device_info["inuse"] = device_info["inuse"] or get_mount_point(bname)

      local udevinfo = {}
      if luci.sys.exec("which udevadm") ~= "" then
        local udevadm = io.popen("udevadm info --query=property --name="..device)
        for attr in udevadm:lines() do
          local k, v = attr:match("(%S+)=(%S+)")
          udevinfo[k] = v
        end
        udevadm:close()

        device_info["info"] = udevinfo
        if udevinfo["ID_MODEL"] then device_info["model"] = udevinfo["ID_MODEL"] end
      end
      devices[bname] = device_info
  end
  -- luci.util.perror(luci.util.serialize_json(devices))
  return devices
end

-- get formart cmd
d.get_format_cmd = function()
  local AVAILABLE_FMTS = {
    ext2 = { cmd = "mkfs.ext2", option = "-F -E lazy_itable_init=1" },
    ext3 = { cmd = "mkfs.ext3", option = "-F -E lazy_itable_init=1" },
    ext4 = { cmd = "mkfs.ext4", option = "-F -E lazy_itable_init=1" },
    fat32 = { cmd = "mkfs.fat", option = "-F 32 -I" },
    exfat = { cmd = "mkexfat", option = "-f" },
    hfsplus = { cmd = "mkhfs", option = "-f" },
    ntfs = { cmd = "mkntfs", option = "-f" },
    swap = { cmd = "mkswap", option = "-f" },
    btrfs = { cmd = "mkfs.btrfs", option = "-f" }
  }
  result = {}
  for fmt, obj in pairs(AVAILABLE_FMTS) do
    local cmd = luci.sys.exec("/usr/bin/which " .. obj["cmd"])
    if cmd:match(obj["cmd"]) then
      result[fmt] = { cmd = cmd:match("^.+"..obj["cmd"]) ,option = obj["option"] }
    end
  end
  return result
end

d.create_raid = function(rname, rlevel, rmembers)
  local mb = {}
  for _, v in ipairs(rmembers) do
    mb[v]=v
  end
  rmembers = {}
  for _, v in pairs(mb) do
    table.insert(rmembers, v)
  end
  if type(rname) == "string" then
    if rname:match("^md%d-%s+") then
      rname = "/dev/"..rname:match("^(md%d-)%s+")
    elseif rname:match("^/dev/md%d-%s+") then
      rname = "/dev/"..rname:match("^(/dev/md%d-)%s+")
    elseif not rname:match("/") then
      rname = "/dev/md/".. rname
    else
      return "ERR: Invalid raid name"
    end
  else
    local mdnum = 0
    for num=1,127 do
      local md = io.open("/dev/md"..tostring(num), "r")
      if md == nil then
        mdnum = num
        break
      else
        io.close(md)
      end
    end
    if mdnum == 0 then return "ERR: Cannot find proper md number" end
    rname = "/dev/md"..mdnum
  end

  if rlevel == "5" or rlevel == "6" then
    if #rmembers < 3 then return "ERR: Not enough members" end
  end
  if rlevel == "10" then
    if #rmembers < 4 then return "ERR: Not enough members" end
  end
  if #rmembers < 2 then return "ERR: Not enough members" end
  local cmd = d.command.mdadm .. " --create "..rname.." --run --assume-clean --homehost=any --level=" .. rlevel .. " --raid-devices=" .. #rmembers .. " " .. table.concat(rmembers, " ")
  local res = luci.util.exec(cmd)
  return res
end

d.gen_mdadm_config = function()
  if not nixio.fs.access("/etc/config/mdadm") then return end
  local uci = require "luci.model.uci"
  local x = uci.cursor()
  -- delete all array sections
  x:foreach("mdadm", "array", function(s) x:delete("mdadm",s[".name"]) end)
  local cmd = d.command.mdadm .. " -D -s"
  --ARRAY /dev/md1 metadata=1.2 name=any:1 UUID=f998ae14:37621b27:5c49e850:051f6813
  --ARRAY /dev/md3 metadata=1.2 name=any:3 UUID=c068c141:4b4232ca:f48cbf96:67d42feb
  for _, v in ipairs(luci.util.execl(cmd)) do
    local device, uuid = v:match("^ARRAY%s-([^%s]+)%s-[^%s]-%s-[^%s]-%s-UUID=([^%s]+)%s-")
    if device and uuid then
      local section_name = x:add("mdadm", "array")
      x:set("mdadm", section_name, "device", device)
      x:set("mdadm", section_name, "uuid", uuid)
    end
  end
  x:commit("mdadm")
  -- enable mdadm
  luci.util.exec("/etc/init.d/mdadm enable")
end

return d
