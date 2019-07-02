NAME = luci-app-rtorrent
VERSION = $(shell awk '/^Version:/ {print $$2}' control/control)
ARCH = $(shell awk '/^Architecture:/ {print $$2}' control/control)
IPK = $(NAME)_$(VERSION)_$(ARCH).ipk

.PHONY: control

all: packages-file

ipk: clean control data
	mkdir -p ipk
	echo "2.0" > ipk/debian-binary
	cd ipk && tar czf $(IPK) control.tar.gz data.tar.gz debian-binary
	rm -f ipk/*.tar.gz ipk/debian-binary

control:
	mkdir -p ipk
	rm -f ../ipk/control.tar.gz
	cd control && tar czf ../ipk/control.tar.gz *

data:
	mkdir -p ipk
	cd src && tar czf ../ipk/data.tar.gz *

key:
	mkdir -p key
	usign -G -s key/sign_key -p key/sign_key.pub -c "$(NAME) key"
	cd key && usign -F -p sign_key.pub | xargs cp sign_key.pub

packages-file: ipk
	mkdir -p ipk
	cp control/control ipk/Packages
	echo "Filename: $(IPK)" >> ipk/Packages
	du -b ipk/$(IPK) | awk '{print "Size:", $$1}' >> ipk/Packages
	md5sum ipk/$(IPK) | awk '{print "MD5Sum:", $$1}' >> ipk/Packages
	sha256sum ipk/$(IPK) | awk '{print "SHA256sum:", $$1}' >> ipk/Packages
	usign -S -m ipk/Packages -s key/sign_key
	gzip ipk/Packages

clean:
	rm -fr ipk

test-deploy: test-remove
	cp -a src/usr/* /usr
	cp -a src/www/* /www
	rm -fr /tmp/luci-indexcache /tmp/luci-modulecache

test-remove:
	find src/usr src/www -type f -o -type l | sed 's/^src//' | xargs rm -f
	rm -fr /usr/lib/lua/luci/model/cbi/rtorrent
	rm -fr /usr/lib/lua/luci/view/rtorrent
	rm -fr /usr/lib/lua/xmlrpc
	rm -fr /www/luci-static/resources/icons/filetypes
	rm -fr /tmp/luci-indexcache /tmp/luci-modulecache

test-reinstall:
	opkg list-installed | grep -q $(NAME) \
	&& opkg --force-reinstall install $(NAME) \
	|| echo "$(NAME) not installed, skip reinstall"

