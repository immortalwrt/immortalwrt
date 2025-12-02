PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

RAMFS_COPY_BIN='fw_printenv fw_setenv head'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'

remove_oem_ubi_volume() {
	local oem_volume_name="$1"
	local oem_ubivol
	local mtdnum
	local ubidev

	mtdnum=$(find_mtd_index "$CI_UBIPART")
	if [ ! "$mtdnum" ]; then
		return
	fi

	ubidev=$(nand_find_ubi "$CI_UBIPART")
	if [ ! "$ubidev" ]; then
		ubiattach --mtdn="$mtdnum"
		ubidev=$(nand_find_ubi "$CI_UBIPART")
	fi

	if [ "$ubidev" ]; then
		oem_ubivol=$(nand_find_volume "$ubidev" "$oem_volume_name")
		[ "$oem_ubivol" ] && ubirmvol "/dev/$ubidev" --name="$oem_volume_name"
	fi
}

platform_check_image() {
	return 0;
}

platform_pre_upgrade() {
	case "$(board_name)" in
	asus,rt-ax89x)
		asus_initial_setup
		;;
	redmi,ax6|\
	xiaomi,ax3600|\
	xiaomi,ax9000)
		xiaomi_initramfs_prepare
		;;
	esac
}

platform_do_upgrade() {
	case "$(board_name)" in
	aliyun,ap8220|\
	zte,mf269-stock)
		CI_UBIPART="rootfs"
		nand_do_upgrade "$1"
		;;
	arcadyan,aw1000|\
	cmcc,rm2-6|\
	compex,wpq873|\
	dynalink,dl-wrx36|\
	edimax,cax1800|\
	netgear,rax120v2|\
	netgear,sxr80|\
	netgear,sxs80|\
	netgear,wax218|\
	netgear,wax620|\
	netgear,wax630|\
	zyxel,nwa210ax)
		nand_do_upgrade "$1"
		;;
	asus,rt-ax89x)
		CI_UBIPART="UBI_DEV"
		CI_KERNPART="linux"
		CI_ROOTPART="jffs2"
		nand_do_upgrade "$1"
		;;
	buffalo,wxr-5950ax12)
		CI_KERN_UBIPART="rootfs"
		CI_ROOT_UBIPART="user_property"
		buffalo_upgrade_prepare
		nand_do_flash_file "$1" || nand_do_upgrade_failed
		nand_do_restore_config || nand_do_upgrade_failed
		buffalo_upgrade_optvol
		;;
	edgecore,eap102)
		active="$(fw_printenv -n active)"
		if [ "$active" -eq "1" ]; then
			CI_UBIPART="rootfs2"
		else
			CI_UBIPART="rootfs1"
		fi
		# force altbootcmd which handles partition change in u-boot
		fw_setenv bootcount 3
		fw_setenv upgrade_available 1
		nand_do_upgrade "$1"
		;;
	linksys,homewrk)
		CI_UBIPART="rootfs"
		remove_oem_ubi_volume ubi_rootfs
		nand_do_upgrade "$1"
		;;
	linksys,mx4200v1|\
	linksys,mx4200v2|\
	linksys,mx4300)
		linksys_mx_pre_upgrade "$1"
		remove_oem_ubi_volume squashfs
		nand_do_upgrade "$1"
		;;
	linksys,mx5300|\
	linksys,mx8500)
		linksys_mx_pre_upgrade "$1"
		remove_oem_ubi_volume ubifs
		nand_do_upgrade "$1"
		;;
	prpl,haze|\
	qnap,301w)
		CI_KERNPART="0:HLOS"
		CI_ROOTPART="rootfs"
		emmc_do_upgrade "$1"
		;;
	redmi,ax6|\
	xiaomi,ax3600|\
	xiaomi,ax9000)
		# Make sure that UART is enabled
		fw_setenv boot_wait on
		fw_setenv uart_en 1

		# Enforce single partition.
		fw_setenv flag_boot_rootfs 0
		fw_setenv flag_last_success 0
		fw_setenv flag_boot_success 1
		fw_setenv flag_try_sys1_failed 8
		fw_setenv flag_try_sys2_failed 8

		# Kernel and rootfs are placed in 2 different UBI
		CI_KERN_UBIPART="ubi_kernel"
		CI_ROOT_UBIPART="rootfs"
		nand_do_upgrade "$1"
		;;
	redmi,ax6-stock|\
	xiaomi,ax3600-stock|\
	xiaomi,ax9000-stock)
		part_num="$(fw_printenv -n flag_boot_rootfs)"
		if [ "$part_num" -eq "1" ]; then
			CI_UBIPART="rootfs_1"
			target_num=1
			# Reset fail flag for the current partition
			# With both partition set to fail, the partition 2 (bit 1)
			# is loaded
			fw_setenv flag_try_sys2_failed 0
		else
			CI_UBIPART="rootfs"
			target_num=0
			# Reset fail flag for the current partition
			# or uboot will skip the loading of this partition
			fw_setenv flag_try_sys1_failed 0
		fi

		# Tell uboot to switch partition
		fw_setenv flag_boot_rootfs "$target_num"
		fw_setenv flag_last_success "$target_num"

		# Reset success flag
		fw_setenv flag_boot_success 0

		nand_do_upgrade "$1"
		;;
	spectrum,sax1v1k)
		CI_KERNPART="0:HLOS"
		CI_ROOTPART="rootfs"
		CI_DATAPART="rootfs_data"
		emmc_do_upgrade "$1"
		;;
	tplink,deco-x80-5g|\
	tplink,eap620hd-v1|\
	tplink,eap660hd-v1)
		remove_oem_ubi_volume ubi_rootfs
		tplink_do_upgrade "$1"
		;;
	yuncore,ax880)
		active="$(fw_printenv -n active)"
		if [ "$active" -eq "1" ]; then
			CI_UBIPART="rootfs_1"
		else
			CI_UBIPART="rootfs"
		fi
		# force altbootcmd which handles partition change in u-boot
		fw_setenv bootcount 3
		fw_setenv upgrade_available 1
		nand_do_upgrade "$1"
		;;
	zbtlink,zbt-z800ax)
		local mtdnum="$(find_mtd_index 0:bootconfig)"
		local alt_mtdnum="$(find_mtd_index 0:bootconfig1)"
		part_num="$(hexdump -e '1/1 "%01x|"' -n 1 -s 168 -C /dev/mtd$mtdnum | cut -f 1 -d "|" | head -n1)"
		# vendor firmware may swap the rootfs partition location, u-boot append: ubi.mtd=rootfs
		# since we use fixed-partitions, need to force boot from the first rootfs partition
		if [ "$part_num" -eq "1" ]; then
			mtd erase /dev/mtd$mtdnum
			mtd erase /dev/mtd$alt_mtdnum
		fi
		nand_do_upgrade "$1"
		;;
	zte,mf269)
		CI_KERN_UBIPART="ubi_kernel"
		CI_ROOT_UBIPART="rootfs"
		nand_do_upgrade "$1"
		;;
	zyxel,nbg7815)
		local config_mtdnum="$(find_mtd_index 0:bootconfig)"
		[ -z "$config_mtdnum" ] && reboot
		part_num="$(hexdump -e '1/1 "%01x|"' -n 1 -s 168 -C /dev/mtd$config_mtdnum | cut -f 1 -d "|" | head -n1)"
		if [ "$part_num" -eq "0" ]; then
			CI_KERNPART="0:HLOS"
			CI_ROOTPART="rootfs"
		else
			CI_KERNPART="0:HLOS_1"
			CI_ROOTPART="rootfs_1"
		fi
		emmc_do_upgrade "$1"
		;;
	verizon,cr1000a)
		CI_KERNPART="0:HLOS"
		CI_ROOTPART="rootfs"
		rootpart=$(find_mmc_part "$CI_ROOTPART")
		mmcblk_hlos=$(find_mmc_part "$CI_KERNPART" | sed -e "s/^\/dev\///")
		hlos_start=$(cat /sys/class/block/$mmcblk_hlos/start)
		hlos_size=$(cat /sys/class/block/$mmcblk_hlos/size)
		hlos_start_hex=$(printf "%X\n" "$hlos_start")
		hlos_size_hex=$(printf "%X\n" "$hlos_size")
		fw_setenv set_custom_bootargs "setenv bootargs console=ttyMSM0,115200n8 root=$rootpart rootwait fstools_ignore_partname=1"
		fw_setenv read_hlos_emmc "mmc read 44000000 0x$hlos_start_hex 0x$hlos_size_hex"
		fw_setenv setup_and_boot "run set_custom_bootargs;run read_hlos_emmc; bootm 44000000"
		fw_setenv bootcmd "run setup_and_boot"
		emmc_do_upgrade "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}

platform_copy_config() {
	case "$(board_name)" in
	prpl,haze|\
	qnap,301w|\
	spectrum,sax1v1k|\
	zyxel,nbg7815|\
	verizon,cr1000a)
		emmc_copy_config
		;;
	esac
}
