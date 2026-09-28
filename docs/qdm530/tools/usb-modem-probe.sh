#!/bin/sh
# QDM530 USB modem data-path probe (READ-ONLY, safe).
# Run on the booted board over SSH. Reports whether the RG520N-EB's USB interface
# enumerates on the IPQ5018 host and as what (QMI / MBIM / serial), which decides
# whether WAN can come up over USB.

echo "===== 1. USB devices on the bus ====="
if command -v lsusb >/dev/null 2>&1; then lsusb; else
  for d in /sys/bus/usb/devices/*; do
    [ -f "$d/idVendor" ] || continue
    printf '%s  %s:%s  %s %s\n' "${d##*/}" \
      "$(cat "$d/idVendor")" "$(cat "$d/idProduct")" \
      "$(cat "$d/manufacturer" 2>/dev/null)" "$(cat "$d/product" 2>/dev/null)"
  done
fi
echo "  (Quectel = idVendor 2c7c)"

echo "===== 2. Modem control/data device nodes ====="
ls -l /dev/ttyUSB* /dev/cdc-wdm* /dev/wwan* 2>/dev/null || echo "  none of ttyUSB*/cdc-wdm*/wwan*"

echo "===== 3. Network interfaces from USB (qmi/mbim/ncm/rndis) ====="
ls -l /sys/class/net/ 2>/dev/null | grep -iE "wwan|usb|ncm|mbim" || echo "  no wwan*/usb* netdev"
for n in /sys/class/net/*; do
  drv=$(readlink "$n/device/driver" 2>/dev/null)
  [ -n "$drv" ] && echo "  ${n##*/} -> driver ${drv##*/}"
done

echo "===== 4. Relevant modules loaded ====="
lsmod 2>/dev/null | grep -iE "qmi_wwan|cdc_mbim|cdc_ncm|cdc_wdm|option|usb_wwan|usbserial|rndis|usbnet" || echo "  (none - may be auto-loaded on enumerate)"

echo "===== 5. dmesg: USB + modem enumeration ====="
dmesg 2>/dev/null | grep -iE "usb [0-9]|2c7c|quectel|qmi_wwan|cdc_mbim|option|new .* USB device|dwc3|xhci|wwan" | tail -40

echo "===== 6. ModemManager view (if running) ====="
command -v mmcli >/dev/null 2>&1 && mmcli -L 2>/dev/null || echo "  mmcli not present / MM not running"

echo "===== 7. AT port hint ====="
echo "  The AT port is usually the 3rd/4th /dev/ttyUSB* (ttyUSB2 on this modem)."
echo "  Confirm with: for p in /dev/ttyUSB*; do echo \"== \$p ==\"; done"
echo ""
echo "If a qmi_wwan/cdc-wdm node shows up, the modem is in QMI usbnet mode and WAN"
echo "can be dialed over USB. If only ttyUSB serial ports show, switch it to QMI"
echo "with AT+QCFG=\"usbnet\",0 first (a state-changing command - review before use)."
