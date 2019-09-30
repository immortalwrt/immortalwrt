-- Copyright 2014-2018 Sandor Balazsi <sandor.balazsi@gmail.com>
-- Licensed to the public under the GNU General Public License.

local http = require "socket.http"
local https = require "ssl.https"
local ssl = require "ssl"
local ltn12 = require "ltn12"
local fs = require "nixio.fs"
local dispatcher = require "luci.dispatcher"
require "luci.model.cbi.rtorrent.string"

local string, os, io, math, assert = string, os, io, math, assert
local ipairs, table, unpack, tonumber = ipairs, table, unpack, tonumber

local COOKIES_FILE = "/etc/cookies.txt"

module "luci.model.cbi.rtorrent.common"

function get_torrent_pages(hash)
	return {
		{ name = "Info", link = dispatcher.build_url("admin/rtorrent/info/") .. hash },
		{ name = "File List", link = dispatcher.build_url("admin/rtorrent/files/") .. hash },
		{ name = "Tracker List", link = dispatcher.build_url("admin/rtorrent/trackers/") .. hash },
		{ name = "Peer List", link = dispatcher.build_url("admin/rtorrent/peers/") .. hash }
	}
end

function get_admin_pages()
	return {
		{ name = "rTorrent", link = dispatcher.build_url("admin/rtorrent/admin/rtorrent") },
		{ name = "RSS", link = dispatcher.build_url("admin/rtorrent/admin/rss") }
	}
end

function human_size(bytes)
	local symbol = {[0]="B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB"}
	local exp = bytes > 0 and math.floor(math.log(bytes) / math.log(1024)) or 0
	local value = bytes / math.pow(1024, exp)
	local acc = bytes > 0 and 2 - math.floor(math.log10(value)) or 2
	if acc < 0 then acc = 0 end
	return string.format("%." .. acc .. "f " .. symbol[exp], value)
end

function human_time(sec)
	local t = os.date("*t", sec)
	if     t["day"]  > 25 then return "&#8734;"
	elseif t["day"]  > 1 then return string.format("%dd<br />%dh %dm", t["day"] - 1, t["hour"], t["min"])
	elseif t["hour"] > 1 then return string.format("%dh<br />%dm %ds", t["hour"] - 1, t["min"], t["sec"])
	elseif t["min"]  > 0 then return string.format("%dm %ds", t["min"], t["sec"])
	else   return string.format("%ds", t["sec"]) end
end

function div(body, ...)
	local class = {}
	for _, c in ipairs({...}) do
		if c then table.insert(class, c) end
	end
	if #class > 0 then
		return "<div class=\"%s\">%s</div>" % {table.concat(class, " "), body}
	else
		return body
	end
end

function wget(url)
	local file = assert(io.popen("/usr/bin/wget "
		.. "--quiet "
		.. "--user-agent=unknown "
		.. "--referer=http://www.google.com "
		.. "--load-cookies=" .. COOKIES_FILE .. " "
		.. "--output-document=- "
		.. url, "r"))
	local response = file:read("*all")
	file:close()
	return response
end

function get(url)
	local response = {}
	local proto = url:starts("https://") and https or http
	proto.TIMEOUT = 5
	local body, code, headers, status = proto.request {
		method = "GET",
		headers = {
			["Referer"] = "http://www.google.com",
			["User-Agent"] = "unknown",
			["Cookie"] = get_cookies(url)
		},
		url = url,
		redirect = (proto.PORT == 80) and true or nil,
		sink = ltn12.sink.table(response)
	}
	if code == 301 then return get(headers["location"]) end
	if code == 200 then return true, table.concat(response)
	else
		local body = wget(url)
		if body:len() > 0 then return true, body
		else return false, status end
	end
end

function get_cookies(url)
	local cookies = {}
	for _, line in ipairs(fs.readfile(COOKIES_FILE):split("\n\r")) do
		if not line:match("^\s*#.*") then
			local domain, tailmatch, path, secure, expiration, name, value = unpack(line:split())
			local url_match = (secure == "TRUE") and "^https://" or "^https?://"
			url_match = url_match .. (tailmatch == "TRUE" and ".*" or "") .. domain .. path
			if url:match(url_match) and tonumber(expiration) >= os.time() then
				table.insert(cookies, name .. "=" .. value)
			end
		end
	end
	return table.concat(cookies, "; ")
end

