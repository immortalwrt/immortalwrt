--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <lisaac.cn@gmail.com>
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
	http://www.apache.org/licenses/LICENSE-2.0
$Id$
]]--
require "luci.util"
local docker = require "luci.docker"
local uci = (require "luci.model.uci").cursor()

local _docker = {}

_docker.new = function(option)
  local option = option or {}
  options = {
    socket_path = option.socket_path or uci.get("docker","local", "socket_path"),
    debug = option.debug or uci.get("docker","local", "debug") == 'true' and true or false,
    debug_path = option.debug_path or uci.get("docker","local", "debug_path")
  }
  local _new = docker.new(options)
  _new.options.status_path = uci.get("docker","local", "status_path")
  return _new
end
_docker.options={}
_docker.options.status_path = uci.get("docker","local", "status_path")

_docker.append_status=function(self,val)
  local file_docker_action_status=io.open(self.options.status_path,"a+")
  file_docker_action_status:write(val)
  file_docker_action_status:close()
end

_docker.clear_status=function(self)
  nixio.fs.remove(self.options.status_path)
end

return _docker