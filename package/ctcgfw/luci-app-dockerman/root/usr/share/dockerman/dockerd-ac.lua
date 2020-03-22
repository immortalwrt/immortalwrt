require "luci.util"
docker = require "luci.docker"
uci = (require "luci.model.uci").cursor()
dk = docker.new({socket_path = "/var/run/docker.sock"})

if dk:_ping().code ~= 200 then return end
containers_list = dk.containers:list({query = {all=true}}).body
allowed_container = uci:get("dockerman", "local", "ac_allowed_container")

if not allowed_container or next(allowed_container)==nil then return end
allowed_ip = {}
for i, v in ipairs(containers_list) do
  for ii, vv in ipairs(allowed_container) do
    if v.Id:sub(1,12) == vv and v.NetworkSettings and v.NetworkSettings.Networks and v.NetworkSettings.Networks.bridge and v.NetworkSettings.Networks.bridge.IPAddress then
      print(v.NetworkSettings.Networks.bridge.IPAddress)
      luci.util.exec("iptables -I DOCKER-MAN -d "..v.NetworkSettings.Networks.bridge.IPAddress.." -o docker0 -j RETURN")
      table.remove(allowed_container, ii)
    end
  end
end
