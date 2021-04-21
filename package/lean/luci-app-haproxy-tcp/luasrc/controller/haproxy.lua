module("luci.controller.haproxy", package.seeall)
function index()
        if not nixio.fs.access("/etc/config/haproxy") then
                return
        end
        local page = entry({"admin", "services", "haproxy"}, cbi("haproxy"), _("HAProxy"))
	page.dependent = true
	page.acl_depends = { "luci-app-haproxy-tcp" }
end