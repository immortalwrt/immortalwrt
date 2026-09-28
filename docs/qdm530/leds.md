# QDM530 front-panel LEDs

The board has **four front-panel RGB LEDs** driven by an **Awinic AW9523B** 16-channel
I2C GPIO/LED expander (`0x5b` on `blsp1_i2c3` / `0x78b7000`). This note records how they
are wired, why they need a dedicated driver, the channel→LED→colour map found on the
hardware, and how each LED is driven.

## Why a custom driver (leds-aw9523), not gpio-leds

These LEDs light only in the AW9523B's **constant-current ("LED") mode** — plain GPIO
drive produces no visible change (confirmed on HW). Mainline has `pinctrl-aw9523` (GPIO
only) but **no LED-class driver** for the chip, so this port adds one:
`package/kernel/leds-aw9523/` (modelled on `leds-aw200xx`). Each of the chip's 16
constant-current channels can be exposed as a standard Linux LED class device, so the
LEDs show up under `/sys/class/leds` as `color:function` and work with the normal LED
triggers (`netdev`, `timer`, `default-on`, the boot/running/failsafe/upgrade aliases).

### Reset / I2C gotcha

`pinctrl-aw9523` **requires** a `reset-gpios` and pulses RSTN (tlmm 32) at probe. On this
board that pulse wedged the QUP I2C controller — `i2c_qup 78b7000.i2c: flush timed out` —
and the ~7 s stall even raced the NAND/UBI rootfs probe (`cannot open mtd rootfs` → hang).
Left at its power-on level the chip answers I2C fine. So **leds-aw9523 does not touch the
RSTN GPIO at all** — it only soft-resets over I2C (SOFT_RESET register) — and the DT node
carries no `reset-gpios`. That one decision is what makes the probe reliable.

## Channel → LED → colour map

Each DT child `reg` is a **DIM channel index** (the DIM register is `0x20 + reg`). The map
was found on HW by writing `0xff` to each DIM register (`0x20`–`0x2f`) in turn and watching
the panel. LEDs are numbered from the power button inward.

| LED | function | red | green | blue |
|-----|----------|-----|-------|------|
| #1  | `power`  | reg 0 (0x20) | reg 1 (0x21) | reg 2 (0x22) |
| #2  | `wan`    | reg 7 (0x27) | reg 15 (0x2f) | reg 8 (0x28) |
| #3  | `mobile` | reg 3 (0x23) *(dim)* | reg 10 (0x2a) | reg 11 (0x2b) |
| #4  | `status` | — | reg 13 (0x2d) | reg 14 (0x2e) |

Notes:
- #1–#3 are full RGB; **#4 has no red channel** (green + blue only).
- LED3's red (reg 3) is noticeably dimmer than the other channels (a board resistor
  choice), so "medium signal" leans orange rather than pure amber.
- DIM channels `4,5,6,9,12` (regs `0x24,0x25,0x26,0x29,0x2c`) drive nothing — unused pins.

## Function, colour and behaviour

| LED | meaning | driven by | states |
|-----|---------|-----------|--------|
| **#1 power** | system power / health | DT `led-*` aliases | green solid = running; green blink = boot; red = failsafe; red blink = upgrade |
| **#2 wan** | internet / data link | `netdev` trigger on `wwand0` | green = link up, blinks on tx/rx |
| **#3 mobile** | cellular signal strength | `qdm530-modem-leds` daemon (RSRP) | green ≥ −95 dBm; orange −95…−110; red < −110; off = no reading |
| **#4 status** | data-connection state | `qdm530-modem-leds` daemon | green = wan CONNECTED; blue = registered / connecting; off = no service |

The power and wan LEDs use standard kernel mechanisms (the boot/running aliases, and a
`netdev` trigger set from `board.d/01_leds`). The mobile (signal) and status LEDs have no
kernel trigger that fits cellular state, so a light poller — `/usr/sbin/qdm530-modem-leds`,
a procd service — reads wwand over ubus every 10 s (`ubus call wwand status` for the
connection state, `ubus call wwand modem_signal` for RSRP) and sets the LED brightnesses.
It picks the NR5G RSRP when the modem is on 5G, otherwise LTE.

## Files

- `package/kernel/leds-aw9523/` — the LED-class driver (`kmod-leds-aw9523`).
- `target/linux/qualcommax/dts/ipq5018-qdm530.dts` — the `aw9523` `led-controller@5b`
  node (compatible `awinic,aw9523-led`) with the eleven `led@N` children, and the
  `led-boot/-failsafe/-running/-upgrade` aliases.
- `.../base-files/etc/board.d/01_leds` — the wan `netdev` trigger (on `wwand0`).
- `.../base-files/usr/sbin/qdm530-modem-leds` — the signal/status poller.
- `.../base-files/etc/init.d/qdm530-modem-leds` — procd service for the poller.
- `.../base-files/etc/uci-defaults/28_enable-modem-leds` — enables the service on this board.

## Manual control

The LEDs are ordinary class devices, e.g.:

```sh
echo 255 > /sys/class/leds/green:power/brightness     # on
echo 0   > /sys/class/leds/green:power/brightness     # off
# a colour on LED3 (mobile) = mix its three channels:
echo 255 > /sys/class/leds/red:mobile/brightness
echo 110 > /sys/class/leds/green:mobile/brightness    # -> orange
```
Stop the poller first (`/etc/init.d/qdm530-modem-leds stop`) if you want to hold the
mobile/status LEDs at a manual value.
