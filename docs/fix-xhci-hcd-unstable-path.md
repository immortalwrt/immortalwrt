# Fix: USB WiFi Invalid Radio Entry After Reboot

## 问题描述

Photonicat2 Mini (RK3576) 路由器在某些启动中，USB WiFi 设备会在 `/etc/config/wireless` 中产生额外的无效 radio 条目（如 `radio2`），与已有的 `radio1` 指向同一块物理网卡，但 SSID/加密配置不同，导致 WiFi 配置混乱。

## 根因分析

### 现象

```
config wifi-device 'radio1'
    option path 'platform/soc/23000000.usb/xhci-hcd.4.auto/usb1/1-1/1-1.3'

config wifi-device 'radio2'   # 无效条目
    option path 'platform/soc/23000000.usb/xhci-hcd.0.auto/usb1/1-1/1-1.3'
```

同一块 USB WiFi 网卡，路径中的 `xhci-hcd.N.auto` 在不同启动之间值不同（`.0` vs `.4`）。

### 内核层根因

Linux 内核使用**全局** `platform_devid_ida` 计数器分配 `PLATFORM_DEVID_AUTO` 编号，所有设备共享：

```c
// drivers/base/platform.c
static DEFINE_IDA(platform_devid_ida);  // 全局，不按设备名称分组

// drivers/usb/dwc3/host.c
xhci = platform_device_alloc("xhci-hcd", PLATFORM_DEVID_AUTO);
// 分配到的编号取决于此前所有使用 PLATFORM_DEVID_AUTO 的设备数量
```

RK3576 系统中，在 `23000000.usb`（dwc3）probe 之前，以下 PMIC 子设备已各自消耗一个编号：

| 编号 | 设备 |
|------|------|
| `.0` | `rk805-pinctrl.0.auto` |
| `.1` | `rk808-regulator.1.auto` |
| `.2` | `rk805-pwrkey.2.auto` |
| `.3` | `arm-scmi.3.auto` |
| `.4` | `xhci-hcd.4.auto` ← USB WiFi |

PMIC 子设备（I²C 子设备）与 `23000000.usb` 存在 **probe 时序竞争**。某些启动中 `23000000.usb` 抢在 PMIC 子设备之前完成 probe，拿到 `.0.auto`；另一些启动 PMIC 先完成，`xhci-hcd` 拿到 `.4.auto`。

### 用户态触发机制

`wifi-detect.uc` 在每次 `wifi config` 时生成 `/etc/board.json`，以完整路径（含 `xhci-hcd.N.auto`）作为设备唯一标识。`mac80211.uc` 通过路径匹配判断是否为已知设备——路径不同则视为新设备，追加新 radio 条目。

## 修复方案

在用户态去除路径中不稳定的 `xhci-hcd.N.auto` 段，只保留稳定的 USB 物理拓扑路径（`usb<N>/...`）。

### 1. wifi-detect.uc

文件：`package/network/config/wifi-scripts/files/usr/share/hostap/wifi-detect.uc`

```diff
 function phy_path(name) {
     let devpath = realpath(`/sys/class/ieee80211/${name}/device`);

     devpath = replace(devpath, /^\/sys\/devices\//, "");
     if (match(devpath, /^platform\/.*\/pci/))
         devpath = replace(devpath, /^platform\//, "");
+    else if (match(devpath, /\/xhci-hcd\.[0-9]+\.auto\//))
+        devpath = replace(devpath, /^.*\/xhci-hcd\.[0-9]+\.auto\//, "");
```

结果：`platform/soc/23000000.usb/xhci-hcd.4.auto/usb1/1-1/1-1.3` → `usb1/1-1/1-1.3`

### 2. libiwinfo patch

文件：`package/network/utils/iwinfo/patches/202-fix-xhci-hcd-unstable-id.patch`

在 `iwinfo_nl80211.c` 的 `nl80211_phy_path_str()` 中同步去除，保证 `iwinfo` 路径生成与 `wifi-detect.uc` 一致（`mac80211.sh` 的 `find_phy()` 依赖 `iwinfo nl80211 phyname` 的路径匹配）：

```diff
+    /* For USB WiFi devices under xhci-hcd, the platform device instance
+     * number (xhci-hcd.N.auto) is dynamically assigned and can change
+     * across reboots depending on driver registration order. Strip this
+     * unstable component and keep only the stable USB topology path
+     * starting from usb<N>/... */
+    {
+        char *xhci = strstr(link, "/xhci-hcd.");
+        if (xhci) {
+            char *usb = strchr(xhci + 1, '/');
+            if (usb)
+                link = usb + 1;
+        }
+    }
+
     snprintf(buf + buf_len, sizeof(buf) - buf_len, "/ieee80211");
```

## 验证

修复后 `iwinfo nl80211 path phy0` 返回短路径：

```
usb1/1-1/1-1.3
```

UCI 中 radio1 路径更新为：

```
wireless.radio1.path='usb1/1-1/1-1.3'
```

重启后不再产生多余 radio 条目。

## 已知限制

- **多 xhci 控制器 + 相同 USB 拓扑**：若系统存在多个 xhci-hcd 控制器且 USB 设备拓扑相同（如 `usb1/1-1/1-1.3`），短路径会产生冲突。本方案适用于单 xhci-hcd 控制器的设备。
- **DTS 层面无法修复**：PMIC 子设备是 I²C 子设备，无法通过 DTS phandle 建立与 `23000000.usb` 的 deferred probe 依赖链。

## 设备信息

- 设备：Photonicat2 Mini
- SoC：RK3576
- USB WiFi：AIC8800（通过 USB Hub `usb5e3,610/620` 连接）
- PCIe WiFi：`22000000.pcie`
