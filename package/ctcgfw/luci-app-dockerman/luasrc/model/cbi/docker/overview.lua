--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <lisaac.cn@gmail.com>
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
	http://www.apache.org/licenses/LICENSE-2.0
$Id$
]]--
if nixio.fs.access("/etc/config/dockerd") then
	local running = (luci.sys.call("pidof portainer >/dev/null") == 0)
	local button = ""

	if running then
		button = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<br /><br /><input type=\"button\" value=\" " .. translate("Open Portainer Docker Admin") .. " \" onclick=\"window.open('http://'+window.location.hostname+':" .. 9999 .. "')\"/><br />"
	end

	m = Map("dockerd", "Docker CE", translate("Docker is a set of platform-as-a-service (PaaS) products that use OS-level virtualization to deliver software in packages called containers.") .. button)


	m:section(SimpleSection).template  = "docker/docker_status"

	s = m:section(TypedSection, "docker")
	s.anonymous = true

	wan_mode = s:option(Flag, "wan_mode", translate("Enable WAN access Dokcer"), translate("Enable WAN access docker mapped ports"))
	wan_mode.default = 0
	wan_mode.rmempty = false

	o=s:option(DummyValue,"readme",translate(" "))
	o.description=translate("<a href=\"../../../../DockerReadme.pdf\" target=\"_blank\" />"..translate("Download DockerReadme.pdf").."</a>")
end


m2 = Map("docker", translate("Docker"))
s = m2:section(NamedSection, "local", "section", translate("Setting"))

socket_path = s:option(Value, "socket_path", translate("Socket Path"))
status_path = s:option(Value, "status_path", translate("Action Status Tempfile Path"), translate("Where you want to save the docker status file"))
debug = s:option(Flag, "debug", translate("Enable Debug"), translate("For debug, It shows all docker API actions of luci-app-docker in Debug Tempfile Path"))
debug.enabled="true"
debug.disabled="false"
debug_path = s:option(Value, "debug_path", translate("Debug Tempfile Path"), translate("Where you want to save the debug tempfile"))
if m then 
	return m, m2
else
	return m2
end