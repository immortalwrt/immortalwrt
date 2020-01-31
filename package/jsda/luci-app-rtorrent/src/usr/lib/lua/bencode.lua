-- Copyright (c) 2009, 2010, 2011, 2012 by Moritz Wilhelmy
-- Copyright (c) 2009 by Kristofer Karlsson
-- Public domain lua-module for handling bittorrent-bencoded data.
-- This module includes both a recursive decoder and a recursive encoder.

local sort, concat, insert = table.sort, table.concat, table.insert
local pairs, ipairs, type, tonumber = pairs, ipairs, type, tonumber
local sub, find = string.sub, string.find

local M = {}

-- helpers

local function islist(t) 
	local n = #t 
	for k, v in pairs(t) do 
		if type(k) ~= "number" 
		or k % 1 ~= 0 		-- integer?
		or k < 1
		or k > n 
		then 
			return false 
		end 
	end 
	for i = 1, n do
		if t[i] == nil then 
			return false 
		end 
	end 
	return true
end 

-- encoder functions

local encode_rec -- encode_list/dict and encode_rec are mutually recursive...

local function encode_list(t, x)

	insert(t, "l")

	for _,v in ipairs(x) do 
		local err,ev = encode_rec(t, v);    if err then return err,ev end
	end

	insert(t, "e") 
end

local function encode_dict(t, x)
	insert(t, "d")
	-- bittorrent requires the keys to be sorted.
	local sortedkeys = {}
	for k, v in pairs(x) do
		if type(k) ~= "string" then
			return "bencoding requires dictionary keys to be strings", k
		end
		insert(sortedkeys, k)
	end
	sort(sortedkeys)

	for k, v in ipairs(sortedkeys) do
		local err,ev = encode_rec(t, v);    if err then return err,ev end
		      err,ev = encode_rec(t, x[v]); if err then return err,ev end
	end
	insert(t, "e")
end

local function encode_int(t, x)

	if x % 1 ~= 0 then return "number is not an integer", x end
	insert(t, "i" )
	insert(t,  x  )
	insert(t, "e" )
end

local function encode_str(t, x)

	insert(t, #x  ) 
	insert(t, ":" )
	insert(t,  x  )
end

encode_rec = function(t, x)

	local  typx = type(x)
	if     typx == "string" then  return encode_str  (t, x)
	elseif typx == "number" then  return encode_int  (t, x)
	elseif typx == "table"  then

		if islist(x)    then  return encode_list (t, x)
		else                  return encode_dict (t, x)
		end
	else
		return "type cannot be converted to an acceptable type for bencoding", typx
	end
end

-- call recursive bencoder function with empty table, stringify that table.
-- this is the only encode* function visible to module users.
M.encode = function (x)

	local t = {}
	local err, val = encode_rec(t,x)
	if not err then
		return concat(t)
	else
		return nil, err, val
	end
end

-- decoder functions

local function decode_integer(s, index) 
	local a, b, int = find(s, "^(%-?%d+)e", index) 
	if not int then return nil, "not a number", nil end
	int = tonumber(int) 
	if not int then return nil, "not a number", int end
	return int, b + 1 
end 

local function decode_list(s, index) 
	local t = {} 
	while sub(s, index, index) ~= "e" do 
		local obj, ev
		obj, index, ev = M.decode(s, index) 
		if not obj then return obj, index, ev end
		insert(t, obj)
	end 
	index = index + 1 
	return t, index 
end 
	 
local function decode_dictionary(s, index) 
	local t = {} 
	while sub(s, index, index) ~= "e" do 
		local obj1, obj2, ev

		obj1, index, ev = M.decode(s, index) 
		if not obj1 then return obj1, index, ev end

		obj2, index, ev = M.decode(s, index) 
		if not obj2 then return obj2, index, ev end

		t[obj1] = obj2 
	end 
	index = index + 1 
	return t, index 
end 
	 
local function decode_string(s, index) 
	local a, b, len = find(s, "^([0-9]+):", index) 
	if not len then return nil, "not a length", len end
	index = b + 1 
	 
	local v = sub(s, index, index + len - 1) 
	if #v < tonumber(len) then return nil, "truncated string at end of input", v end
	index = index + len 
	return v, index 
end 
	 
	 
M.decode = function (s, index) 
	if not s then return nil, "no data", nil end
	index = index or 1 
	local t = sub(s, index, index) 
	if not t then return nil, "truncation error", nil end

	if t == "i" then 
		return decode_integer(s, index + 1) 
	elseif t == "l" then 
		return decode_list(s, index + 1) 
	elseif t == "d" then 
		return decode_dictionary(s, index + 1) 
	elseif t >= '0' and t <= '9' then 
		return decode_string(s, index) 
	else 
		return nil, "invalid type", t
	end 
end

return M
