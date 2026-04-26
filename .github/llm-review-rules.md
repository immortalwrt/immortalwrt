# LLM review rules — openwrt/openwrt

Project-specific patterns to flag, even when other in-tree files still
use the deprecated form. The LLM review routine reads this at session
start.

## Deprecated device-tree patterns

- **LED label syntax.** `label = "green:status";` --> use
  `color = <LED_COLOR_ID_GREEN>;` + `function = LED_FUNCTION_STATUS;`
  (constants from `include/dt-bindings/leds/common.h`; pick the
  `LED_COLOR_ID_<COLOR>` and `LED_FUNCTION_<FUNC>` matching the old
  `<color>:<function>` string, falling back to `function = "<func>";`
  if no matching `LED_FUNCTION_*` constant exists).
- **MAC from MTD.** `mediatek,mtd-eeprom = <&factory 0xNNNN>;` --> use
  `nvmem-cells = <&macaddr_factory_NN>;` +
  `nvmem-cell-names = "mac-address";`.
