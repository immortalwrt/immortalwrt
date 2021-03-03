local m, up_core

m = SimpleForm("Version")
m.reset = false
m.submit = false

up_core=m:field(DummyValue,"update_luci",translate("更新LuCI")) 
up_core.rawhtml  = true
up_core.template = "unblockneteasemusic/update_luci"
up_core.value = translate("未检查") 
up_core.description = "更新完毕后请手动刷新该界面"

up_core=m:field(DummyValue,"update_core",translate("更新主程序")) 
up_core.rawhtml  = true
up_core.template = "unblockneteasemusic/update_core"
up_core.value = translate("未检查") 
up_core.description = "更新完毕后会自动在后台重启插件，无需手动重启"

return m
