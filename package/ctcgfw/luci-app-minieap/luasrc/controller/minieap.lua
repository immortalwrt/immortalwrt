module("luci.controller.minieap", package.seeall)

function index()
    if not nixio.fs.access("/etc/config/minieap") then
        return
    end
    if luci.sys.call("command -v minieap >/dev/null") ~= 0 then
        return
    end
    entry({"admin", "services", "minieap"},
        alias("admin", "services", "minieap", "general"),
        _("minieap"), 10).dependent = true

    entry({"admin", "services", "minieap", "general"}, cbi("minieap/general"), _("minieap Settings"), 10).leaf = true
    -- entry({"admin", "services", "minieap", "customfile"}, cbi("minieap/customfile"), _("custom configfiles"), 20).leaf = true
    entry({"admin", "services", "minieap", "log"}, cbi("minieap/log"), _("minieap LOG"), 30).leaf = true
end
