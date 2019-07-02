-- Copyright 2014-2018 Sandor Balazsi <sandor.balazsi@gmail.com>
-- Licensed to the public under the GNU General Public License.

local bencode = require "bencode"
local nixio = require "nixio"
local rtorrent = require "rtorrent"
local xmlrpc = require "xmlrpc"
local common = require "luci.model.cbi.rtorrent.common"
require "luci.model.cbi.rtorrent.string"

f = SimpleForm("rtorrent", "Add Torrent")
f.submit = "Add"

local torrent

uri = f:field(TextValue, "uri", "Torrent<br />or magnet URI")
uri.rows = 1

function uri.validate(self, value, section)
	if "magnet:" == string.sub(value:trim(), 1, 7) then
		torrent = bencode.encode({ ["magnet-uri"] = value:trim() })
	else
		local ok, res = common.get(value)
		if not ok then return nil, "Not able to download torrent: " .. res end
		local tab, err = bencode.decode(res)
		if not tab then return nil, "Not able to parse torrent file: " .. err end
		torrent = res
	end
	return value
end

file = f:field(FileUpload, "file", "Upload torrent file")

function file.validate(self, value, section)
	torrent = nixio.fs.readfile(value)
	self:remove(section)
	local tab, err = bencode.decode(torrent)
	if not tab then return nil, "Not able to parse torrent file: " .. err end
	return value
end

dir = f:field(Value, "dir", "Download directory")
dir.default = rtorrent.call("directory.default")
dir.datatype = "directory"
dir.rmempty = false

tags = f:field(Value, "tags", "Tags")
local user =  luci.dispatcher.context.authuser
tags.default = "all" .. (user ~= "root" and " " .. user or "")
tags.rmempty = false

start = f:field(Flag, "start", "Start now")
start.default  = "1"
start.rmempty  = false

function f.handle(self, state, data)
	if state == FORM_VALID and torrent and #torrent > 0 then
		local params = {}
		table.insert(params, data.start == "1" and "load.raw_start" or "load.raw")
		table.insert(params, "") -- target
		table.insert(params, xmlrpc.newTypedValue((nixio.bin.b64encode(torrent)), "base64"))
		table.insert(params, "d.directory.set=\"" .. data.dir .. "\"")
		table.insert(params, "d.custom1.set=\"" .. data.tags .. "\"")
		if data.uri then
			table.insert(params, "d.custom3.set=" .. nixio.bin.b64encode(data.uri))
		end
		rtorrent.call(unpack(params))
		luci.http.redirect(luci.dispatcher.build_url("admin/rtorrent/add"))
	end
	return true
end

return f

