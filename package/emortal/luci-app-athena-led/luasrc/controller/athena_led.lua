module("luci.controller.athena_led", package.seeall)

function index()
    if not nixio.fs.access("/etc/config/athena_led") then
        return
    end
    entry({ "admin", "system", "athena_led" }, firstchild(), _("Athena LED Ctrl"), 80).dependent = false
    entry({ "admin", "system", "athena_led", "general" }, cbi("athena_led/settings"), _("Base Setting"), 1)

    entry({ "admin", "system", "athena_led", "status" }, call("act_status"))
end

function act_status()
    local e = {}
    e.running = luci.sys.call("pgrep /usr/sbin/athena-led >/dev/null") == 0
    luci.http.prepare_content("application/json")
    luci.http.write_json(e)
end