module("luci.controller.filebrowser", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/filebrowser") then
		return
	end
	local page
	page = entry({"admin", "services", "filebrowser"}, cbi("filebrowser"), _("FileBrowser"), 100)
	page.i18n = "filebrowser"
	page.dependent = true
end
