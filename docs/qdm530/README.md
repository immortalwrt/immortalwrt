# Unbranded QDM530 5G CPE (IPQ5018 / AP-MP03.6-C1) — porting reference

A research reference for bringing an unbranded whitelabel 5G CPE (silkscreen
`QDM530-GL`, Qualcomm IPQ5018 + Quectel RG520N-EB modem) up on mainline
ImmortalWrt/OpenWrt. It sold with a vendor QSDK build (OpenWrt Chaos Calmer 15.05.1,
kernel 4.4.60). This document is the "what is this board and where does the port
stand" note for anyone doing the same kind of work — in the spirit of the community
write-ups for boards like the GL-B3000 and the Cudy P5.

> The device carries no vendor MAC in hardware; all MAC/SSID values below are shown as
> generic placeholders. No device serial numbers, IMEI/IMSI/ICCID or other
> per-unit identifiers are included.

## Board identity

| Field | Value |
|---|---|
| SoC | Qualcomm IPQ5018 (dual Cortex-A53 @ ~1.0 GHz, ARM64) |
| Board / RDP | `AP-MP03.6-C1` — Qualcomm RDP434 lineage (GL-B3000 is the mp03.5-c1 sibling) |
| DT compatible | `qcom,ipq5018-mp03.6-c1`, `qcom,ipq5018` (this port: `unbranded,qdm530`) |
| Vendor firmware | OpenWrt Chaos Calmer 15.05.1, kernel 4.4.60 (32-bit armv7 userland, QSDK) |
| ImmortalWrt device | `unbranded_qdm530` (target `qualcommax/ipq50xx`) |

## Hardware

| Component | Detail |
|---|---|
| RAM | ~400 MiB usable (`MemTotal` ~411 MiB; boot log reserves 0x40000000–0x41000000 = 16 MiB) |
| Flash | 1× SPI-NAND 128 MiB (GigaDevice F50D1G41LB, `0xc8 0x11`), SLC, 128 KiB erase, 2048 B page — same architecture as GL-B3000 |
| Ethernet | 2 ports: **lan1** = IPQ5018 internal 1G GE PHY, **lan2** = Realtek RTL8221B 2.5G PHY (both LAN; this is a CPE, WAN is cellular) |
| Wi-Fi | Dual-band, no 6 GHz: 2.4 GHz IPQ5018 built-in + 5 GHz QCN9074 companion. Mainline `ath11k`. |
| Modem | Quectel RG520N-EB (5G NR / LTE) on PCIe1/MHI (`17cb:0308`); USB host is disabled (the combo PCIe/USB3 PHY is committed to pcie1), so the USB AT/DIAG ports are unavailable |
| GPIO/LED | AW9523B I2C expander @ `0x5b` on `78b7000.i2c`; drives the 4 front RGB LEDs in constant-current mode via `leds-aw9523` (see `leds.md`) |
| Console | UART `ttyMSM0` @ 115200 (`78af000.serial`), pins GPIO20/21 (`blsp0_uart0`) |
| Front port | RJ12 6-pin serial console (IPQ5018 `ttyMSM0`), **not** an ethernet port — plug-and-play with the RJ12-to-USB console cable (built-in USB-serial chip) |
| Antennas | 6× u.FL Wi-Fi (more 5 GHz chains than 2.4 GHz), cellular N78 + ANT1–4 |
| Other | USB Type-C (wired to the IPQ5018 host controller, not a device port), SIM slot |

## Boot, console and recovery

- **Serial console** (`ttyMSM0` @ 115200 8N1) is the prerequisite for any first
  flash. It is exposed on the front **RJ12** port: the RJ12-to-USB console cable has
  a built-in USB-serial controller, so it connects plug-and-play. The same UART also
  breaks out to unlabelled PCB pads (used during early bring-up before the cable);
  the modem module has its own separate console UART on other pads. Note the IPQ5018
  UART is 1.8V, so a bare 3.3V adapter on the raw pads needs level shifting — the
  RJ12 cable handles this itself.
- **U-Boot** stops at a password prompt (`passwd_abort`). The password is `quectel`
  (found in the APPSBL binary). Flooding it catches the prompt reliably.
- **U-Boot environment** ships a TFTP recovery config: `ipaddr=192.168.10.10`,
  `serverip=192.168.10.19`, `bootcmd=bootipq`, `bootdelay=5`, `machid=8040003`.
  A host at `192.168.10.19` serving the image over TFTP can RAM-boot or reflash.
  See `tools/tftp-recovery-setup.sh`.
- **A/B slots** exist for every critical partition (`rootfs`/`rootfs_1`,
  `APPSBL`/`APPSBL_1`, `QSEE`/`QSEE_1`, `BOOTCONFIG`/`BOOTCONFIG1`). Writing the
  factory image to *both* rootfs slots boots it regardless of the BOOTCONFIG-selected
  slot, without editing BOOTCONFIG. Slot selection lives in the `BOOTCONFIG`/
  `BOOTCONFIG1` MTD structs (an `echo` to `/proc` does not persist it).
- **External GPIO watchdog gotcha:** the board resets at ~1.5 s unless an external
  watchdog on **TLMM GPIO 26** is fed. This is the single biggest bring-up trap on
  this board — see `bootloop-postmortem.md`.

### NAND partition layout (physical, from `/proc/mtd`)

```
mtd0  0:SBL1        mtd5  0:QSEE_1      mtd10 0:APPSBLENV   mtd15 rootfs
mtd1  0:MIBIB       mtd6  0:DEVCFG      mtd11 0:APPSBL      mtd16 rootfs_1
mtd2  0:BOOTCONFIG  mtd7  0:DEVCFG_1    mtd12 0:APPSBL_1    mtd17 usr_config
mtd3  0:BOOTCONFIG1 mtd8  0:CDT         mtd13 0:ART         mtd18 usr_factory
mtd4  0:QSEE        mtd9  0:CDT_1       mtd14 0:TRAINING
```
`rootfs`/`rootfs_1` are the two ~52 MiB UBI slots; a UBI attached to one exposes the
`kernel`, `wifi_fw`, `bt_fw`, `ubi_rootfs`, `rootfs_data` volumes.

## Port status

| Area | Status | Notes |
|---|---|---|
| Boot to userspace | ✅ | persistent NAND boot, ubifs overlay, shell + SSH |
| Ethernet lan1 (1 G) | ✅ | IPQ5018 internal GE PHY |
| Ethernet lan2 (2.5 G) | ✅ | RTL8221B; native 2.5G peer + 1G rate-matching + hot-plug |
| Wi-Fi 2.4 GHz | ✅ | `ath11k`, broadcasting |
| Wi-Fi 5 GHz | ✅ | QCN9074, UNII-3 only (ch149 / HE20), pinned via an ieee80211 hotplug; the generic board.bin (board_id 0xff, no caldata) gives a self-managed regdomain that disables ch36–144 |
| Modem WAN — PCIe1/MHI | ✅ | RG520N-EB `17cb:0308`; link trains, WAN auto-seeded + dialed by wwand over QMI-over-QRTR (rmnet/QMAP), IPv4 |
| Modem — USB | ❌ | USB host disabled: combo PCIe/USB3 PHY is given to pcie1, and dwc3 won't run without the SS PHY — so no USB AT/DIAG ports |
| Front-panel LEDs | ✅ | 4 RGB LEDs on the AW9523B via `leds-aw9523` (`/sys/class/leds`): power, wan (netdev), mobile/signal + status (daemon). See `leds.md` |
| Watchdog | ✅ | external GPIO WDT on tlmm 26 fed via `gpio-watchdog` |
| Thermal (tsens) | ➖ N/A | no tsens calibration fuses on this board (the vendor's tsens calibration failed too) — disabled; on mainline the uncalibrated probe also Oopses (`__init`-after-free) |

## Modem

The RG520N-EB is driven over **PCIe/MHI**, which carries the WAN; its USB
interface is kept only as a control/diagnostic side channel.

- **PCIe1 / MHI (working WAN).** The controller is `pcie1` (base `0x80000000`,
  1-lane, PHY block at `0x7e000`). The link trains and the modem enumerates
  (`[17cb:0308]`, MHI mission mode). Two fixes made it work: the long "Phy link
  never came up" failure was a **PERST# GPIO polarity** bug (GPIO18 is
  `ACTIVE_LOW`), and the shared **combo PCIe/USB3 PHY** must be left to PCIe by
  disabling `usbphy1` (USB3) — otherwise the TCSR mux hands the PHY to USB and it
  reads all-zero. The modem's QMI has no cdc-wdm node — it lives on the **QRTR**
  bus (`mhi0_IPCR` via `qcom_mhi_qrtr`) — so the WAN is dialed by **wwand**
  through a QMI-over-QRTR transport (`option device 'qrtr'`), with data on
  rmnet/QMAP (`wwand0@mhi_hwip0`, mux_id 1). Seeded on first boot by
  `base-files/etc/uci-defaults/26_wwand-modem`; set a carrier APN there or in the
  wwand LuCI UI if the SIM needs one. See `pcie1-modem-combo-phy.md`.
- **USB (disabled on the board).** The combo PCIe/USB3 PHY is committed to pcie1
  (above), so the USB3 PHY stays off; mainline dwc3 then refuses to register at all
  (there is no USB2-only fallback — `dwc3-qcom: failed to register DWC3 Core`), so the
  SoC USB host controller is disabled in the DTS. Consequence on the board: the modem's
  USB AT/DIAG ports (`2c7c:0801`) do not enumerate and there is no USB-tethering WAN; AT
  where needed goes over QMI instead. Running USB and pcie1 together would need the
  vendor's TCSR-mux arrangement, which is not implemented here.

  This does **not** cut the modem off from USB: the modem's own USB-device port is wired
  out to the board's Type-C connector, so AT/DIAG and `adb` into the modem's internal
  Linux are reachable any time by plugging the Type-C into a laptop with a data cable —
  and with the board's own USB host disabled there is no contention for that port, so it
  enumerates on the laptop cleanly. The WAN (PCIe/MHI) is unaffected by this.

## Open issues

- **5 GHz is UNII-3 only** (channels 149–161, 20 MHz). The QCN9074 board.bin has no
  genuine per-board caldata (`board_id` reads 0xff; the board's `0:ART` is all 0xff),
  so ath11k's self-managed regdomain disables ch36–144; the radio is pinned to
  ch149/HE20 by `hotplug.d/ieee80211/13-qdm530-5ghz-channel`. Full-band 5 GHz would
  need real vendor caldata, which the board does not carry.
- **No SoC thermal sensor (tsens).** A hardware limitation, not a regression: the
  board's calibration fuses are unprogrammed, so the vendor firmware's tsens already
  logged `tsens calibration failed` — thermal sensing never worked on this unit. On
  mainline the uncalibrated tsens probe additionally Oopses (`__init`-after-free, it
  runs after the init sections are freed), so `&tsens` is disabled. Fixing the crash
  would only yield an uncalibrated (useless) reading; the modem carries its own
  thermal mitigation and the SoC is a low-power dual-A53 @1 GHz.
- **Wi-Fi + modem coexistence.** `pcie0` (5 GHz Wi-Fi) and `pcie1` (modem) are both
  enabled; watch for the reported xHCI↔MHI IRQ interaction under simultaneous
  Wi-Fi + modem load.

## Building

This board is `unbranded_qdm530` in `target/linux/qualcommax/ipq50xx`. The modem
stack is **wwand** (daemon + LuCI UI), from the wwand feed already listed in
`feeds.conf.default` (`ddimension/openwrt-repo`):

```sh
./scripts/feeds update -a && ./scripts/feeds install -a
# seed config from the provided diffconfig, then expand
cp qdm530.diffconfig .config && make defconfig
make -j"$(nproc)"
```

See **`build-and-flash.md`** for the full recipe, including the interim wwand feed
override needed until the QMI-over-QRTR PRs merge upstream (and the `PKG_VERSION`
re-download gotcha), the sanity-checks, and the first-flash / sysupgrade steps.

> **Do not bundle `mwan3` (nor `luci-app-mwan3`).** This is a single-WAN cellular
> CPE; with only one uplink, `mwan3track` has nothing to fail over to, so a normal
> brief cellular blip makes it mark the WAN "offline" and **pull the default route**,
> blackholing every client until its ping score recovers — the "connected but no
> internet" symptom. The modem link itself stays healthy throughout (`ping -I wwand0`
> keeps working); `netifd`+`wwand` already handle reconnection. Keep it out of the
> build config.

Key in-tree pieces:

- `target/linux/qualcommax/dts/ipq5018-qdm530.dts`
- `target/linux/qualcommax/image/ipq50xx.mk` (`unbranded_qdm530` device)
- `target/linux/qualcommax/config-6.18`
- `target/linux/qualcommax/ipq50xx/base-files/etc/board.d/02_network`
- `target/linux/qualcommax/ipq50xx/base-files/etc/uci-defaults/` (MAC gen, wireless
  country, `26_wwand-modem` cellular-WAN seed, `28_enable-modem-leds`)
- `target/linux/qualcommax/ipq50xx/base-files/etc/board.d/01_leds` + `usr/sbin/`
  `qdm530-modem-leds` + `etc/init.d/qdm530-modem-leds` (front-panel LEDs — see `leds.md`)
- `package/kernel/leds-aw9523/` (AW9523B constant-current LED driver, `kmod-leds-aw9523`)
- `target/linux/qualcommax/patches-6.18/0955–0957` (SPI-NAND ECC context, UNIPHY
  refclk, GMAC1 rate-matching)
- `package/firmware/ipq-wifi-qdm530/` (board data for `ath11k`)

## References

- GL-B3000 (mp03.5-c1) thread: https://forum.openwrt.org/t/ipq5018-glinet-b3000-info/210771
- IPQ5018 mp03.5-c1 help thread: https://forum.openwrt.org/t/requesting-help-for-ipq5018-mg03-5-c1/188964
- Unbranded IPQ5018 (SmartInc S3000AX): https://forum.openwrt.org/t/adding-support-for-unbranded-ipq5018/251164
- NSS offload on 6.18 for GL-B3000: https://forum.openwrt.org/t/ipq5018-nss-offload-on-kernel-6-18-with-the-upstream-ethernet-stack-gl-b3000/253014
- ImmortalWrt ipq50xx images: https://downloads.immortalwrt.org/snapshots/targets/qualcommax/ipq50xx/
- wwand modem stack: https://github.com/ddimension/wwand

See `build-and-flash.md` for the build/flash recipe, `leds.md` for the front-panel LED
mapping and driver, `modem-modes.md` for reaching/tuning the modem (adb, AT, band lock,
EP vs default mode), `bootloop-postmortem.md` for the watchdog debugging story,
`pcie1-modem-combo-phy.md` for the modem-over-PCIe bring-up (the PERST-polarity fix that
made it train, the combo-PHY/USB3 mux caveat, and the QMI-over-QRTR WAN path), and
`tools/` for the read-only probes used during bring-up.
