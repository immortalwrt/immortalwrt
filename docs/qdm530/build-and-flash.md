# QDM530 — building and flashing

How to build an ImmortalWrt image for the Unbranded QDM530 5G CPE and get it onto
the board. Read `README.md` first for the board overview; this is the hands-on
build/flash recipe.

> **Interim note (until the wwand PRs land).** The cellular WAN uses a
> **QMI-over-QRTR** transport that is not in the upstream wwand release yet — it is
> in flight as two PRs (the daemon at `ddimension/wwand`, the packaging at
> `ddimension/openwrt-repo`). Until both merge, the build needs a small local
> override of the wwand feed (below). Once they merge, skip the override and the
> stock feed just works.

## 1. Feeds

`feeds.conf.default` already lists the wwand feed:

```
src-git wwand https://github.com/ddimension/openwrt-repo.git
```

Update and install feeds as usual:

```sh
./scripts/feeds update -a
./scripts/feeds install -a
```

## 2. wwand feed override (interim — skip once upstream merges)

The stock wwand feed builds wwand from `ddimension/wwand`, which does **not** yet
contain `qmi_over_qrtr.uc` (the QRTR transport this board's modem needs). Point the
feed at the fork that has it and tell the package to install it. Edit
**`feeds/wwand/wwand/Makefile`** (this is the local feed clone, done *after*
`feeds update wwand` has cloned it):

```make
# was ddimension/wwand @ fcab7f9 (PKG_VERSION 1.6.8_p77)
PKG_VERSION:=1.6.8_p79
PKG_SOURCE_URL:=https://github.com/xhudan/wwand.git
PKG_SOURCE_VERSION:=8cc6bce8527a4a1707a1f53a4eca1f082d8417f5
PKG_MIRROR_HASH:=skip
```

and add `qmi_over_qrtr.uc` to the QMI backend's install list:

```make
define Package/wwand-qmi/install
	$(INSTALL_DIR) $(1)$(UCDIR)
	$(INSTALL_DATA) $(WWAND_UCODE)/modem.uc $(1)$(UCDIR)/
	$(INSTALL_DATA) $(WWAND_UCODE)/context.uc $(1)$(UCDIR)/
	$(INSTALL_DATA) $(WWAND_UCODE)/qmi_backend.uc $(1)$(UCDIR)/
	$(INSTALL_DATA) $(WWAND_UCODE)/callend.uc $(1)$(UCDIR)/
	$(INSTALL_DATA) $(WWAND_UCODE)/qmi_lazy.uc $(1)$(UCDIR)/
	$(INSTALL_DATA) $(WWAND_UCODE)/qmi_over_qrtr.uc $(1)$(UCDIR)/   # <-- add
endef
```

> ### ⚠️ Gotcha: bumping the commit alone does NOT re-download
> wwand's source tarball in `dl/` is named by **`PKG_VERSION`**
> (`dl/wwand-<PKG_VERSION>.tar.zst`), *not* by `PKG_SOURCE_VERSION`. So if you
> change only `PKG_SOURCE_VERSION`, OpenWrt finds the existing tarball by its
> `PKG_VERSION` name and **reuses the stale source** — the build then fails with
> `install: cannot stat '.../qmi_over_qrtr.uc': No such file or directory` because
> the old checkout never had that file. Always bump **`PKG_VERSION`** too (and if in
> doubt, `rm dl/wwand-*.tar.zst`). The version convention is `X.Y.Z_pN` = N commits
> after tag `vX.Y.Z`; `fcab7f9` is `p77` and the fork is two commits past it, hence
> `p79`.

> ### ⚠️ Do not re-run `feeds update wwand` after editing
> `./scripts/feeds update wwand` resets the feed clone to its upstream state and
> **discards these edits**. After editing the Makefile, use `./scripts/feeds install
> wwand` only. If you do update by accident, re-apply the two edits above before
> building.

(The GitHub archive API may 403/404 for a fork commit; the build falls back to a
plain `git clone` + `git checkout`, which works — the download is not actually
failing.)

## 3. Configure and build

A ready-made build seed lives in `qdm530.diffconfig` (web UI + relayd repeater on top
of the board defaults; the modem stack and board firmware — including `kmod-tun` —
come from `DEVICE_PACKAGES`, so they are not listed there):

```sh
cp qdm530.diffconfig .config
make defconfig
make -j"$(nproc)"
```

Sanity-check the resulting `.config` before the long build if you like:

```sh
grep -E 'DEVICE_unbranded_qdm530=y|PACKAGE_wwand=y|PACKAGE_wwand-mhi=y|PACKAGE_kmod-qrtr=y|WWAND_UCODE_SOURCE' .config
# expect the packages =y and `# CONFIG_WWAND_UCODE_SOURCE is not set` (bytecode build)
```

Output lands in `bin/targets/qualcommax/ipq50xx/`:

- `...-unbranded_qdm530-squashfs-sysupgrade.bin` — upgrade an already-flashed board
- `...-unbranded_qdm530-squashfs-factory.ubi` — first install over the vendor firmware
- `...-unbranded_qdm530-initramfs-uImage.itb` — RAM-boot image for recovery/bring-up

Confirm the modem stack made it into the image:

```sh
grep -E '^wwand|kmod-qrtr|luci-(app|proto)-wwand' \
  bin/targets/qualcommax/ipq50xx/*unbranded_qdm530*.manifest
```

## 4. First flash (from the vendor firmware)

The first flash needs the **serial console** (front RJ12 port, see `README.md` →
"Boot, console and recovery") because U-Boot stops at a password prompt
(`quectel`). Two routes:

- **TFTP RAM-boot then sysupgrade (recommended).** Serve the `initramfs-uImage.itb`
  from a host at `192.168.10.19` (the U-Boot `serverip`), RAM-boot it from the
  U-Boot prompt, then from the booted initramfs run `sysupgrade` with the
  `factory.ubi` (or write the UBI to both `rootfs`/`rootfs_1` slots). See
  `tools/tftp-recovery-setup.sh` and `bootloop-postmortem.md`.
- **Write the factory UBI to NAND** from a RAM-booted initramfs over SSH/serial.

Keep the vendor firmware backed up (dump the MTD partitions) before overwriting.

## 5. Upgrading an already-flashed board

```sh
# copy the sysupgrade.bin to the board, then on the board:
sysupgrade -n /tmp/immortalwrt-...-unbranded_qdm530-squashfs-sysupgrade.bin
```

Use **`-n` (reset config)** when you want the board to come up on its shipped
defaults — in particular so the first-boot `uci-defaults` run, including
`26_wwand-modem`, re-seeds the cellular WAN. A config-preserving upgrade (without
`-n`) keeps your existing `/etc/config/network` and does **not** re-run those
scripts.

## 6. Verify

After boot (LAN default is `192.168.1.1`):

```sh
# cellular WAN was auto-seeded and dialed
uci show network.wwmodem network.wan
ifstatus wan | grep -E '"up"|address'
ping -c3 -I wwand0 8.8.8.8

# both Wi-Fi bands (2.4 GHz built-in + 5 GHz QCN9074 on pcie0)
iw dev; logread | grep -iE 'ath11k|wifi'

# a LAN/Wi-Fi client should reach the internet (interface is named `wan`,
# so it lands in the `wan` firewall zone and is NAT'd automatically)
```

If the WAN does not come up, check `logread -e wwand` — the QMI-over-QRTR bring-up
logs its modem-node discovery and the WDA data-format negotiation there.
