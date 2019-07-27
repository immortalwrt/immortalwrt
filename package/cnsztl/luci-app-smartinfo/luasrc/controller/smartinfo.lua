--[[
LuCI - Lua Configuration Interface - smartctl support

Script by animefans_xj @ nvacg.org (af_xj@hotmail.com , xujun@smm.cn)

Licensed under the GNU GPL License, Version 3 (the "license");
you may not use this file except in compliance with the License.
you may obtain a copy of the License at

	http://www.gnu.org/licenses/gpl.txt

$Id$
]]--


module("luci.controller.smartinfo",package.seeall)

function index()
	require("luci.i18n")
	luci.i18n.loadc("smartinfo")
	if not nixio.fs.access("/etc/config/smartinfo") then
		return
	end
	
	local page = entry({"admin","services","smartinfo"},cbi("smartinfo"),_("S.M.A.R.T Info"))
	page.i18n="smartinfo"
	page.dependent=true
	entry({"admin","services","smartinfo","smartdetail"},call("smart_detail")).leaf = true

	entry({"admin","services","smartinfo","status"}, call("smart_status")).leaf = true
	entry({"admin","services","smartinfo","run"},call("run_smart")).leaf=true
	entry({"admin","services","smartinfo","smartattr"},call("smart_attr")).leaf=true

end


function smart_status()
  local cmd = io.popen("/usr/lib/smartinfo/smart_status.sh")
  if cmd then
    local dev = { }
    while true do
      local ln = cmd:read("*l")
      if not ln then
        break
      elseif ln:match("^/%l+/%l+:%a+") then
        local name,status = ln:match("^/%l+/(%l+):(%a+)")
        local model,size
        
        if (status=="OK" or status=="Failed" or status=="Unsupported") then
          model="%s %s" % {nixio.fs.readfile("/sys/class/block/%s/device/vendor" % name), nixio.fs.readfile("/sys/class/block/%s/device/model" % name)}
          local s = tonumber((nixio.fs.readfile("/sys/class/block/%s/size" % name)))
          size = "%s MB" % {s and math.floor(s / 2048)}
        else
          model="Unavailabled"
          size="Unavailabled"
        end
        
        if name and status then
            dev[#dev+1]= {
              name = name,
              model = model,
              size  = size,
              status  = status
            }
        end
      else
      
      end
    end
  
  cmd:close()
  luci.http.prepare_content("application/json")
  luci.http.write_json(dev)
  end
end

function run_smart(dev)
  local cmd = io.popen("smartctl --attributes -d sat /dev/%s" % dev)
  if cmd then
    local report = {}
    local ln = cmd:read("*all")
    report = {
                out = ln
              }
    cmd:close()
    luci.http.prepare_content("application/json")
    luci.http.write_json(report)
  end
end

function smart_detail(dev)
  luci.template.render("smartinfo/smart_detail", {dev=dev})
end

function smart_attr(dev)
  local cmd = io.popen("smartctl --attributes -d sat /dev/%s" % dev)
  if cmd then
    local attr = { }
    while true do
      local ln = cmd:read("*l")
      if not ln then
        break
      elseif ln:match("^.*%d+%s+.+%s+.+%s+.+%s+.+%s+.+%s+.+%s+.+%s+.+%s+.+") then
        local id,attrbute,flag,value,worst,thresh,type,updated,raw = ln:match("^%s*(%d+)%s+([%a%p]+)%s+(%w+)%s+(%d+)%s+(%d+)%s+(%d+)%s+([%a%p]+)%s+(%a+)%s+[%w%p]+%s+(.+)")

        id= "%x" % id
        if not id:match("^%w%w") then
          id = "0%s" % id
        end

        attr[#attr+1]= {
            id = id:upper(),
            attrbute = attrbute,
            flag  = flag,
            value = value,
            worst = worst,
            thresh  = thresh,
            type = type,
            updated = updated,
            raw  = raw
          }
      else
      
      end
    end
  
  cmd:close()
  luci.http.prepare_content("application/json")
  luci.http.write_json(attr)
  end

end
