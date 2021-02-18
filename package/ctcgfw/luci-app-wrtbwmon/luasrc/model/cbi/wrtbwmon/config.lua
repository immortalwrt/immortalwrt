local lastvalue
local cursor = luci.model.uci.cursor()
local m, s, o
m = Map("wrtbwmon", translate("Usage - Configuration"))

s = m:section(NamedSection, "general", "wrtbwmon", translate("General settings"))

o = s:option(Flag, "enabled", translate("Enabled"))
o.rmempty= true

o = s:option(Value, "path", translate("Database Path"),
	translate("This box is used to select the Database path, "
	.. "which is /tmp/usage.db by default."))
o:value("/tmp/usage.db")
o:value("/etc/usage.db")
o.rmempty= false

function m.on_parse(self)
	lastvalue = cursor:get("wrtbwmon", "general", "path")
end

function o.write(self,section,value)
	local fpath = nixio.fs.dirname(value) .. "/"

	if not nixio.fs.access(fpath) then
		if not nixio.fs.mkdirr(fpath) then
			return Value.write(self, section, lastvalue)
		end
	end

	io.popen("/etc/init.d/wrtbwmon stop")
	io.popen("mv -f " .. fileRename(lastvalue, "*") .. " ".. fpath)
	Value.write(self,section,value)
	io.popen("/etc/init.d/wrtbwmon start")

	return true
end

function fileRename(fileName, tag)
	local idx = fileName:match(".+()%.%w+$")
	if(idx) then
		return fileName:sub(1, idx-1) .. tag .. fileName:sub(idx, -1)
	else
		return fileName .. tag
	end
end

return m
