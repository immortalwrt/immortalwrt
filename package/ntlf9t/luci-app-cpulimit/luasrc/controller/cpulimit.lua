
module("luci.controller.cpulimit", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/cpulimit") then
		return
	end
	
	local page = entry({"admin", "system", "cpulimit"}, cbi("cpulimit"), luci.i18n.translate("cpulimit"), 65)
	page.i18n = "cpulimit"
	page.dependent = true
	
end
