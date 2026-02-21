REQUIRE_IMAGE_METADATA=1

platform_do_upgrade() {
    local board=$(board_name)
    
    case "$board" in
    huawei,ec6108v9c)
        echo "Upgrading kernel partition..."
        get_image "$1" | dd bs=1M count=16 of=/dev/mmcblk0p3 || {
            echo "Kernel upgrade failed!"
            return 1
        }
        
        echo "Upgrading rootfs partition..."
        get_image "$1" | dd bs=1M skip=16 of=/dev/mmcblk0p4 || {
            echo "Rootfs upgrade failed!"
            return 1
        }
        
        sync
        echo "Upgrade completed successfully!"
        ;;
    *)
        default_do_upgrade "$1"
        ;;
    esac
}
```

### 工作原理
```
sysupgrade.bin 结构：
┌────────────────┬──────────────┬──────────┐
│ kernel (16MB)  │   rootfs     │ metadata │
└────────────────┴──────────────┴──────────┘

写入到 /dev/mmcblk0p3 时：
                  ┌───────── 从这里开始写入
                  ↓
┌──────────┬──────────┬────────────┬──────────────┐
│ fastboot │ bootargs │   kernel   │    rootfs    │
│   1MB    │   1MB    │    16MB    │    512MB     │
│  0-1MB   │  1-2MB   │  2MB-18MB  │  18MB-530MB  │
└──────────┴──────────┴────────────┴──────────────┘
                      │←── 16MB ──→│←─ rootfs ──→│
                      写入覆盖这两个分区