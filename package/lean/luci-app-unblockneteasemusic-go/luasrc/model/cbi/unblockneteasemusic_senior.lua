local http = luci.http

mp = Map("unblockneteasemusic", translate("解除网易云音乐播放限制 (Golang)"))
mp.description = translate("原理：采用 [酷我/酷狗/咕咪] 音源(后续有空补充)，替换网易云音乐 灰色 歌曲链接<br/>具体使用方法参见：https://github.com/cnsilvan/luci-app-unblockneteasemusic<br/>提示：客户端网易云音乐能用就别升级app，最新版本不一定能用")
mp:section(SimpleSection).template = "unblockneteasemusic/unblockneteasemusic_status"

s = mp:section(TypedSection, "unblockneteasemusic")
s.anonymous=true
s.addremove=false

daemon_enable = s:option(Flag, "daemon_enable", translate("启用进程守护"))
daemon_enable.description = translate("开启后，附属程序会自动检测主程序运行状态，在主程序退出时自动重启")
daemon_enable.default = 0
daemon_enable.rmempty = false

search_limit = s:option(Value, "search_limit", translate("搜索结果限制"))
search_limit.description = translate("在搜索页面显示其他平台搜索结果个数，可填（0-3）")
search_limit.default = "0"
search_limit.rmempty = false

endpoint_enable = s:option(Flag, "endpoint_enable", translate("启用地址转换"))
endpoint_enable.description = translate("开启后，设备需要信任证书，经测试ios设备需要开启，android设备使用咪咕源下载时需要开启，其他情况无法使用时再开启尝试")
endpoint_enable.default = 0
endpoint_enable.rmempty = false

force_best_quality = s:option(Flag, "force_best_quality", translate("强制音质优先"))
force_best_quality.description = translate("开启后，客户端选择音质将失效")
force_best_quality.default = 0
force_best_quality.rmempty = false

auto_update = s:option(Flag, "auto_update", translate("核心库自动更新"))
auto_update.description = translate("")
auto_update.default = 0
auto_update.rmempty = false

delete = s:option(Button,"_delete", translate("删除根证书"))
delete.description = translate("删除证书，以便下次启动时生成，可用于解决过期证书等问题")
delete.inputstyle = "reload"
delete.write = function()
	delete_()
end
download = s:option(Button,"_download", translate("下载根证书"))
download.description = translate("请在客户端信任该证书。该证书由你设备自动生成，安全可靠<br/>IOS信任证书步骤：1. 安装证书--设置-描述文件-安装 2. 通用-关于本机-证书信任设置-启动完全信任")
download.inputstyle = "reload"
download.write = function()
	download_()
end


local currentTagCMD="UnblockNeteaseMusic -v |grep Version|awk '{print $2}'"
local currentRuntimeCMD="UnblockNeteaseMusic -v |grep runtime|awk -F\\( '{print $2}'|awk '{print $3,$4}'|sed -E 's/\)//g'|sed 's/[ \t]*$//g'"

function excute_cmd(cmd)
    local t = io.popen(cmd)
	local ret = t:read("*all")
	t:close()
    return ret
end
function currentVersion()
	local current=excute_cmd(currentTagCMD)
	local runtime=excute_cmd(currentRuntimeCMD)
    auto_update.description=string.format("当前版本:%s(%s)",current,runtime)
end
currentVersion()

function download_()
	local sFile, block
	sFile=nixio.open("/usr/share/UnblockNeteaseMusic/ca.crt","r")
	http.header('Content-Disposition','attachment; filename="ca.crt"')
	http.prepare_content("application/octet-stream")
	while true do
		block=sFile:read(nixio.const.buffersize)
		if(not block)or(#block==0)then
			break
		else
			http.write(block)
		end
	end
	sFile:close()
	http.close()
end
function delete_()
	local sPath, fd
	sPath = "/usr/share/UnblockNeteaseMusic/server.crt"
	fd = os.remove(sPath)
    if not fd then
		delete.description = string.format('删除证书，以便下次启动时生成，可用于解决过期证书等问题<br/><span style="color: red">%s</span>', translate("Couldn't delete file: ") .. sPath)
		return
    end
    delete.description = translate("删除证书，以便下次启动时生成，可用于解决过期证书等问题")
end


return mp
