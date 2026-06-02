# Nginx PHP FastCGI Location Fix Log

Date: 2026-05-27

## Problem

Packages containing PHP files installed under `/www/` fail to work when nginx is the web server instead of uhttpd. PHP files are served as static files (`application/octet-stream`), causing browsers to download them instead of executing them.

**Root cause**: nginx requires explicit `fastcgi_pass` location blocks to route `.php` requests to PHP-FPM/CGI, while uhttpd handles this automatically via its `interpreter` config.

## Fix Pattern

Add a `.locations` file to `/etc/nginx/conf.d/` with nested location structure:

```nginx
location /<web_path>/ {
    index index.php;

    location ~ \.php$ {
        fastcgi_param HTTP_PROXY "";
        include fastcgi_params;
        fastcgi_pass 127.0.0.1:1026;
        fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
    }
}
```

This only affects nginx environments (`include conf.d/*.locations`), no impact on uhttpd.

## Fixed Packages

### 1. muink/luci-app-tinyfilemanager

- **Issue**: Existing `tinyfilemanager.locations` used `location = /tinyfilemanager/` (exact match). PHP 302 redirects to `/tinyfilemanager/index.php?p=` could not match, causing `.php` download.
- **Fix**: Changed to nested `location /tinyfilemanager/` prefix match + inner `location ~ \.php$`.
- **File**: `root/etc/nginx/conf.d/tinyfilemanager.locations`
- **PR**: https://github.com/muink/luci-app-tinyfilemanager/pull/8

### 2. Thaolga/openwrt-nekobox/luci-theme-spectra

- **Issue**: 27 PHP files under `/www/spectra/`, no nginx location config.
- **Fix**: Created `spectra.locations` with nested location structure.
- **File**: `luci-theme-spectra/root/etc/nginx/conf.d/spectra.locations`
- **PR**: https://github.com/Thaolga/openwrt-nekobox/pull/22

### 3. Thaolga/openwrt-nekobox/luci-app-nekobox

- **Issue**: 36 PHP files under `/www/nekobox/`, no nginx location config.
- **Fix**: Created `nekobox.locations` with nested location structure.
- **File**: `luci-app-nekobox/root/etc/nginx/conf.d/nekobox.locations`
- **PR**: https://github.com/Thaolga/openwrt-nekobox/pull/23

### 4. feeds/packages/net/hs20 (hs20-server)

- **Issue**: PHP files installed to `/www/hs20/` from hostapd source, no nginx location config. Package depends on `+uhttpd` explicitly.
- **Fix**: Created `hs20.locations` and modified `Makefile` to install it to `/etc/nginx/conf.d/`.
- **Files**: `files/hs20.locations`, `Makefile` (added install line)
- **PR**: Local modification only (upstream feeds package).

## Not Affected

### Lienol/openwrt-package/luci-app-kodexplorer

- Runs its own standalone nginx instance on a custom port with full `nginx.conf.template`.
- PHP handling is self-contained, not dependent on the system nginx config.
