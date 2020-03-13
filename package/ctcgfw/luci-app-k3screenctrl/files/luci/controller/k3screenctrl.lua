-- Copyright (C) 2018 XiaoShan mivm.cn

module("luci.controller.k3screenctrl", package.seeall)

function index()
    if not nixio.fs.access("/etc/config/k3screenctrl") then
        return
    end
    local page
    page = entry({"admin","system","k3screenctrl"}, cbi("k3screenctrl"), _("Screen"), 60)
    page.dependent = true
end
