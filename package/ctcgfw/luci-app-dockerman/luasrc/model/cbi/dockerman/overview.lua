--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-dockerman>
]]--

require "luci.util"
local docker = require "luci.model.docker"
local uci = require "luci.model.uci"

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

local map_dockerman = Map("dockerman", translate("Docker"), translate("DockerMan is a Simple Docker manager client for LuCI, If you have any issue please visit:") .. " ".. [[<a href="https://github.com/lisaac/luci-app-dockerman" target="_blank">]] ..translate("Github") .. [[</a>]])
local docker_info_table = {}
-- docker_info_table['0OperatingSystem'] = {_key=translate("Operating System"),_value='-'}
-- docker_info_table['1Architecture'] = {_key=translate("Architecture"),_value='-'}
-- docker_info_table['2KernelVersion'] = {_key=translate("Kernel Version"),_value='-'}
docker_info_table['3ServerVersion'] = {_key=translate("Docker Version"),_value='-'}
docker_info_table['4ApiVersion'] = {_key=translate("Api Version"),_value='-'}
docker_info_table['5NCPU'] = {_key=translate("CPUs"),_value='-'}
docker_info_table['6MemTotal'] = {_key=translate("Total Memory"),_value='-'}
docker_info_table['7DockerRootDir'] = {_key=translate("Docker Root Dir"),_value='-'}
docker_info_table['8IndexServerAddress'] = {_key=translate("Index Server Address"),_value='-'}
docker_info_table['9RegistryMirrors'] = {_key=translate("Registry Mirrors"),_value='-'}

local s = map_dockerman:section(Table, docker_info_table)
s:option(DummyValue, "_key", translate("Info"))
s:option(DummyValue, "_value")
s = map_dockerman:section(SimpleSection)
s.containers_running = '-'
s.images_used = '-'
s.containers_total = '-'
s.images_total = '-'
s.networks_total = '-'
s.volumes_total = '-'
local containers_list
-- local socket = luci.model.uci.cursor():get("dockerman", "local", "socket_path")
if (require "luci.model.docker").new():_ping().code == 200 then
  local dk = docker.new()
  containers_list = dk.containers:list({query = {all=true}}).body
  local images_list = dk.images:list().body
  local vol = dk.volumes:list()
  local volumes_list = vol and vol.body and vol.body.Volumes or {}
  local networks_list = dk.networks:list().body or {}
  local docker_info = dk:info()
  -- docker_info_table['0OperatingSystem']._value = docker_info.body.OperatingSystem
  -- docker_info_table['1Architecture']._value = docker_info.body.Architecture
  -- docker_info_table['2KernelVersion']._value = docker_info.body.KernelVersion
  docker_info_table['3ServerVersion']._value = docker_info.body.ServerVersion
  docker_info_table['4ApiVersion']._value = docker_info.headers["Api-Version"]
  docker_info_table['5NCPU']._value = tostring(docker_info.body.NCPU)
  docker_info_table['6MemTotal']._value = byte_format(docker_info.body.MemTotal)
  if docker_info.body.DockerRootDir then
    local statvfs = nixio.fs.statvfs(docker_info.body.DockerRootDir)
    local size = statvfs and (statvfs.bavail * statvfs.bsize) or 0
    docker_info_table['7DockerRootDir']._value = docker_info.body.DockerRootDir .. " (" .. tostring(byte_format(size)) .. " " .. translate("Available") .. ")"
  end
  docker_info_table['8IndexServerAddress']._value = docker_info.body.IndexServerAddress
  for i, v in ipairs(docker_info.body.RegistryConfig.Mirrors) do
    docker_info_table['9RegistryMirrors']._value = docker_info_table['9RegistryMirrors']._value == "-" and v or (docker_info_table['9RegistryMirrors']._value .. ", " .. v)
  end

  s.images_used = 0
  for i, v in ipairs(images_list) do
    for ci,cv in ipairs(containers_list) do
      if v.Id == cv.ImageID then
        s.images_used = s.images_used + 1
        break
      end
    end
  end
  s.containers_running = tostring(docker_info.body.ContainersRunning)
  s.images_used = tostring(s.images_used)
  s.containers_total = tostring(docker_info.body.Containers)
  s.images_total = tostring(#images_list)
  s.networks_total = tostring(#networks_list)
  s.volumes_total = tostring(#volumes_list)
end
s.template = "dockerman/overview"

local section_dockerman = map_dockerman:section(NamedSection, "local", "section", translate("Setting"))
section_dockerman:tab("daemon", translate("Docker Daemon"))
section_dockerman:tab("ac", translate("Access Control"))
section_dockerman:tab("dockerman",  translate("DockerMan"))

local socket_path = section_dockerman:taboption("dockerman", Value, "socket_path", translate("Docker Socket Path"))
socket_path.default = "/var/run/docker.sock"
socket_path.placeholder = "/var/run/docker.sock"
socket_path.rmempty = false

local remote_endpoint = section_dockerman:taboption("dockerman", Flag, "remote_endpoint", translate("Remote Endpoint"), translate("Dockerman connect to remote endpoint"))
remote_endpoint.rmempty = false
remote_endpoint.enabled = "true"
remote_endpoint.disabled = "false"

local remote_host = section_dockerman:taboption("dockerman", Value, "remote_host", translate("Remote Host"))
remote_host.placeholder = "10.1.1.2"
-- remote_host:depends("remote_endpoint", "true")

local remote_port = section_dockerman:taboption("dockerman", Value, "remote_port", translate("Remote Port"))
remote_port.placeholder = "2375"
remote_port.default = "2375"
-- remote_port:depends("remote_endpoint", "true")

-- local status_path = section_dockerman:taboption("dockerman", Value, "status_path", translate("Action Status Tempfile Path"), translate("Where you want to save the docker status file"))
-- local debug = section_dockerman:taboption("dockerman", Flag, "debug", translate("Enable Debug"), translate("For debug, It shows all docker API actions of luci-app-dockerman in Debug Tempfile Path"))
-- debug.enabled="true"
-- debug.disabled="false"
-- local debug_path = section_dockerman:taboption("dockerman", Value, "debug_path", translate("Debug Tempfile Path"), translate("Where you want to save the debug tempfile"))

if nixio.fs.access("/usr/bin/dockerd") then
  local allowed_interface = section_dockerman:taboption("ac", DynamicList, "ac_allowed_interface", translate("Allowed access interfaces"), translate("Which interface(s) can access containers under the bridge network, fill-in Interface Name"))
  local interfaces = luci.sys and luci.sys.net and luci.sys.net.devices() or {}
  for i, v in ipairs(interfaces) do
    allowed_interface:value(v, v)
  end
  local allowed_container = section_dockerman:taboption("ac", DynamicList, "ac_allowed_container", translate("Containers allowed to be accessed"), translate("Which container(s) under bridge network can be accessed, even from interfaces that are not allowed, fill-in Container Id or Name"))
  -- allowed_container.placeholder = "container name_or_id"
  if containers_list then
    for i, v in ipairs(containers_list) do
      if  v.State == "running" and v.NetworkSettings and v.NetworkSettings.Networks and v.NetworkSettings.Networks.bridge and v.NetworkSettings.Networks.bridge.IPAddress then
        allowed_container:value(v.Id:sub(1,12), v.Names[1]:sub(2) .. " | " .. v.NetworkSettings.Networks.bridge.IPAddress)
      end
    end
  end

  local dockerd_enable = section_dockerman:taboption("daemon", Flag, "daemon_ea", translate("Enable"))
  dockerd_enable.enabled = "true"
  dockerd_enable.rmempty = true
  local data_root = section_dockerman:taboption("daemon", Value, "daemon_data_root", translate("Docker Root Dir"))
  data_root.placeholder = "/opt/docker/"
  local registry_mirrors = section_dockerman:taboption("daemon", DynamicList, "daemon_registry_mirrors", translate("Registry Mirrors"))
  registry_mirrors:value("https://hub-mirror.c.163.com", "https://hub-mirror.c.163.com")

  local log_level = section_dockerman:taboption("daemon", ListValue, "daemon_log_level", translate("Log Level"), translate('Set the logging level'))
  log_level:value("debug", "debug")
  log_level:value("info", "info")
  log_level:value("warn", "warn")
  log_level:value("error", "error")
  log_level:value("fatal", "fatal")
  local hosts = section_dockerman:taboption("daemon", DynamicList, "daemon_hosts", translate("Server Host"), translate('Daemon unix socket (unix:///var/run/docker.sock) or TCP Remote Hosts (tcp://0.0.0.0:2375), default: unix:///var/run/docker.sock'))
  hosts:value("unix:///var/run/docker.sock", "unix:///var/run/docker.sock")
  hosts:value("tcp://0.0.0.0:2375", "tcp://0.0.0.0:2375")
  hosts.rmempty = true
end
return map_dockerman
