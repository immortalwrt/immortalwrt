include $(TOPDIR)/rules.mk 

PKG_NAME:=luci-app-clash
PKG_VERSION:=1.5.1
PKG_MAINTAINER:=frainzy1477


include $(INCLUDE_DIR)/package.mk

define Package/luci-app-clash
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI app for clash
	DEPENDS:=+luci +luci-base +wget +iptables +coreutils-base64 +coreutils +coreutils-nohup +bash +ipset +libustream-openssl +libopenssl +openssl-util
	PKGARCH:=all
	MAINTAINER:=frainzy1477
endef

define Package/luci-app-clash/description
	Luci Interface for clash.
endef

define Build/Prepare
	po2lmo ${CURDIR}/po/zh-cn/clash.po ${CURDIR}/po/zh-cn/clash.zh-cn.lmo
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/conffiles
/etc/config/clash
endef

define Package/$(PKG_NAME)/prerm
#!/bin/sh
# check if we are on real system
if [ -z "$${IPKG_INSTROOT}" ]; then
    echo "Removing rc.d symlink for clash"
     /etc/init.d/clash disable
     /etc/init.d/clash stop
    echo "Removing firewall rule for clash"
	  uci -q batch <<-EOF >/dev/null
		delete firewall.clash
		commit firewall
EOF
fi

exit 0
endef

define Package/$(PKG_NAME)/preinst
#!/bin/sh

if [ -z "$${IPKG_INSTROOT}" ]; then
	rm -rf /tmp/dnsmasq.d/custom_list.conf 2>/dev/null
	rm -rf /tmp/dnsmasq.clash 2>/dev/null
	mv /etc/config/clash /etc/config/clash.bak 2>/dev/null
	rm -rf /usr/lib/lua/luci/model/cbi/clash 2>/dev/null
	rm -rf /usr/lib/lua/luci/view/clash 2>/dev/null
	rm -rf /usr/share/clash/new_core_version 2>/dev/null
	rm -rf /usr/share/clash/new_clashr_core_version 2>/dev/null
	rm -rf /usr/share/clash/new_luci_version 2>/dev/null
	rm -rf /usr/share/clash/web 2>/dev/null
	mv /usr/share/clash/config/sub/config.yaml /usr/share/clashbackup/config.bak1 2>/dev/null
	mv /usr/share/clash/config/upload/config.yaml /usr/share/clashbackup/config.bak2 2>/dev/null
	mv /usr/share/clash/config/custom/config.yaml /usr/share/clashbackup/config.bak3 2>/dev/null
fi

exit 0
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh

if [ -z "$${IPKG_INSTROOT}" ]; then
	rm -rf /tmp/luci*
	mv /etc/config/clash.bak /etc/config/clash 2>/dev/null
	mv /usr/share/clashbackup/config.bak1 /usr/share/clash/config/sub/config.yaml 2>/dev/null
	mv /usr/share/clashbackup/config.bak2 /usr/share/clash/config/upload/config.yaml 2>/dev/null
	mv /usr/share/clashbackup/config.bak3 /usr/share/clash/config/custom/config.yaml 2>/dev/null
	/etc/init.d/clash disable 2>/dev/null
fi

exit 0
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/clash
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/clash
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/clash
	$(INSTALL_DIR) $(1)/tmp	
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	$(INSTALL_DIR) $(1)/usr/share/
	$(INSTALL_DIR) $(1)/usr/share/clash
	$(INSTALL_DIR) $(1)/usr/share/clash/rules
	$(INSTALL_DIR) $(1)/usr/share/clash/rules/g_rules
	$(INSTALL_DIR) $(1)/usr/share/clash/dashboard
	$(INSTALL_DIR) $(1)/usr/share/clash/dashboard/img
	$(INSTALL_DIR) $(1)/usr/share/clash/dashboard/js
	$(INSTALL_DIR) $(1)/etc/clash/clashtun
	$(INSTALL_DIR) $(1)/usr/share/clashbackup
	$(INSTALL_DIR) $(1)/etc/clash/provider
	$(INSTALL_DIR) $(1)/usr/share/clash/config
	$(INSTALL_DIR) $(1)/usr/share/clash/config/sub
	$(INSTALL_DIR) $(1)/usr/share/clash/config/upload
	$(INSTALL_DIR) $(1)/usr/share/clash/config/custom
	$(INSTALL_DIR) $(1)/usr/share/clash/v2ssr
	
	$(INSTALL_BIN) 	./root/etc/init.d/clash $(1)/etc/init.d/clash
	$(INSTALL_CONF) ./root/etc/config/clash $(1)/etc/config/clash
	$(INSTALL_CONF) ./root/etc/clash/* $(1)/etc/clash/
	
	$(INSTALL_BIN) ./root/usr/share/clash/v2ssr/v2ssr_custom_rule.yaml $(1)/usr/share/clash/v2ssr/
	$(INSTALL_BIN) ./root/usr/share/clash/v2ssr/policygroup $(1)/usr/share/clash/v2ssr/

	$(INSTALL_BIN) ./root/usr/share/clash/*.sh $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/dns.yaml $(1)/usr/share/clash/
	
	$(INSTALL_BIN) ./root/usr/share/clash/rules/g_rules/Steam.rules $(1)/usr/share/clash/rules/g_rules
	$(INSTALL_BIN) ./root//usr/share/clash/rules/rules.list $(1)/usr/share/clash/rules

	$(INSTALL_BIN) ./root/usr/share/clash/tundns.yaml $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/custom_rule.yaml $(1)/usr/share/clash/
	
	$(INSTALL_BIN) ./root/usr/share/clash/luci_version $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/server.list $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/clash_real.txt $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/logstatus_check $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/clash.txt $(1)/tmp/
	
	$(INSTALL_BIN) ./root/usr/share/clash/dashboard/index.html $(1)/usr/share/clash/dashboard/
	$(INSTALL_BIN) ./root/usr/share/clash/dashboard/main.0bddb85299f970595cb5.css $(1)/usr/share/clash/dashboard/
	$(INSTALL_BIN) ./root/usr/share/clash/dashboard/img/33343e6117c37aaef8886179007ba6b5.png $(1)/usr/share/clash/dashboard/img/
	$(INSTALL_BIN) ./root/usr/share/clash/dashboard/js/1.bundle.0bddb85299f970595cb5.min.js $(1)/usr/share/clash/dashboard/js/
	$(INSTALL_BIN) ./root/usr/share/clash/dashboard/js/bundle.0bddb85299f970595cb5.min.js $(1)/usr/share/clash/dashboard/js/
	
	$(INSTALL_DATA) ./luasrc/clash.lua $(1)/usr/lib/lua/luci/
	$(INSTALL_DATA) ./luasrc/controller/*.lua $(1)/usr/lib/lua/luci/controller/
	$(INSTALL_DATA) ./luasrc/model/cbi/clash/*.lua $(1)/usr/lib/lua/luci/model/cbi/clash/
	$(INSTALL_DATA) ./luasrc/view/clash/* $(1)/usr/lib/lua/luci/view/clash/
	$(INSTALL_DATA) ./po/zh-cn/clash.zh-cn.lmo $(1)/usr/lib/lua/luci/i18n/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
