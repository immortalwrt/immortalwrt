# QDM530 modem-over-PCIe (pcie1) — bring-up, and the PERST-polarity trap

How the Quectel RG520N-EB modem was brought up over PCIe/MHI on mainline: what the
real blocker turned out to be (a one-line PERST GPIO polarity bug), the combo-PHY
sub-issue that has to be understood if you ever enable USB3, and what is still left
(bringing the WAN up over MHI). Written for the next person (or a future us) so the
hard-won parts are not re-derived.

## TL;DR

- **The modem trains on `pcie1` and reaches MHI mission mode.** dmesg:
  `PCIe Gen.2 x1 link up`, `pci 0001:01:00.0: [17cb:0308] PCIe Endpoint`,
  `mhi-pci-generic 0001:01:00.0: MHI PCI device found: qcom-sdx65m`,
  `mhi mhi0: Power on setup success`; LTSSM = `L0 (0x11)`.
- **Root cause of the long "Phy link never came up" saga: PERST# GPIO polarity.**
  `perst-gpios` on GPIO18 was set `GPIO_ACTIVE_HIGH`, which made
  `qcom_perst_assert(..., false)` (deassert, gpiod value 0) drive the pin
  **physically LOW** = PERST# asserted = the modem PCIe endpoint held in reset
  forever. The host then just cycled in Detect (LTSSM `DETECT_QUIET`) with no
  receiver. The fix is one line: **`GPIO_ACTIVE_LOW`** (standard PCIe; deassert →
  physical HIGH → endpoint released), matching the Cudy P5.
- IPQ5018 has **two** PCIe controllers: `pcie0` @`a0000000` (2-lane, PHY @`0x86000`,
  the QCN9074 5 GHz Wi-Fi) and `pcie1` @`0x80000000` (1-lane, PHY @`0x7e000`, the
  modem). Both train.
- **Separately**, `pcie1`'s PHY is a PCIe/USB3 *combo* PHY. If (and only if) you
  enable USB3 (`usbphy1`), the USB-SS driver steals the combo PHY via a TCSR mux and
  the `pcie1` PHY block goes dead. With USB3 off (the current config) the PHY is
  fine. See "The combo-PHY sub-issue" below — it was a red herring for *training*
  but it is real, and it matters the moment USB3 is turned on.

## The fix

`ipq5018-qdm530.dts`, `&pcie1`:
```
perst-gpios = <&tlmm 18 GPIO_ACTIVE_LOW>;   /* was GPIO_ACTIVE_HIGH */
```
`qcom_perst_assert()` (in `pcie-qcom.c`) asserts with gpiod value 1 and deasserts
with 0. PERST# is active-low, so the endpoint is released when the pin is physically
HIGH → the DT polarity must be `ACTIVE_LOW` so that deassert (0) → HIGH.

Why it was missed for so long: an earlier bring-up flipped it to `ACTIVE_HIGH` on
the theory that "the link never came up under `ACTIVE_LOW`" meant the pin needed the
other polarity. That was a misdiagnosis — the modem endpoint only presents itself a
little while after power-on, so **both** polarities log `Phy link never came up` at
the ~1.6 s probe. The distinguishing evidence only appeared once `pcie1`'s LTSSM was
read directly (`CONFIG_PCIE_DW_DEBUGFS`, `.../ltssm_status`): stuck at `DETECT_QUIET`
= host up but no endpoint receiver = endpoint held in reset = wrong PERST polarity.

The modem here trains at ~1.68 s, no long wait needed, because it is soldered and
always powered (its USB side is up by ~15 s regardless). On a board where the modem
is cold at boot (e.g. an M.2 card, like the Cudy P5) the endpoint can take ~100 s to
appear and may need a power-cycle to leave PBL — not the case for this board.

## The combo-PHY sub-issue (only relevant with USB3 on)

```
                 TCSR mux 0x10540 bit0
                 (0 = PCIe, 1 = USB)
                        |
   USB-SS PHY  5d000 ---+--- combo PHY ---+--- pcie1 (1-lane) view @ 0x7e000  (modem)
   (usbphy1)
   USB-HS PHY  5b000  ------------------------ dwc3 USB2 (independent, HS)
   PCIe0 PHY   86000  ------------------------ pcie0 (2-lane, dedicated)      (QCN9074 WiFi)
```

- `pcie0`'s PHY (`0x86000`) is **dedicated** → always alive.
- `pcie1`'s PHY (`0x7e000`) is the **combo** PHY, muxed with USB3. Whoever the TCSR
  mux (`0x10540` bit0) points at owns it. The mainline USB-SS driver
  (`phy-qcom-uniphy-usb-ss-22ull.c`) calls `qcom_uniphy_usb_mux_select(uniphy, true)`
  on probe → mux = USB → the `pcie1` PHY block reads all-zero and the modem cannot
  train.
- The USB2 (HS) PHY (`0x5b000`) is a separate block, but it is **not** a usable
  fallback on its own: mainline `dwc3` refuses to register its core without the
  SuperSpeed PHY (`dwc3-qcom: failed to register DWC3 Core`; dropping `usb3-phy`
  from the dwc3 `phys` list only makes xhci fail `can't setup: -110`). So once the
  combo PHY is given to `pcie1`, the whole USB controller is dead — there is **no
  USB2-only mode**.

The current config keeps `usbphy1` (USB3) **disabled** so the combo PHY belongs to
`pcie1` and the modem trains; and because that leaves dwc3 unable to register, the
USB controller (`&usb`) and its HS PHY (`&usbphy0`) are **disabled** too, so USB
does not sit in a permanent deferred-probe failure. The cost is no USB host at all
(the modem's USB AT/DIAG ports do not enumerate, and there is no USB-tethering WAN).
**To run USB and `pcie1` at once** you must stop the USB-SS driver from flipping the
mux — patch `qcom_uniphy_usb_mux_select()` to always write `0` (the vendor-equivalent
state that keeps both the combo PHY and dwc3 alive) — then re-enable
`&usb`/`&usbphy0`. This is not implemented here.

## How the WAN is dialed: QMI-over-QRTR

The modem is fully up on MHI (`mhi0_IP_HW0` → the `mhi_hwip0` rmnet netdev, plus
`mhi0_IPCR` carrying QMI on the **QRTR** bus via `qcom_mhi_qrtr`). The data call is
dialed by **wwand**.

The catch that shaped this: **`quectel-CM` (the dialer QModem uses) cannot drive a
mainline-MHI modem.** It expects the Quectel out-of-tree driver's `/dev/mhi_QMI`,
whereas on this modem the QMI service is not a cdc-wdm / `/dev/wwan0qmi0` chardev at
all — it lives on the QRTR bus. So wwand grew a **QMI-over-QRTR transport**: it opens
an `AF_QIPCRTR` socket, discovers the modem's QMI services (WDS/DMS/NAS/WDA/…) by
node, and runs the full QMI stack over QRTR (emulating the CTL service locally, since
QRTR has none). The data format is negotiated with WDA `SET_DATA_FORMAT` using the
PCIE endpoint (`ep_type=3`, `ep_id=4`) and QMAP **v1** (the modem rejects QMAPv5 over
MHI), then rmnet/QMAP brings up `wwand0@mhi_hwip0` (mux_id 1). Configuration is the
single sentinel `option device 'qrtr'` on a `wwand_modem` section — see
`base-files/etc/uci-defaults/26_wwand-modem`. Upstreaming is in flight at
`ddimension/wwand`.

## Coexistence note (pcie0 + pcie1)

Both PCIe controllers run together (the vendor firmware runs Wi-Fi on `pcie0` and
the modem on `pcie1` at once), and `pcie0` (5 GHz Wi-Fi) is enabled alongside
`pcie1` in the DTS. The Cudy P5 thread reports an IRQ conflict (xHCI ↔ MHI) that can
crash the 5 GHz radio when both are up; worth watching on air under simultaneous
Wi-Fi + modem load.

## Diagnostics used during bring-up (since reverted)

The **keeper** is the one-line `perst-gpios ... GPIO_ACTIVE_LOW` change (plus
`usbphy1` disabled for the combo-PHY). The following were diagnostics only and have
been reverted:

- `0958-DIAG-pcie1-phy-block-dump-experiment.patch` — dumped the PHY block after init
  (`PHY-DIAG l<lanes> +0x<off>: ...`), to see alive vs all-zero.
- `CONFIG_PCIE_DW_DEBUGFS=y` in `config-6.18` — exposed `.../ltssm_status` (how
  `DETECT_QUIET` vs `L0` was read; invaluable, but experimental).
- `/delete-node/ &pcie0;` in the DTS — deleted the Wi-Fi controller to isolate
  `pcie1` during testing (`status="disabled"` alone is re-enabled by the U-Boot FDT
  fixup; `/delete-node/` is what actually removes it). Reverted to restore 5 GHz
  Wi-Fi.
- `&usbphy1 { status = "disabled"; }` — decide: keeping USB3 off is fine, or
  re-enable with the mux-force-PCIe patch above.

## Sources that helped

- **Mainline IPQ5018 PCI enablement** (the driver + DT our tree runs), George
  Moussalem: [PATCH v8, linux-phy, 2025-04](https://lists.infradead.org/pipermail/linux-phy/2025-April/021770.html)
  and the earlier [v7 series](https://lkml.iu.edu/hypermail/linux/kernel/2503.3/03501.html).
  The IPQ5018 PHY init table is a single sequence shared by both the 1-lane and
  2-lane PHYs.
- **The combo PCIe/USB3 PHY conversion** (proves `0x7e000`/`0x5d000` are one combo
  PHY, and where the TCSR mux is flipped): George Moussalem, "phy: qualcomm:
  qcom-uniphy-pcie-28lp: Convert to PCIe/USB3 combo PHY driver" (patches 0151-0155 in
  kuncy7's tree below), and the "Enable USB3 for Qualcomm IPQ5018" series on
  [Ratatoskr / linux-phy](https://ratatoskr.run/linux-phy/2026/08/17458836/t).
- **The twin board, Cudy P5** (IPQ5018 + RM/RG520N modem on `pcie1`/MHI, mainline
  kernel 6.18, non-QSDK) — the reference that uses `perst-gpios ... GPIO_ACTIVE_LOW`
  and documents the ~100 s cold-modem endpoint delay + GPIO35 power-cycle:
  `kuncy7/openwrt-nss-edma` branch `c3po-tag-8021q`,
  [`ipq5018-cudy-p5.dts`](https://github.com/kuncy7/openwrt-nss-edma) and PRs
  [#7](https://github.com/kuncy7/openwrt-nss-edma/pull/7) /
  [#10](https://github.com/kuncy7/openwrt-nss-edma/pull/10). OpenWrt forum:
  ["IPQ5018: Adding Cudy P5 support"](https://forum.openwrt.org/t/ipq5018-adding-cudy-p5-support/252655).
- **XUNISON/ZEARTS D50** — reference only, **not** a `pcie1` comparison: its shipped
  support (georgemoussalem/openwrt `ipq50xx-xunison-d50`,
  `ipq5018-exigo-hub-d50-5g.dts`) runs the **modem over USB** and uses `pcie0` only
  for Wi-Fi. OpenWrt forum:
  ["Adding support for XUNISON Exigo D50 5G / ZEARTS D50"](https://forum.openwrt.org/t/adding-support-for-xunison-exigo-d50-5g-zearts-d50-5g-ipq5018/247016).
- **Quectel RG520N Hardware Design** (PCIE_REFCLK, `Ton2 ≥ 100 µs` before PERST#,
  PERST/CLKREQ pin behaviour). This is the **RG520N** series (our modem is
  RG520N-EB), *not* RM520N:
  [RG520N-AT Hardware Design (FCC)](https://fcc.report/FCC-ID/XMR2023RG520NAT/6628054.pdf),
  [RG520F&RG520N Series Hardware Design (FCC)](https://fcc.report/FCC-ID/XMR2022RG520NNA/6185642.pdf).

See `README.md` (board reference), `bootloop-postmortem.md` (the watchdog story).
