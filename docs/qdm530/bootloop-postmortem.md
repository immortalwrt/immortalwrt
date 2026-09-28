# QDM530 boot-loop post-mortem — why it took so long

**Symptom:** every ImmortalWrt boot on the QDM530 (IPQ5018 Quectel RG520 CPE) reset
at ~1.5 s, in a loop. SBL reported `GCC [RstStat:0x10, RstDbg:0x500100] WDog Stat : 0x4`
on each reset — i.e. a **watchdog** reset.

**Actual root cause (found 2026-09-19):** an **external hardware watchdog fed by
toggling TLMM GPIO 26**. Nothing in our device tree fed it, so it bit at ~1.5 s.
The sibling in-tree boards `ipq5018-ax830/ax850` carry the same kind of watchdog on
GPIO **27** — one pin off. Fix: a `gpio-watchdog` (`linux,wdt-gpio`) node on
`<&tlmm 26>`, `hw_algo="toggle"`, `hw_margin_ms=500`, `always-running`, plus
`CONFIG_GPIO_WATCHDOG_ARCH_INITCALL=y` so it is fed at arch_initcall (~0.05 s).

## Why we went in circles

The reset *looked exactly like* the well-documented IPQ5018 SoC-watchdog / PCIe-timing
boot loop, so we kept "fixing" that — the wrong target — for a long time:

1. **"It's the APSS watchdog (b017000)."** We made `qcom_wdt` take it over
   unconditionally at `subsys_initcall` and ping it. No effect. (Later *proven* no
   effect: we fed it from ~0.08 s with a confirming print, still reset.)
2. **"It's a secure/TZ watchdog."** Tried the SCM `SEC_WDOG_DIS` (cmd 0x7) disable.
   No effect. We then wrote it off as an unkillable TZ watchdog — wrong conclusion.
3. **"It's the PCIe link-up timeout eating the boot budget."** The vendor U-Boot
   force-injects the PCIe nodes; `pcie1` (modem) never trains (~1 s "Phy link never
   came up"). We deleted the PCIe nodes, disabled USB/wcss/NAND, tried async probe
   and a shortened PCIE_LINK_WAIT — all to "reach userspace before the bite."
4. **"It's the CMN PLL clock-gating bug."** Applied the known OpenWrt patch. Not our
   board's problem.
5. **Console self-sabotage.** Our own diagnostics (`initcall_debug`,
   `ignore_loglevel`) flooded the 115200 console so hard that the kernel only reached
   0.036 s of work by the 1.5 s wall — which made us *misread* the reset as purely
   time-based and chase boot-speed. Removing `initcall_debug` alone cut a "1.04 s
   msm_serial probe" (really: the printk backlog flush) down to ~0.11 s.

Every one of these is a real IPQ5018 topic with forum/PR write-ups, which is exactly
why they were seductive. None was the cause. The common mistake: **treating a
board-specific external component as the generic SoC issue the internet describes.**

## What actually cracked it

- The reset survived feeding *both* SoC watchdogs early (APSS + a GPIO guess),
  proving neither was it.
- The vendor U-Boot keeps the board alive at its own prompt for as long as you like →
  **U-Boot must be petting the watchdog.** So we watched *what U-Boot pets*:
  - `i2c md 0x5b …` on the AW9523 expander across samples → outputs static → not it.
  - Dumped **all 47 TLMM `GPIO_IN_OUT` registers** (base `0x01000000`, stride
    `0x1000`, reg `+0x4`, bit1 = output) from the U-Boot prompt across several
    samples. **Only GPIO 26 alternated `0x0 ↔ 0x3`.** That is the kick line.
- Pointed the `gpio-watchdog` node at tlmm 26 → boot sailed past 1.5 s to userspace.

**Lesson:** when a reset looks like a documented SoC problem but every documented fix
fails, stop trusting the diagnosis and *observe the working reference* (here, the
vendor bootloader) directly. A 20-minute register dump would have saved weeks.

## After the watchdog: one more blocker

`tsens` (thermal) probe was deferred until ~9.4 s — after `Freeing unused kernel
memory` freed the `__init` sections — then called the `__init` function
`init_tsens_v1_no_rpm()` and faulted (IABT). Disabled `&tsens` for now.

## References we consulted (and how each panned out)

- OpenWrt PR #24653 — IPQ CMN PLL clock-gating fix.
  https://github.com/openwrt/openwrt/pull/24653 — applied, **not** our cause.
  (Cited from our own notes; verify the exact number before relying on it.)
- In-tree sibling DTS `target/linux/qualcommax/dts/ipq5018-ax830.dts` and
  `ipq5018-ax850.dts` — **the genuinely useful lead**: they carry a
  `gpio-watchdog`/`linux,wdt-gpio` node (on tlmm 27), which is what told us an
  *external GPIO* watchdog is a thing on these boards. The tree, not a forum.
- Linux `drivers/watchdog/gpio_wdt.c` and `CONFIG_GPIO_WATCHDOG_ARCH_INITCALL` —
  for feeding the WDT early in boot.

## Net fix set (in `ipq5018-qdm530.dts` + `config-6.18`)

- `gpio-watchdog` on `<&tlmm 26>` (the fix).
- `CONFIG_GPIO_WATCHDOG_ARCH_INITCALL=y`.
- `&tsens` disabled (deferred `__init`-after-free crash).
- NAND kept enabled; the factory image flashed to both rootfs slots → persistent NAND
  boot.

## Status after the post-mortem

The recovery-time trims (PCIe/WiFi/modem/USB deletions) and the temporary diagnostic
`dev_err` prints in `gpio_wdt`/`qcom-wdt` were **removed** once the board booted
reliably; the tree now builds a full-feature image. WiFi (2.4 GHz working, 5 GHz
radio up), both ethernet ports (1 G + 2.5 G) and the modem over USB all come up. The
one still-open hardware bring-up item is the modem's PCIe link (`pcie1`) — see
`README.md` ("Modem" and "Open issues").
