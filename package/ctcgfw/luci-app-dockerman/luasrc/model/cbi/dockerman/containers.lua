--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-dockerman>
]]--

require "luci.util"
local http = require "luci.http"
local uci = luci.model.uci.cursor()
local docker = require "luci.model.docker"
local dk = docker.new()

local images, networks, containers
local res = dk.images:list()
if res.code <300 then images = res.body else return end
res = dk.networks:list()
if res.code <300 then networks = res.body else return end
res = dk.containers:list({query = {all=true}})
if res.code <300 then containers = res.body else return end

local urlencode = luci.http.protocol and luci.http.protocol.urlencode or luci.util.urlencode

function get_containers()
  local data = {}
  if type(containers) ~= "table" then return nil end
  for i, v in ipairs(containers) do
    local index = v.Created .. v.Id
    data[index]={}
    data[index]["_selected"] = 0
    data[index]["_id"] = v.Id:sub(1,12)
    data[index]["name"] = v.Names[1]:sub(2)
    data[index]["_name"] = '<a href='..luci.dispatcher.build_url("admin/docker/container/"..v.Id)..'  class="dockerman_link" title="'..translate("Container detail")..'">'.. v.Names[1]:sub(2).."</a>"
    data[index]["_status"] = v.Status
    if v.Status:find("^Up") then
      data[index]["_status"] = '<font color="green">'.. data[index]["_status"] .. "</font>"
    else
      data[index]["_status"] = '<font color="red">'.. data[index]["_status"] .. "</font>"
    end
    if (type(v.NetworkSettings) == "table" and type(v.NetworkSettings.Networks) == "table") then
      for networkname, netconfig in pairs(v.NetworkSettings.Networks) do
        data[index]["_network"] = (data[index]["_network"] ~= nil and (data[index]["_network"] .." | ") or "").. networkname .. (netconfig.IPAddress ~= "" and (": " .. netconfig.IPAddress) or "")
      end
    end
    -- networkmode = v.HostConfig.NetworkMode ~= "default" and v.HostConfig.NetworkMode or "bridge"
    -- data[index]["_network"] = v.NetworkSettings.Networks[networkmode].IPAddress or nil
    -- local _, _, image = v.Image:find("^sha256:(.+)")
    -- if image ~= nil then
    --   image=image:sub(1,12)
    -- end
    if v.Ports and next(v.Ports) ~= nil then
      data[index]["_ports"] = nil
      for _,v2 in ipairs(v.Ports) do
        data[index]["_ports"] = (data[index]["_ports"] and (data[index]["_ports"] .. ", ") or "")
        .. ((v2.PublicPort and v2.Type and v2.Type == "tcp") and ('<a href="javascript:void(0);" onclick="window.open((window.location.origin.match(/^(.+):\\d+$/) && window.location.origin.match(/^(.+):\\d+$/)[1] || window.location.origin) + \':\' + '.. v2.PublicPort ..', \'_blank\');">') or "")
        .. (v2.PublicPort and (v2.PublicPort .. ":") or "")  .. (v2.PrivatePort and (v2.PrivatePort .."/") or "") .. (v2.Type and v2.Type or "")
        .. ((v2.PublicPort and v2.Type and v2.Type == "tcp")and "</a>" or "")
      end
    end
    for ii,iv in ipairs(images) do
      if iv.Id == v.ImageID then
        data[index]["_image"] = iv.RepoTags and iv.RepoTags[1] or (iv.RepoDigests[1]:gsub("(.-)@.+", "%1") .. ":<none>")
      end
    end
    if type(v.Mounts) == "table" and next(v.Mounts) then
      for _, v2 in pairs(v.Mounts) do
        if v2.Type ~= "volume" then
          data[index]["_mounts"] = (data[index]["_mounts"] and (data[index]["_mounts"] .. "<br>") or "") .. v2.Source .. "￫" .. v2.Destination
        end
      end
    end
    data[index]["_image_id"] = v.ImageID:sub(8,20)
    data[index]["_command"] = v.Command
  end
  return data
end

local c_lists = get_containers()
-- list Containers
-- m = Map("docker", translate("Docker"))
m = SimpleForm("docker", translate("Docker"))
m.submit=false
m.reset=false

docker_status = m:section(SimpleSection)
docker_status.template = "dockerman/apply_widget"
docker_status.err=docker:read_status()
docker_status.err=docker_status.err and docker_status.err:gsub("\n","<br>"):gsub(" ","&nbsp;")
if docker_status.err then docker:clear_status() end

c_table = m:section(Table, c_lists, translate("Containers"))
c_table.nodescr=true
-- v.template = "cbi/tblsection"
-- v.sortable = true
container_selecter = c_table:option(Flag, "_selected","")
container_selecter.disabled = 0
container_selecter.enabled = 1
container_selecter.default = 0

container_id = c_table:option(DummyValue, "_id", translate("ID"))
container_id.width="10%"
container_name = c_table:option(DummyValue, "_name", translate("Container Name"))
container_name.rawhtml = true
container_status = c_table:option(DummyValue, "_status", translate("Status"))
container_status.width="15%"
container_status.rawhtml=true
container_ip = c_table:option(DummyValue, "_network", translate("Network"))
container_ip.width="15%"
container_ports = c_table:option(DummyValue, "_ports", translate("Ports"))
container_ports.width="10%"
container_ports.rawhtml = true
container_ports = c_table:option(DummyValue, "_mounts", translate("Mounts"))
container_ports.width="15%"
container_ports.rawhtml = true
container_image = c_table:option(DummyValue, "_image", translate("Image"))
container_image.width="8%"
container_command = c_table:option(DummyValue, "_command", translate("Command"))
container_command.width="20%"

container_selecter.write=function(self, section, value)
  c_lists[section]._selected = value
end

local start_stop_remove = function(m,cmd)
  local c_selected = {}
  -- 遍历table中sectionid
  local c_table_sids = c_table:cfgsections()
  for _, c_table_sid in ipairs(c_table_sids) do
    -- 得到选中项的名字
    if c_lists[c_table_sid]._selected == 1 then
      c_selected[#c_selected+1] = c_lists[c_table_sid].name --container_name:cfgvalue(c_table_sid)
    end
  end
  if #c_selected >0 then
    docker:clear_status()
    local success = true
    for _,cont in ipairs(c_selected) do
      docker:append_status("Containers: " .. cmd .. " " .. cont .. "...")
      local res = dk.containers[cmd](dk, {id = cont})
      if res and res.code >= 300 then
        success = false
        docker:append_status("code:" .. res.code.." ".. (res.body.message and res.body.message or res.message).. "\n")
      else
        docker:append_status("done\n")
      end
    end
    if success then docker:clear_status() end
    luci.http.redirect(luci.dispatcher.build_url("admin/docker/containers"))
  end
end

action_section = m:section(Table,{{}})
action_section.notitle=true
action_section.rowcolors=false
action_section.template="cbi/nullsection"

btnnew=action_section:option(Button, "_new")
btnnew.inputtitle= translate("New")
btnnew.template = "dockerman/cbi/inlinebutton"
btnnew.inputstyle = "add"
btnnew.forcewrite = true
btnstart=action_section:option(Button, "_start")
btnstart.template = "dockerman/cbi/inlinebutton"
btnstart.inputtitle=translate("Start")
btnstart.inputstyle = "apply"
btnstart.forcewrite = true
btnrestart=action_section:option(Button, "_restart")
btnrestart.template = "dockerman/cbi/inlinebutton"
btnrestart.inputtitle=translate("Restart")
btnrestart.inputstyle = "reload"
btnrestart.forcewrite = true
btnstop=action_section:option(Button, "_stop")
btnstop.template = "dockerman/cbi/inlinebutton"
btnstop.inputtitle=translate("Stop")
btnstop.inputstyle = "reset"
btnstop.forcewrite = true
btnkill=action_section:option(Button, "_kill")
btnkill.template = "dockerman/cbi/inlinebutton"
btnkill.inputtitle=translate("Kill")
btnkill.inputstyle = "reset"
btnkill.forcewrite = true
btnremove=action_section:option(Button, "_remove")
btnremove.template = "dockerman/cbi/inlinebutton"
btnremove.inputtitle=translate("Remove")
btnremove.inputstyle = "remove"
btnremove.forcewrite = true
btnnew.write = function(self, section)
  luci.http.redirect(luci.dispatcher.build_url("admin/docker/newcontainer"))
end
btnstart.write = function(self, section)
  start_stop_remove(m,"start")
end
btnrestart.write = function(self, section)
  start_stop_remove(m,"restart")
end
btnremove.write = function(self, section)
  start_stop_remove(m,"remove")
end
btnstop.write = function(self, section)
  start_stop_remove(m,"stop")
end
btnkill.write = function(self, section)
  start_stop_remove(m,"kill")
end

return m
