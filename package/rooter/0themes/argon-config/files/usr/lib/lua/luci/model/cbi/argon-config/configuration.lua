local nxfs = require 'nixio.fs'
local wa = require 'luci.tools.webadmin'
local opkg = require 'luci.model.ipkg'
local sys = require 'luci.sys'
local http = require 'luci.http'
local nutil = require 'nixio.util'
local name = 'argon'
local uci = require 'luci.model.uci'.cursor()

local fstat = nxfs.statvfs(opkg.overlay_root())
local space_total = fstat and fstat.blocks or 0
local space_free = fstat and fstat.bfree or 0
local space_used = space_total - space_free

local free_byte = space_free * fstat.frsize

local primary, dark_primary, blur_radius, blur_radius_dark, blur_opacity, mode
if nxfs.access('/etc/config/argon') then
	primary = uci:get_first('argon', 'global', 'primary')
	dark_primary = uci:get_first('argon', 'global', 'dark_primary')
	blur_radius = uci:get_first('argon', 'global', 'blur')
	blur_radius_dark = uci:get_first('argon', 'global', 'blur_dark')
	blur_opacity = uci:get_first('argon', 'global', 'transparency')
	blur_opacity_dark = uci:get_first('argon', 'global', 'transparency_dark')
	mode = uci:get_first('argon', 'global', 'mode')
end

function glob(...)
    local iter, code, msg = nxfs.glob(...)
    if iter then
        return nutil.consume(iter)
    else
        return nil, code, msg
    end
end

local transparency_sets = {
    0,
    0.1,
    0.2,
    0.3,
    0.4,
    0.5,
    0.6,
    0.7,
    0.8,
    0.9,
    1
}

-- [[ 模糊设置 ]]--
br = SimpleForm('config', translate('Argon Theme Settings'), translate('Here you can set the blur and transparency of the login page of argon theme. You also can change theme from Light to Dark.'))
br.reset = false
br.submit = false
s = br:section(SimpleSection) 

o = s:option(ListValue, 'mode', translate('Theme mode'))
o:value('normal', translate('Follow System'))
o:value('light', translate('Force Light'))
o:value('dark', translate('Force Dark'))
o.default = mode
o.rmempty = false
o.description = translate('You can choose Theme color mode here')

o = s:option(Value, 'primary', translate('[Light mode] Primary Color'), translate('A HEX Color ; ( Default: #5e72e4 )'))
o.default = primary
o.datatype = ufloat
o.rmempty = false



o = s:option(ListValue, 'transparency', translate('[Light mode] Login Transparency'), translate('0 transparent - 1 opaque ; ( Suggest: transparent: 0 or translucent preset: 0.5 )'))
for _, v in ipairs(transparency_sets) do
    o:value(v)
end
o.default = blur_opacity
o.datatype = ufloat
o.rmempty = false

o = s:option(Value, 'blur', translate('[Light mode] Login Frosted Glass Radius'), translate('Larger value will more blurred ; ( Suggest:  clear: 1 or blur preset: 10 )'))
o.default = blur_radius
o.datatype = ufloat
o.rmempty = false

o = s:option(Value, 'dark_primary', translate('[Dark mode] Primary Color'), translate('A HEX Color ; ( Default: #483d8b )'))
o.default = dark_primary
o.datatype = ufloat
o.rmempty = false

o = s:option(ListValue, 'transparency_dark', translate('[Dark mode] Login Transparency'), translate('0 transparent - 1 opaque ; ( Suggest: Black translucent preset: 0.5 )'))
for _, v in ipairs(transparency_sets) do
    o:value(v)
end
o.default = blur_opacity_dark
o.datatype = ufloat
o.rmempty = false

o = s:option(Value, 'blur_dark', translate('[Dark mode] Login Frosted Glass Radius'), translate('Larger value will more blurred ; ( Suggest:  clear: 1 or blur preset: 10 )'))
o.default = blur_radius_dark
o.datatype = ufloat
o.rmempty = false

o = s:option(Button, 'save', translate('Save Changes'))
o.inputstyle = 'reload'

function br.handle(self, state, data)
    if (state == FORM_VALID and data.blur ~= nil and data.blur_dark ~= nil and data.transparency ~= nil and data.transparency_dark ~= nil and data.mode ~= nil) then
        nxfs.writefile('/tmp/aaa', data)
        for key, value in pairs(data) do
            uci:set('argon','@global[0]',key,value)
        end 
        uci:commit('argon')
    end
    return true
end

return br, form
