-- Copyright 2014-2018 Sandor Balazsi <sandor.balazsi@gmail.com>
-- Licensed to the public under the GNU General Public License.

function string.starts(str, begin)
	if not str then return false end
	return string.sub(str, 1, string.len(begin)) == begin
end

function string.ends(str, tail)
	if not str then return false end
	return string.sub(str, -string.len(tail)) == tail
end

function string.split(str, sep)
	if sep == nil then sep = "%s" end
	local t = {}
	for s in str:gmatch("([^" .. sep .. "]+)") do
		table.insert(t, s)
	end
	return t
end

function string.ucfirst(str)
	return (str:gsub("^%l", string.upper))
end

function string.trim(str)
	return str:match("^()%s*$") and "" or str:match("^%s*(.*%S)")
end

