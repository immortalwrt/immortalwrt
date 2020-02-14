-- Copyright 2003-2010 Kepler Project
-- XML-RPC implementation for Lua.

local lxp = require "lxp"
local lom = require "lxp.lom"

local assert, error, ipairs, pairs, select, type, tonumber, unpack = assert, error, ipairs, pairs, select, type, tonumber, unpack
local format, gsub, strfind, strsub = string.format, string.gsub, string.find, string.sub
local concat, tinsert = table.concat, table.insert
local ceil = math.ceil
local parse = lom.parse

module (...)

_COPYRIGHT = "Copyright (C) 2003-2014 Kepler Project"
_DESCRIPTION = "LuaXMLRPC is a library to make remote procedure calls using XML-RPC"
_PKGNAME = "LuaXMLRPC"
_VERSION_MAJOR = 1
_VERSION_MINOR = 2
_VERSION_MICRO = 2
_VERSION = _VERSION_MAJOR .. "." .. _VERSION_MINOR .. "." .. _VERSION_MICRO

---------------------------------------------------------------------
-- XML-RPC Parser
---------------------------------------------------------------------

---------------------------------------------------------------------
local function trim (s)
	return (type(s) == "string" and gsub (s, "^%s*(.-)%s*$", "%1"))
end

---------------------------------------------------------------------
local function is_space (s)
	return type(s) == "string" and trim(s) == ""
end

---------------------------------------------------------------------
-- Get next non-space element from tab starting from index i.
-- @param tab Table.
-- @param i Numeric index.
-- @return Object and its position on table; nil and an invalid index
--	when there is no more elements.
---------------------------------------------------------------------
function next_nonspace (tab, i)
	if not i then i = 1 end
	while is_space (tab[i]) do i = i+1 end
	return tab[i], i
end

---------------------------------------------------------------------
-- Get next element of tab with the given tag starting from index i.
-- @param tab Table.
-- @param tag String with the name of the tag.
-- @param i Numeric index.
-- @return Object and its position on table; nil and an invalid index
--	when there is no more elements.
---------------------------------------------------------------------
local function next_tag (tab, tag, i)
	if not i then i = 1 end
	while tab[i] do
		if type (tab[i]) == "table" and tab[i].tag == tag then
			return tab[i], i
		end
		i = i + 1
	end
	return nil, i
end

---------------------------------------------------------------------
local function x2number (tab)
	if tab.tag == "int" or tab.tag == "i4" or tab.tag == "i8" or tab.tag == "double" then
		return tonumber (next_nonspace (tab, 1), 10)
	end
end

---------------------------------------------------------------------
local function x2boolean (tab)
	if tab.tag == "boolean" then
		local v = next_nonspace (tab, 1)
		return v == true or v == "true" or tonumber (v) == 1 or false
	end
end

---------------------------------------------------------------------
local function x2string (tab)
	return tab.tag == "string" and (tab[1] or "")
end

---------------------------------------------------------------------
local function x2date (tab)
	return tab.tag == "dateTime.iso8601" and next_nonspace (tab, 1)
end

---------------------------------------------------------------------
local function x2base64 (tab)
	return tab.tag == "base64" and next_nonspace (tab, 1)
end

---------------------------------------------------------------------
local function x2name (tab)
	return tab.tag == "name" and next_nonspace (tab, 1)
end

local x2value

---------------------------------------------------------------------
-- Disassemble a member object in its name and value parts.
-- @param tab Table with a DOM representation.
-- @return String (name) and Object (value).
-- @see x2name, x2value.
---------------------------------------------------------------------
local function x2member (tab)
	return
		x2name (next_tag(tab,"name")),
		x2value (next_tag(tab,"value"))
end

---------------------------------------------------------------------
-- Disassemble a struct object into a Lua table.
-- @param tab Table with DOM representation.
-- @return Table with "name = value" pairs.
---------------------------------------------------------------------
local function x2struct (tab)
	if tab.tag == "struct" then
		local res = {}
		for i = 1, #tab do
			if not is_space (tab[i]) then
				local name, val = x2member (tab[i])
				res[name] = val
			end
		end
		return res
	end
end

---------------------------------------------------------------------
-- Disassemble an array object into a Lua table.
-- @param tab Table with DOM representation.
-- @return Table.
---------------------------------------------------------------------
local function x2array (tab)
	if tab.tag == "array" then
		local d = next_tag (tab, "data")
		local res = {}
		for i = 1, #d do
			if not is_space (d[i]) then
				tinsert (res, x2value (d[i]))
			end
		end
		return res
	end
end

---------------------------------------------------------------------
local xmlrpc_types = {
	int = x2number,
	i4 = x2number,
	i8 = x2number,
	boolean = x2boolean,
	string = x2string,
	double = x2number,
	["dateTime.iso8601"] = x2date,
	base64 = x2base64,
	struct = x2struct,
	array = x2array,
}

local x2param, x2fault

---------------------------------------------------------------------
-- Disassemble a methodResponse into a Lua object.
-- @param tab Table with DOM representation.
-- @return Boolean (indicating wether the response was successful)
--	and (a Lua object representing the return values OR the fault
--	string and the fault code).
---------------------------------------------------------------------
local function x2methodResponse (tab)
	assert (type(tab) == "table", "Not a table")
	assert (tab.tag == "methodResponse",
		"Not a `methodResponse' tag: "..tab.tag)
	local t = next_nonspace (tab, 1)
	if t.tag == "params" then
		return true, unpack (x2param (t))
	elseif t.tag == "fault" then
		local f = x2fault (t)
		return false, f.faultString, f.faultCode
	else
		error ("Couldn't find a <params> nor a <fault> element")
	end
end

---------------------------------------------------------------------
-- Disassemble a value element into a Lua object.
-- @param tab Table with DOM representation.
-- @return Object.
---------------------------------------------------------------------
x2value = function (tab)
	local t = tab.tag
	assert (t == "value", "Not a `value' tag: "..t)
	local n = next_nonspace (tab)
	if type(n) == "string" or type(n) == "number" then
		return n
	elseif type (n) == "table" then
		local t = n.tag
		local get = xmlrpc_types[t]
		if not get then error ("Invalid <"..t.."> element") end
		return get (next_nonspace (tab))
        elseif type(n) == "nil" then
                -- the next best thing is to assume it's an empty string
               return ""

	end
end

---------------------------------------------------------------------
-- Disassemble a fault element into a Lua object.
-- @param tab Table with DOM representation.
-- @return Object.
---------------------------------------------------------------------
x2fault = function (tab)
	assert (tab.tag == "fault", "Not a `fault' tag: "..tab.tag)
	return x2value (next_nonspace (tab))
end

---------------------------------------------------------------------
-- Disassemble a param element into a Lua object.
-- Ignore white spaces between elements.
-- @param tab Table with DOM representation.
-- @return Object.
---------------------------------------------------------------------
x2param = function (tab)
	assert (tab.tag == "params", "Not a `params' tag")
	local res = {}
	local p, i = next_nonspace (tab, 1)
	while p do
		if p.tag == "param" then
			tinsert (res, x2value (next_tag (p, "value")))
		end
		p, i = next_nonspace (tab, i+1)
	end
	return res
end

---------------------------------------------------------------------
-- Disassemble a methodName element into a Lua object.
-- @param tab Table with DOM representation.
-- @return Object.
---------------------------------------------------------------------
local function x2methodName (tab)
	assert (tab.tag == "methodName", "Not a `methodName' tag: "..tab.tag)
	return (next_nonspace (tab, 1))
end

---------------------------------------------------------------------
-- Disassemble a methodCall element into its name and a list of parameters.
-- @param tab Table with DOM representation.
-- @return Object.
---------------------------------------------------------------------
local function x2methodCall (tab)
	assert (tab.tag == "methodCall", "Not a `methodCall' tag: "..tab.tag)
	return
		x2methodName (next_tag (tab,"methodName")),
		x2param (next_tag (tab,"params"))
end

---------------------------------------------------------------------
-- End of XML-RPC Parser
---------------------------------------------------------------------

---------------------------------------------------------------------
-- Convert a Lua Object into an XML-RPC string.
---------------------------------------------------------------------

---------------------------------------------------------------------
local formats = {
	boolean = "<boolean>%d</boolean>",
	number = "<double>%d</double>",
	string = "<string>%s</string>",
	base64 = "<base64>%s</base64>",

	array = "<array><data>\n%s\n</data></array>",
	double = "<double>%s</double>",
	int = "<int>%s</int>",
	struct = "<struct>%s</struct>",

	member = "<member><name>%s</name>%s</member>",
	value = "<value>%s</value>",

	param = "<param>%s</param>",

	params = [[
    <params>
      %s
    </params>]],

	fault = [[
    <fault>
      %s
    </fault>]],

	methodCall = [[
<?xml version="1.0"?>
<methodCall>
  <methodName>%s</methodName>
%s
</methodCall>
]],

	methodResponse = [[
<?xml version="1.0"?>
<methodResponse>
%s
</methodResponse>]],
}
formats.table = formats.struct

local toxml = {}
toxml.double = function (v,t) return format (formats.double, v) end
toxml.int = function (v,t) return format (formats.int, v) end
toxml.string = function (v,t) return format (formats.string, v) end
toxml.base64 = function (v,t) return format (formats.base64, v) end

---------------------------------------------------------------------
-- Build a XML-RPC representation of a boolean.
-- @param v Object.
-- @return String.
---------------------------------------------------------------------
function toxml.boolean (v)
	local n = (v and 1) or 0
	return format (formats.boolean, n)
end

---------------------------------------------------------------------
-- Build a XML-RPC representation of a number.
-- @param v Object.
-- @param t Object representing the XML-RPC type of the value.
-- @return String.
---------------------------------------------------------------------
function toxml.number (v, t)
	local tt = (type(t) == "table") and t["*type"]
	if tt == "int" or tt == "i4" or tt == "i8" then
		return toxml.int (v, t)
	elseif tt == "double" then
		return toxml.double (v, t)
	elseif v == ceil(v) then
		return toxml.int (v, t)
	else
		return toxml.double (v, t)
	end
end

---------------------------------------------------------------------
-- @param typ Object representing a type.
-- @return Function that generate an XML element of the given type.
-- The object could be a string (as usual in Lua) or a table with
-- a field named "type" that should be a string with the XML-RPC
-- type name.
---------------------------------------------------------------------
local function format_func (typ)
	if type (typ) == "table" then
		return toxml[typ.type]
	else
		return toxml[typ]
	end
end

---------------------------------------------------------------------
-- @param val Object representing an array of values.
-- @param typ Object representing the type of the value.
-- @return String representing the equivalent XML-RPC value.
---------------------------------------------------------------------
function toxml.array (val, typ)
	local ret = {}
	local et = typ.elemtype
	local f = format_func (et)
	for i,v in ipairs (val) do
		if et and et ~= "array" then
			tinsert (ret, format (formats.value, f (v, et)))
		else
			local ct,cv = type_val(v)
			local cf = format_func(ct)
			tinsert (ret, format (formats.value, cf(cv, ct)))
		end

	end
	return format (formats.array, concat (ret, '\n'))
end

---------------------------------------------------------------------
---------------------------------------------------------------------
function toxml.struct (val, typ)
	local ret = {}
	if type (typ) == "table" then
		for n,t in pairs (typ.elemtype) do
			local f = format_func (t)
			tinsert (ret, format (formats.member, n, f (val[n], t)))
		end
	else
		for i, v in pairs (val) do
			tinsert (ret, toxml.member (i, v))
		end
	end
	return format (formats.struct, concat (ret))
end

toxml.table = toxml.struct

---------------------------------------------------------------------
---------------------------------------------------------------------
function toxml.member (n, v)
	return format (formats.member, n, toxml.value (v))
end

---------------------------------------------------------------------
-- Get type and value of object.
---------------------------------------------------------------------
function type_val (obj)
	local t = type (obj)
	local v = obj
	if t == "table" then
		t = obj["*type"] or "table"
		v = obj["*value"] or obj
	end
	return t, v
end

---------------------------------------------------------------------
-- Convert a Lua object to a XML-RPC object (plain string).
---------------------------------------------------------------------
function toxml.value (obj)
	local to, val = type_val (obj)
	if type(to) == "table" then
		return format (formats.value, toxml[to.type] (val, to))
	else
		-- primitive (not structured) types.
		--return format (formats[to], val)
		return format (formats.value, toxml[to] (val, to))
	end
end

---------------------------------------------------------------------
-- @param ... List of parameters.
-- @return String representing the `params' XML-RPC element.
---------------------------------------------------------------------
function toxml.params (...)
	local params_list = {}
	for i = 1, select ("#", ...) do
		params_list[i] = format (formats.param, toxml.value (select (i, ...)))
	end
	return format (formats.params, concat (params_list, '\n      '))
end

---------------------------------------------------------------------
-- @param method String with method's name.
-- @param ... List of parameters.
-- @return String representing the `methodCall' XML-RPC element.
---------------------------------------------------------------------
function toxml.methodCall (method, ...)
	local idx = strfind (method, "[^A-Za-z_.:/0-9]")
	if idx then
		error (format ("Invalid character `%s'", strsub (method, idx, idx)))
	end
	return format (formats.methodCall, method, toxml.params (...))
end

---------------------------------------------------------------------
-- @param err String with error message.
-- @return String representing the `fault' XML-RPC element.
---------------------------------------------------------------------
function toxml.fault (err)
	local code
	local message = err
	if type (err) == "table" then
		code = err.code
		message = err.message
	end
	return format (formats.fault, toxml.value {
		faultCode = { ["*type"] = "int", ["*value"] = code or err.faultCode or 1 },
		faultString = message or err.faultString or "fatal error",
	})
end

---------------------------------------------------------------------
-- @param ok Boolean indicating if the response was correct or a
--	fault one.
-- @param params Object containing the response contents.
-- @return String representing the `methodResponse' XML-RPC element.
---------------------------------------------------------------------
function toxml.methodResponse (ok, params)
	local resp
	if ok then
		resp = toxml.params (params)
	else
		resp = toxml.fault (params)
	end
	return format (formats.methodResponse, resp)
end

---------------------------------------------------------------------
-- End of converter from Lua to XML-RPC.
---------------------------------------------------------------------


---------------------------------------------------------------------
-- Create a representation of an array with the given element type.
---------------------------------------------------------------------
function newArray (elemtype)
	return { type = "array", elemtype = elemtype, }
end

---------------------------------------------------------------------
-- Create a representation of a structure with the given members.
---------------------------------------------------------------------
function newStruct (members)
	return { type = "struct", elemtype = members, }
end

---------------------------------------------------------------------
-- Create a representation of a value according to a type.
-- @param val Any Lua value.
-- @param typ A XML-RPC type.
---------------------------------------------------------------------
function newTypedValue (val, typ)
	return { ["*type"] = typ, ["*value"] = val }
end

---------------------------------------------------------------------
-- Create the XML-RPC string used to call a method.
-- @param method String with method name.
-- @param ... Parameters to the call.
-- @return String with the XML string/document.
---------------------------------------------------------------------
function clEncode (method, ...)
	return toxml.methodCall (method, ...)
end

---------------------------------------------------------------------
-- Convert the method response document to a Lua table.
-- @param meth_resp String with XML document.
-- @return Boolean indicating whether the call was successful or not;
--	and a Lua object with the converted response element.
---------------------------------------------------------------------
function clDecode (meth_resp)
	local d = parse (meth_resp)
	if type(d) ~= "table" then
		error ("Not an XML document: "..meth_resp)
	end
	return x2methodResponse (d)
end

---------------------------------------------------------------------
-- Convert the method call (client request) document to a name and
--	a list of parameters.
-- @param request String with XML document.
-- @return String with method's name AND the table of arguments.
---------------------------------------------------------------------
function srvDecode (request)
	local d = parse (request)
	if type(d) ~= "table" then
		error ("Not an XML document: "..request)
	end
	return x2methodCall (d)
end

---------------------------------------------------------------------
-- Convert a table into an XML-RPC methodReponse element.
-- @param obj Lua object.
-- @param is_fault Boolean indicating wether the result should be
--	a `fault' element (default = false).
-- @return String with XML-RPC response.
---------------------------------------------------------------------
function srvEncode (obj, is_fault)
	local ok = not (is_fault or false)
	return toxml.methodResponse (ok, obj)
end

---------------------------------------------------------------------
-- Register the methods.
-- @param tab_or_func Table or mapping function.
-- If a table is given, it can have one level of objects and then the
-- methods;
-- if a function is given, it will be used as the dispatcher.
-- The given function should return a Lua function that implements.
---------------------------------------------------------------------
dispatch = error
function srvMethods (tab_or_func)
	local t = type (tab_or_func)
	if t == "function" then
		dispatch = tab_or_func
	elseif t == "table" then
		dispatch = function (name)
			local ok, _, obj, method = strfind (name, "^([^.]+)%.(.+)$")
			if not ok then
				return tab_or_func[name]
			else
				if tab_or_func[obj] and tab_or_func[obj][method] then
					return function (...)
						return tab_or_func[obj][method] (obj, ...)
					end
				else
					return nil
				end
			end
		end
	else
		error ("Argument is neither a table nor a function")
	end
end
