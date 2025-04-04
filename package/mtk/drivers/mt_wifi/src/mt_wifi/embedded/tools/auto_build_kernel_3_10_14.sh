if [ "${1}" == "AP" ]; then
    wifi_mode=AP
elif [ "${1}" == "STA" ]; then
    wifi_mode=STA
else
    echo "Wifi-Prebuild: wifi mode is not specified, default wifi mode is AP"
    wifi_mode=AP
fi

if [ "${2}" == "mt7663" -o "${2}" == "MT7663" ]; then
    wifi_chipset=mt7663
elif [ "${2}" == "mt7615" -o "${2}" == "MT7615" ]; then
    wifi_chipset=mt7615
elif [ "${2}" == "mt7626" -o "${2}" == "MT7626" ]; then
    wifi_chipset=mt7626
elif [ "${2}" == "mt7603" -o "${2}" == "MT7603" ]; then
    wifi_chipset=mt7603
elif [ "${2}" == "mt7628" -o "${2}" == "MT7628" ]; then
    wifi_chipset=mt7628
elif [ "${2}" == "mt76x2" -o "${2}" == "MT76X2" ]; then
    wifi_chipset=mt76x2
elif [ "${2}" == "mt7620" -o "${2}" == "MT7620" ]; then
    wifi_chipset=mt7620
elif [ "${2}" == "mt7610" -o "${2}" == "MT7610" ]; then
    wifi_chipset=mt7610
else
    echo "Wifi-Prebuild: wifi chipset is not specified, default wifi chipset is mt7663"
    wifi_chipset=mt7663
fi

echo "Wifi-Prebuild: wifi mode is $wifi_mode, wifi_chipset is $wifi_chipset"

if [ $wifi_chipset = "mt7663" -o $wifi_chipset = "mt7615" -o $wifi_chipset = "mt7626" ]; then
    if [ -d wifi_driver ]; then
        echo "Wifi-Prebuild: kernel-3.10.14.x wifi driver pre-build"
        mkdir -p mt_wifi_ap
        mkdir -p mt_wifi_sta
        cp -a  wifi_driver/os/linux/Kconfig.mt_wifi_ap ./mt_wifi_ap/Kconfig
        cp -a  wifi_driver/os/linux/Makefile.mt_wifi_ap ./mt_wifi_ap/Makefile
        cp -a  wifi_driver/os/linux/Kconfig.mt_wifi_sta ./mt_wifi_sta/Kconfig
        cp -a  wifi_driver/os/linux/Makefile.mt_wifi_sta ./mt_wifi_sta/Makefile
        cp -a wifi_driver/os/linux/Kconfig.mt_wifi wifi_driver/embedded/Kconfig
        if [ -d mt_wifi ]; then
            rm -rf mt_wifi
        fi
        mv wifi_driver mt_wifi
        echo "$wifi_chipset mt_wifi autobuild"
        RT28xx_DIR=./mt_wifi
        CHIPSET=$wifi_chipset
        RT28xx_MODE=$wifi_mode
        HAS_WOW_SUPPORT=n
        HAS_FPGA_MODE=n
        HAS_RX_CUT_THROUGH=n
        RT28xx_BIN_DIR=.
        export RT28xx_DIR CHIPSET RT28xx_MODE HAS_WOW_SUPPORT HAS_FPGA_MODE HAS_RX_CUT_THROUGH RT28xx_BIN_DIR
        make -C mt_wifi/embedded build_tools
        ./mt_wifi/embedded/tools/bin2h
    if [ "${CHIPSET}" == "mt7615" ]; then
        make -C mt_wifi/embedded build_sku_tables
        ./mt_wifi/txpwr/dat2h
    fi
    else
        exit 1
    fi
elif [ $wifi_chipset = "mt7603" ]; then
    dir_main="mt7603_wifi"
    dir_ap="mt7603_wifi_ap"

    if [ -d mt7603e ]; then
        echo Preparing for MT7603 wifi driver...
        chmod 755 -R ./mt7603e/*
        if [ -d $dir_ap ]; then
            rm -rf $dir_ap
        fi
        if [ -d $dir_main ]; then
            rm -rf $dir_main
        fi
        cp -a  mt7603e/rlt_wifi/rlt_wifi_ap ./$dir_ap
        mv mt7603e/rlt_wifi $dir_main
        rm -rf mt7603e/

        echo mt7603 wifi driver is ready!
    else
        echo rlt_wifi is not existing!
        exit 1
    fi
elif [ $wifi_chipset = "mt7628" ]; then
    dir_main="mt7628_wifi"
    dir_ap="mt7628_wifi_ap"

    if [ -d mt7628e ]; then
        echo Preparing for mt7628 wifi driver...
        chmod 755 -R ./mt7628e/*
        if [ -d $dir_ap ]; then
            rm -rf $dir_ap
        fi
        if [ -d $dir_main ]; then
            rm -rf $dir_main
        fi
        cp -rf mt7628e/wifi_driver/embedded/mt_wifi_ap ./$dir_ap
        mv mt7628e/wifi_driver $dir_main
        rm -rf mt7628e/
        echo mt7628 wifi driver is ready!
    else
        exit 1
    fi
elif [ $wifi_chipset = "mt76x2" ]; then
    dir_main="mt76x2_wifi"
    dir_ap="mt76x2_wifi_ap"

    if [ -d mt76x2e ]; then
        echo Preparing for MT76x2 wifi driver...
        chmod 755 -R ./mt76x2e/*
        if [ -d $dir_ap ]; then
            rm -rf $dir_ap
        fi
        if [ -d $dir_main ]; then
            rm -rf $dir_main
        fi
        mkdir $dir_ap
        cp mt76x2e/rlt_wifi/os/linux/Kconfig.rlt_wifi_ap $dir_ap/Kconfig
        cp mt76x2e/rlt_wifi/os/linux/Makefile.rlt_wifi_ap $dir_ap/Makefile
        mv mt76x2e/rlt_wifi $dir_main
        rm -rf mt76x2e
        echo  mt76x2 driver code is ready!
    else
        echo mt7612 driver is not existing!
        exit 1
    fi
elif [ $wifi_chipset = "mt7620" ]; then
    dir_main="mt7620_wifi"
    dir_ap="mt7620_wifi_ap"

    if [ -d mt7620e ]; then
        echo preparing for mt7620 wifi driver...
        chmod 755 -R ./mt7620e/*
        if [ -d $dir_ap ]; then
            rm -rf $dir_ap
        fi
        if [ -d $dir_main ]; then
            rm -rf $dir_main
        fi
        cp -rf mt7620e/WIFI_MT7620/rt2860v2_ap $dir_ap
        mv mt7620e/WIFI_MT7620 $dir_main
        rm -rf mt7620e
        echo mt7620 wifi driver is ready!
    else
        echo WIFI_MT7620 is not existing!
        exit 1
    fi
elif [ $wifi_chipset = "mt7610" ]; then
    dir_main="mt7610_wifi"

    if [ -d mt7610e ]; then
        echo Preparing for MT7610 wifi driver...
        chmod 755 -R ./mt7610e/*
        if [ -d $dir_main ]; then
            rm -rf $dir_main
        fi

        mv mt7610e/rlt_wifi $dir_main
        rm -rf mt7610e
        echo mt7610 driver code is ready!
    else
        echo mt7610 driver is not existing!
        exit 1
    fi
else
    echo "wifi_chipset is invalid: $wifi_chipset"
fi
