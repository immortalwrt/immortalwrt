module("luci.controller.sms", package.seeall)

function index()
	entry({"admin", "status", "sms"}, template("sms"), _("SMS Tool"), 100)
end

function action_sms()
	luci.template.render("sms")
end

