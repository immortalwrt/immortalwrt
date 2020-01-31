-- Copyright 2014-2018 Sandor Balazsi <sandor.balazsi@gmail.com>
-- Licensed to the public under the GNU General Public License.

local nixio = require "nixio"
local http = require "luci.http"
local ltn12 = require "luci.ltn12"
local sys = require "luci.controller.admin.system"
local common = require "luci.model.cbi.rtorrent.common"
require "luci.model.cbi.rtorrent.string"

local ipairs, string = ipairs, string

local PROTECTED_PATH = {"/bin", "/dev", "/etc", "/lib", "/overlay", "/root", "/sbin", "/tmp", "/usr", "/var", "/www"}

module("luci.model.cbi.rtorrent.download", package.seeall)

function security_check(file)
	for _, path_prefix in ipairs(PROTECTED_PATH) do
		if file:starts(path_prefix) then
			http.write("<h1>Access Denied</h1>")
			return false
		end
	end
	return true
end

function download_file(file)
	file = nixio.fs.realpath(file)
	if security_check(file) then
		local f = nixio.open(file, "r")
		http.header('Content-Disposition', 'attachment; filename="%s"' % nixio.fs.basename(file))
		http.header('Content-Length', nixio.fs.stat(file, "size"))
		http.prepare_content("application/octet-stream")
		repeat
			local buf = f:read(2^13)	-- 8k
			http.write(buf)
		until (buf == "")
		f:close()
	end
end

function download_all(path)
	path = nixio.fs.realpath(path)
	if security_check(path) then
		if string.find(string.lower(http.getenv("HTTP_USER_AGENT")), "linux") then
			download_all_as_tar(path)
		else
			download_all_as_zip(path)
		end
	end
end

function download_all_as_zip(path)
	local reader = sys.ltn12_popen("zip -0 -j -r - \"%s\"" % path)
	http.header('Content-Disposition', 'attachment; filename="%s.zip"' % nixio.fs.basename(path))
	http.prepare_content("application/zip")
	ltn12.pump.all(reader, http.write)
end

function download_all_as_tar(path)
	local reader = sys.ltn12_popen("tar -cf - -C \"%s\" ." % path)
	http.header('Content-Disposition', 'attachment; filename="%s.tar"' % nixio.fs.basename(path))
	http.prepare_content("application/x-tar")
	ltn12.pump.all(reader, http.write)
end

