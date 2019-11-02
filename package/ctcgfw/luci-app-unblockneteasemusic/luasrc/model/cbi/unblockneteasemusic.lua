mp = Map("unblockneteasemusic", translate("解除网易云音乐播放限制"))
mp.description = translate("原理：采用 [QQ/虾米/百度/酷狗/酷我/咕咪/JOOX] 等音源，替换网易云音乐 无版权/收费 歌曲链接<br/>具体使用方法参见：https://github.com/project-openwrt/luci-app-unblockneteasemusic")

mp:section(SimpleSection).template = "unblockneteasemusic/unblockneteasemusic_status"

s = mp:section(TypedSection, "unblockneteasemusic")
s.anonymous=true
s.addremove=false

enable = s:option(Flag, "enable", translate("启用本插件"))
enable.description = translate("启用本插件以解除网易云音乐播放限制")
enable.default = 0
enable.rmempty = false

http_port = s:option(Value, "http_port", translate("[HTTP] 监听端口"))
http_port.description = translate("本插件监听的HTTP端口，不可与其他程序/HTTPS共用一个端口")
http_port.placeholder = "5200"
http_port.default = "5200"
http_port.datatype = "port"
http_port.rmempty = false

https_port = s:option(Value, "https_port", translate("[HTTPS] 监听端口"))
https_port.description = translate("本插件监听的HTTPS端口，不可与其他程序/HTTP共用一个端口")
https_port.placeholder = "5201"
https_port.default = "5201"
https_port.datatype = "port"
https_port.rmempty = false

music_source = s:option(ListValue, "music_source", translate("音源接口"))
music_source:value("default", translate("默认"))
music_source:value("qq", translate("QQ音乐"))
music_source:value("xiami", translate("虾米音乐"))
music_source:value("baidu", translate("百度音乐"))
music_source:value("kugou", translate("酷狗音乐"))
music_source:value("kuwo", translate("酷我音乐"))
music_source:value("migu", translate("咕咪音乐"))
music_source:value("joox", translate("JOOX音乐"))
music_source:value("youtube", translate("Youtube音乐"))
music_source.description = translate("音源调用接口")
music_source.default = "default"
music_source.rmempty = false

youtube_key = s:option(Value, "youtube_key", translate("Youtube API Key"))
youtube_key.description = translate("API Key申请地址：https://developers.google.com/youtube/v3/getting-started#before-you-start")
youtube_key.datatype = "string"
youtube_key:depends("music_source", "youtube")

hijack = s:option(ListValue, "hijack_ways", translate("劫持方法"))
hijack:value("dont_hijack", translate("不开启劫持"))
hijack:value("use_ipset", translate("使用IPSet劫持"))
hijack:value("use_hosts", translate("使用Hosts劫持"))
hijack.description = translate("如果使用Hosts劫持，请将HTTP/HTTPS端口设置为80/443")
hijack.default = "dont_hijack"
hijack.rmempty = false

auto_update = s:option(Flag, "auto_update", translate("启用自动更新"))
auto_update.description = translate("启用后会在每天凌晨检测最新版本并自动更新")
auto_update.default = 0
auto_update.rmempty = false

advanced_mode = s:option(Flag, "advanced_mode", translate("启用进阶设置"))
advanced_mode.description = translate("仅推荐高级玩家使用")
advanced_mode.default = 0
advanced_mode.rmempty = false

pub_access = s:option(Flag, "pub_access", translate("部署到公网"))
pub_access.description = translate("默认仅监听局域网，如需提供公开访问请勾选此选项")
pub_access.default = 0
pub_access.rmempty = false
pub_access:depends("advanced_mode", 1)

strict_mode = s:option(Flag, "strict_mode", translate("启用严格模式"))
strict_mode.description = translate("[慎用] 若将服务部署到公网，则强烈建议使用严格模式，此模式下仅放行网易云音乐所属域名的请求")
strict_mode.default = 0
strict_mode.rmempty = false
strict_mode:depends("advanced_mode", 1)

netease_server_ip = s:option(Value, "netease_server_ip", translate("网易云服务器IP"))
netease_server_ip.description = translate("通过 ping music.163.com 即可获得IP地址，仅限填写一个")
netease_server_ip.placeholder = "59.111.181.38"
netease_server_ip.datatype = "ipaddr"
netease_server_ip:depends("set_netease_server_ip", 1)
netease_server_ip:depends("advanced_mode", 1)

endpoint_url = s:option(Value, "endpoint_url", translate("EndPoint"))
endpoint_url.description = translate("具体说明参见：https://github.com/nondanee/UnblockNeteaseMusic")
endpoint_url.placeholder = "https://music.163.com"
endpoint_url.datatype = "string"
endpoint_url:depends("advanced_mode", 1)

proxy_server_ip = s:option(Value, "proxy_server_ip", translate("代理服务器地址"))
proxy_server_ip.description = translate("使用代理服务器获取音乐信息")
proxy_server_ip.placeholder = "http(s)://host:port"
proxy_server_ip.datatype = "string"
proxy_server_ip:depends("advanced_mode", 1)

return mp
