# QDM530 modem: modes, access and tuning

The board is really **two computers**: the IPQ5018 router (ImmortalWrt) and the Quectel
RG520N-EB modem, which is its own Qualcomm **SDX65 running Linux 5.4** with its own
rootfs, network and web server. This note covers how to reach the modem, its operating
**modes** (how its data path is routed), and modem-side tuning (band lock, signal). It is
written for **our ImmortalWrt image**; where the vendor firmware differs it says so.

| | runs | reach it at |
|---|---|---|
| **Board** | IPQ5018, ImmortalWrt | `ssh root@192.168.1.1` |
| **Modem** | SDX65 "sdxlemur", Linux 5.4 | `adb shell` (over USB-C), or `192.168.225.1` in EP mode |

Confirm which one you are on: `cat /proc/sys/kernel/hostname` → `sdxlemur` = inside the
modem, `ImmortalWrt`/`OpenWrt` = the board.

## Reaching the modem over USB (adb) — no mux

The modem's own USB-**device** port is wired out to the board's **Type-C** connector.

1. Use a **data-capable USB-C cable** (not a charge-only cable — a charge-only cable is
   silent on both ends and wastes hours; test it by plugging a phone into the laptop and
   checking `lsusb`).
2. Plug the Type-C into a laptop. The modem enumerates as `2c7c:0801`:
   ```
   lsusb | grep 2c7c
   adb devices        # -> 89f4338  device
   adb shell          # root (uid=0) inside the modem
   ```

There is **no USB mux / no mux button** — earlier notes that said a button switches the
port were wrong. On our image this just works at any time: the board's own USB host is
disabled (the combo PCIe/USB3 PHY is committed to pcie1 — see `pcie1-modem-combo-phy.md`),
so nothing on the board contends for the modem's USB port, and the Type-C hands it cleanly
to the laptop. **The board keeps its WAN** while you do this: the WAN rides PCIe/MHI, which
is independent of the modem's USB port — no router downtime.

If `adb devices` is empty while `lsusb` shows `2c7c`, the laptop's adb server is holding a
stale handle: `adb kill-server && adb devices`.

## Sending AT to the modem

On **our** port the modem has **no board-side AT tty** — its QMI runs on the QRTR bus
(`mhi0_IPCR`, driven by wwand), there is no `/dev/ttyUSB*` or `/dev/wwan0at0`. So AT
commands go through **adb, from inside the modem**, over `/dev/smd11`:

```sh
adb shell
cat /dev/smd11 > /tmp/o.txt &
printf 'AT+QCFG="pcie/mode"\r' > /dev/smd11 ; sleep 3
kill %1 ; tr -d '\r' < /tmp/o.txt
```

`/dev/smd11` is not a tty, so `microcom`/`stty` fail ("Inappropriate ioctl") — that is
normal, use raw read/write as above. Its output interleaves with the modem's own polling
(`AT+QCAINFO`, `AT+QENG`); ignore the extra lines. This path does not depend on the
board, the vendor's Qlib, or MHI, so it works in every mode.

(Signal numbers do **not** need AT — read them on the board with
`ubus call wwand modem_signal '{"modem":"wwmodem"}'`.)

## Modem modes

The modem routes its data path by three NV settings (stored in the modem, so a **router
reflash does not change them**):

| | pcie/mode | usbnet | QMAPWAC | data_interface |
|---|:---:|:---:|:---:|:---:|
| **default** (our image uses this) | 0 | 0 | 0 | 1,0 |
| **EP / RNDIS** | 1 | 3 | 1 | 1,0 |

Value meanings:
- `pcie/mode` 0 = data over **PCIe/MHI**; 1 = **EP** (data over USB).
- `usbnet` 0 = RMNET/QMI, 1 = ECM, 2 = MBIM, 3 = RNDIS, 5 = NCM.
- `QMAPWAC` 0 = default; 1 = paired with EP.
- `data_interface` 1,0 = data via PCIe, control via USB.

Read the current mode (from `adb shell`): `printf 'AT+QCFG="pcie/mode"\r' > /dev/smd11`
(and `"usbnet"`, `AT+QMAPWAC?`), reading back as above.

### Default mode — keep this for normal use

This is what the port is built around: the modem's data rides **PCIe/MHI**, and wwand
dials the WAN over QMI-over-QRTR (`rmnet`/QMAP, interface `wan` on `wwand0`). Leave the
modem in default mode for day-to-day operation — PCIe/MHI is faster and lighter than
RNDIS, and it is the only mode in which the **board** has internet.

### EP mode — modem data to a laptop, NOT to the board

EP mode moves the modem's data path off PCIe and onto USB (RNDIS), and brings up the
modem's own web UI at `192.168.225.1`. **On our image this takes the WAN away from the
board entirely**: `pcie/mode=1` removes the MHI data path, and the board's USB host is
disabled (combo-PHY → pcie1), so the board cannot pick the RNDIS link up either. With the
Type-C plugged into a laptop, the **laptop** gets the RNDIS link, internet, the modem web
UI and adb; the board goes dark for WAN. So use EP mode only for **laptop-direct** modem
work, never as the board's uplink. (This corrects an earlier note that guessed the board
would keep internet in EP mode — it will not, because board USB is off.)

Switch to EP, from `adb shell` — **verify each value before resetting** (skipping this is
the single biggest time-sink):

```sh
printf 'AT+QCFG="usbnet",3\r'    > /dev/smd11 ; sleep 3
printf 'AT+QCFG="pcie/mode",1\r' > /dev/smd11 ; sleep 3
printf 'AT+QMAPWAC=1\r'          > /dev/smd11 ; sleep 3
# read all three back and confirm 3 / 1 / 1, THEN:
printf 'AT+CFUN=1,1\r' > /dev/smd11      # modem reboots, ~30-60 s
```

Revert to default (the same way, values `0/0/0`, verify, then `AT+CFUN=1,1`). After the
modem reboots in default mode the board's PCIe/MHI WAN returns (wwand re-dials); check
`ifstatus wan` and `ping -I wwand0 1.1.1.1`.

## Band lock and signal

These live in the modem's NV and are already set (survey 2026-09-07 for XL Axiata 5G NSA
at this location); a router reflash keeps them.

```
lte_band   3          Band 3 / 1800 MHz — the LTE anchor the NSA 5G hangs off
nr5g_band  41         Band 41 / 2500 MHz — the 5G carrier
mode_pref  LTE:NR5G   4G+5G only (3G/2G dropped)
```

Set/read them via adb → `/dev/smd11` (`AT+QNWPREFCFG="lte_band",...` etc.). To widen the
search back to factory:

```
AT+QNWPREFCFG="lte_band",1:3:5:7:8:20:28:32:38:40:41:42:43:71
AT+QNWPREFCFG="nr5g_band",1:3:5:7:8:20:28:38:40:41:71:75:76:77:78
AT+QNWPREFCFG="mode_pref",AUTO
```

**Do NOT set `mode_pref` to `NR5G`.** XL is 5G **NSA** — LTE and NR run together (Band 3
anchors, Band 41 adds capacity); there is no "5G-only" here. Forcing NR5G drops the LTE
anchor and loses signal entirely rather than "preferring 5G".

Reading signal (RSRP is the limiting factor; SINR sets speed):

| RSRP (dBm) | | SINR (dB) | |
|---|---|---|---|
| > −80 | excellent | > 20 | excellent |
| −80…−90 | good | 13…20 | good |
| −90…−100 | fair | 0…13 | fair |
| −100…−110 | weak | < 0 | poor |
| < −110 | very weak | | |

On the board: `ubus call wwand modem_signal '{"modem":"wwmodem"}'` → `lte`/`nr5g`
`rsrp`/`rssi`/`rsrq`/`snr`. (The front **mobile LED** colours itself from this — see
`leds.md`.) To confirm 5G is actually attached, from adb: `AT+QCAINFO` — an `NR5G` line in
the output means 5G is carrier-aggregated in.

A **PCI/cell lock** (`AT+QNWLOCK="common/4g",1,<earfcn>,<pci>`) is deliberately *not* set:
the modem already camps on the strongest cell here, so there is nothing to gain. Only use
it if the link periodically drops onto a weaker neighbour. Never copy the vendor
`/etc/rc.local` example (PCI 215/150 — from a different location).

## QManager inside the modem

The modem shipped with **QManager** installed (by a previous owner/seller) — it runs
*inside the modem*, not on the router. It keeps an AT poller hammering the modem every
2–30 s, which litters AT output. It is unrelated to the board firmware. To quiet it,
from `adb shell` (reversible, deletes nothing):

```sh
systemctl mask qmanager-poller qmanager-ping
systemctl stop qmanager-poller qmanager-ping
```

(Its enable symlink is under `/lib`, so `systemctl disable` is a no-op — `mask` is the
working way.) The board does not need QManager at all: modem status/SMS/AT come from
wwand + `luci-app-wwand` on the router.

## Flashing — the two storages are separate

Flashing ImmortalWrt touches **only the board's** SPI-NAND. The modem's firmware lives on
its **own** UBI storage inside the RG520N-EB module. A clean ImmortalWrt flash needs no
modem cleanup, and reflashing the modem (Quectel QFLASH, Windows) is a separate, riskier
operation — keep them apart. QManager can stay installed; it does not affect the board.
