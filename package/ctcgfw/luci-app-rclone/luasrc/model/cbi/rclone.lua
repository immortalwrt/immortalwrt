require('luci.sys')
require('luci.util')
require('luci.model.ipkg')
local fs = require 'nixio.fs'

local uci = require 'luci.model.uci'.cursor()

local m, s

local running = (luci.sys.call('pidof rclone > /dev/null') == 0)

local state_msg = ''
local trport = uci:get('rclone', 'config', 'port')

if running then
    state_msg = '<b><font color="green">' .. translate('rclone运行中') .. '</font></b>'
else
    state_msg = '<b><font color="red">' .. translate('rclone未运行') .. '</font></b>'
end

m =
    Map(
    'rclone',
    translate('Rclone'),
    translate('Rclone是一款的命令行工具，支持在不同对象存储、网盘间同步、上传、下载数据。') ..
        ' <br/> <br/> ' ..
            translate('rclone运行状态') ..
                ' : ' ..
                    state_msg ..
                        '<br/> <br/>' ..
                            translate('已安装的WEB界面') ..
                                '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="button" class="cbi-button" style="margin: 0 5px;" value=" ' ..
                                    translate('Webui React') ..
                                        " \" onclick=\"window.open('http://'+window.location.hostname+'/rclone-webui-react')\"/> <br/><br/>"
)

s = m:section(TypedSection, 'global', translate('全局'))
s.addremove = false
s.anonymous = true

enable = s:option(Flag, 'enabled', translate('启用 Rclone 后台服务'))
enable.rmempty = false

-- user = s:option(ListValue, 'user', translate('以此用户权限运行'))
-- local p_user
-- for _, p_user in luci.util.vspairs(luci.util.split(luci.sys.exec('cat /etc/passwd | cut -f 1 -d :'))) do
--     user:value(p_user)
-- end
-- TODO: user add rclone

s = m:section(TypedSection, 'conf', translate('配置'))
s.addremove = false
s.anonymous = true

o = s:option(ListValue, 'addr_type', translate('监听地址'))
o:value('local', translate('监听本机地址'))
o:value('lan', translate('监听局域网地址'))
o:value('wan', translate('监听全部地址'))
o.default = 'lan'
o.rmempty = false

o = s:option(Value, 'port', translate('监听端口'))
o.placeholder = 5572
o.default = 5572
o.datatype = 'port'
o.rmempty = false

o = s:option(Value, 'config_path', translate('配置文件路径'))
o.placeholder = '/etc/rclone/rclone.conf'
o.default = '/etc/rclone/rclone.conf'
o.rmempty = false

o = s:option(Value, 'username', translate('用户名'))
o.placeholder = 'admin'
o.default = 'admin'
o.rmempty = false

o = s:option(Value, 'password', translate('密码'))
o.password = true
o.placeholder = 'admin'
o.default = 'admin'
o.rmempty = false

s = m:section(TypedSection, 'log', translate('日志'))
s.addremove = false
s.anonymous = true

o = s:option(Value, 'path', translate('路径'))
o.placeholder = '/var/log/rclone/output'
o.default = '/var/log/rclone/output'
o.rmempty = false

return m
