module("luci.controller.baidupcs", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/baidupcs") then
		return
	end
	local page
	page = entry({"admin", "services", "baidupcs"}, cbi("baidupcs"), _("Baidupcs Setting"), 100)
	page.i18n = "baidupcs"
	page.dependent = true
end
