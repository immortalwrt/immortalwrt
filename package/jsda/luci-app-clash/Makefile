include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-clash
PKG_VERSION:=1.1.7
PKG_RELEASE:=2
PKG_MAINTAINER:=frainzy1477


include $(INCLUDE_DIR)/package.mk

define Package/luci-app-clash
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI app for clash
	DEPENDS:=+luci +luci-base +wget +iptables +coreutils +coreutils-nohup +bash +ipset +libustream-openssl +libopenssl +openssl-util +clash
	PKGARCH:=all
	MAINTAINER:=frainzy1477
endef

define Package/luci-app-clash/description
	Luci Interface for clash.
endef


define Build/Prepare
	chmod 777 -R ${CURDIR}/tools/po2lmo
	${CURDIR}/tools/po2lmo/src/po2lmo ${CURDIR}/po/zh-cn/clash.po ${CURDIR}/po/zh-cn/clash.zh-cn.lmo

endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/preinst
#!/bin/sh
if [ -f "/tmp/dnsmasq.d/custom_list.conf" ]; then
	rm -rf /tmp/dnsmasq.d/custom_list.conf
fi

if [ -d "/tmp/dnsmasq.clash" ]; then
	rm -rf /tmp/dnsmasq.clash
fi

if [ -f "/etc/config/clash" ]; then
	mv /etc/config/clash /etc/config/clash.bak
fi

if [ -d "/usr/lib/lua/luci/model/cbi/clash" ]; then
	rm -rf /usr/lib/lua/luci/model/cbi/clash
fi	

if [ -d "/usr/lib/lua/luci/view/clash" ]; then
	rm -rf /usr/lib/lua/luci/view/clash
fi

if [ -f /usr/share/clash/new_core_version ]; then
	rm -rf /usr/share/clash/new_core_version
fi

if [ -f /usr/share/clash/new_luci_version ]; then
	rm -rf /usr/share/clash/new_luci_version
fi

if [  -d /usr/share/clash/web ]; then
	rm -rf /usr/share/clash/web
fi

if [  -f /usr/share/clash/config/sub/config.yaml ] && [ "$(ls -l /usr/share/clash/config/sub/config.yaml | awk '{print int($5/1024)}')" -ne 0 ];then
	mv /usr/share/clash/config/sub/config.yaml /usr/share/clash/config/sub/config.bak
fi

if [  -f /usr/share/clash/config/upload/config.yaml ] && [ "$(ls -l /usr/share/clash/config/upload/config.yaml | awk '{print int($5/1024)}')" -ne 0 ];then
	mv /usr/share/clash/config/upload/config.yaml /usr/share/clash/config/upload/config.bak
fi
 
if [  -f /usr/share/clash/config/custom/config.yaml ] && [ "$(ls -l /usr/share/clash/config/custom/config.yaml | awk '{print int($5/1024)}')" -ne 0 ];then
	mv /usr/share/clash/config/custom/config.yaml /usr/share/clash/config/custom/config.bak
fi
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
rm -rf /tmp/luci*

if [ -f "/etc/config/clash.bak" ]; then
	mv /etc/config/clash.bak /etc/config/clash
fi

if [  -f /usr/share/clash/config/sub/config.bak ];then
	mv /usr/share/clash/config/sub/config.bak /usr/share/clash/config/sub/config.yaml
fi

if [  -f /usr/share/clash/config/upload/config.bak ];then
	mv /usr/share/clash/config/upload/config.bak /usr/share/clash/config/upload/config.yaml
fi
 
if [  -f /usr/share/clash/config/custom/config.bak ];then
	mv /usr/share/clash/config/custom/config.bak /usr/share/clash/config/custom/config.yaml
fi
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/clash
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/clash
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/clash
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	$(INSTALL_DIR) $(1)/usr/share/
	$(INSTALL_DIR) $(1)/usr/share/clash
	$(INSTALL_DIR) $(1)/usr/share/clash/yac
	$(INSTALL_DIR) $(1)/usr/share/clash/dashboard
	$(INSTALL_DIR) $(1)/usr/share/clash/dashboard/img
	$(INSTALL_DIR) $(1)/usr/share/clash/dashboard/js
	
	$(INSTALL_DIR) $(1)/usr/share/clash/config
	$(INSTALL_DIR) $(1)/usr/share/clash/config/sub
	$(INSTALL_DIR) $(1)/usr/share/clash/config/upload
	$(INSTALL_DIR) $(1)/usr/share/clash/config/custom
	
	$(INSTALL_BIN) ./root/usr/share/clash/config/upload/config.yaml $(1)/usr/share/clash/config/upload/
	$(INSTALL_BIN) ./root/usr/share/clash/config/custom/config.yaml $(1)/usr/share/clash/config/custom/
	$(INSTALL_BIN) ./root/usr/share/clash/config/sub/config.yaml $(1)/usr/share/clash/config/sub/

	$(INSTALL_BIN) 	./root/etc/init.d/clash $(1)/etc/init.d/clash
	$(INSTALL_CONF) ./root/etc/config/clash $(1)/etc/config/clash
	$(INSTALL_CONF) ./root/etc/clash/* $(1)/etc/clash/

	$(INSTALL_BIN) ./root/usr/share/clash/clash-watchdog.sh $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/clash.sh $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/ipdb.sh $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/get_proxy.sh $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/proxy.sh $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/dns.yaml $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/rule.yaml $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/custom_rule.yaml $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/luci_version $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/check_luci_version.sh $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/check_core_version.sh $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/yum_change.sh $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/groups.sh $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/rule.sh $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/list.sh $(1)/usr/share/clash/
	$(INSTALL_BIN) ./root/usr/share/clash/server.list $(1)/usr/share/clash/

	$(INSTALL_BIN) ./root/usr/share/clash/yac/* $(1)/usr/share/clash/yac/
	$(INSTALL_BIN) ./root/usr/share/clash/dashboard/index.html $(1)/usr/share/clash/dashboard/
	$(INSTALL_BIN) ./root/usr/share/clash/dashboard/main.557c7e0375c2286ea607.css $(1)/usr/share/clash/dashboard/
	$(INSTALL_BIN) ./root/usr/share/clash/dashboard/img/33343e6117c37aaef8886179007ba6b5.png $(1)/usr/share/clash/dashboard/img/
	$(INSTALL_BIN) ./root/usr/share/clash/dashboard/js/1.bundle.557c7e0375c2286ea607.min.js $(1)/usr/share/clash/dashboard/js/
	$(INSTALL_BIN) ./root/usr/share/clash/dashboard/js/bundle.557c7e0375c2286ea607.min.js $(1)/usr/share/clash/dashboard/js/
        
	$(INSTALL_DATA) ./luasrc/clash.lua $(1)/usr/lib/lua/luci/
	$(INSTALL_DATA) ./luasrc/controller/*.lua $(1)/usr/lib/lua/luci/controller/
	$(INSTALL_DATA) ./luasrc/model/cbi/clash/*.lua $(1)/usr/lib/lua/luci/model/cbi/clash/
	$(INSTALL_DATA) ./luasrc/view/clash/* $(1)/usr/lib/lua/luci/view/clash/
	$(INSTALL_DATA) ./po/zh-cn/clash.zh-cn.lmo $(1)/usr/lib/lua/luci/i18n/
	
endef



$(eval $(call BuildPackage,luci-app-clash))
