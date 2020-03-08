module("luci.controller.nfs", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/nfs") then
		return
	end
	
	local page = entry({"admin", "services", "nfs"}, cbi("nfs"), luci.i18n.translate("NFS"))
	page.i18n = "nfs"
	page.dependent = true
	
	
	local page = entry({"mini", "services", "nfs"}, cbi("nfs", {autoapply=true}), luci.i18n.translate("NFS"))
	page.i18n = "nfs"
	page.dependent = true
end
