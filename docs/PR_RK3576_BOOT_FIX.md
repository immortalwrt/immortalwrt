# PR: uboot-rockchip: Add RK3576 support to HANG_TO_BROM patch

## Summary
Add RK3576 support to the `101-rockchip-Reset-to-bootrom-download-mode-on-hang.patch` to fix boot failure after direct flashing on RK3576 devices.

## Problem
RK3576 devices (e.g., Photonicat 2) fail to boot after direct flashing with rkdeveloptool, while web upgrade from a working firmware works fine.

The root cause is that the TF-A blob for RK3576 does not clear `PMU0_GRF_SOC_CON1` during PSCI reset, causing `BOOT_MODE_REG` to lose its value after reset.

## Changes
1. **boot_mode.c**: Add RK3576 SOC_CON1 clear operation before reset
   - Register address: `0x26024004` (PMU0_GRF_BASE + 0x04)
   - Write value: `0xFFFF0000` (same as RK3588)

2. **generic-rk3576_defconfig**: Enable `CONFIG_ROCKCHIP_HANG_TO_BROM=y`

## Testing
- Device: Ariaboard Photonicat 2 (RK3576)
- Firmware: ImmortalWrt r37561
- Test method: Direct flash via rkdeveloptool
- Result: Boot successful, all peripherals working (dual Ethernet, WiFi, USB, 5G modem)

## Technical Details
| SoC | PMU0_GRF_SOC_CON1 Address | Write Value |
|-----|---------------------------|-------------|
| RK3568 | 0xFDC20104 | 0x40000 |
| RK3576 | 0x26024004 | 0xFFFF0000 |
| RK3588 | 0xFD58A004 | 0xFFFF0000 |

The RK3576 register address was derived from ARM TF-A upstream source code:
- `PMU0_GRF_BASE`: 0x26024000
- `PMU0GRF_SOC_CON(1)`: offset 0x04

Signed-off-by: Your Name <your.email@example.com>
