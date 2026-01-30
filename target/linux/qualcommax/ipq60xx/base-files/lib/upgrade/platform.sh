PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

RAMFS_COPY_BIN='fw_printenv fw_setenv head seq'
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

platform_do_upgrade() {
	case "$(board_name)" in
	alfa-network,ap120c-ax)
		CI_UBIPART="rootfs_1"
		alfa_bootconfig_rootfs_rotate "0:BOOTCONFIG" "148"
		nand_do_upgrade "$1"
		;;
	cambiumnetworks,xe3-4)
		fw_setenv bootcount 0
		nand_do_upgrade "$1"
		;;
	anysafe,e1)
		CI_UBIPART="rootfs"
		nand_do_upgrade "$1"
		;;
	cmiot,ax18|\
	redmi,ax5|\
	xiaomi,ax1800|\
	zn,m2|\
	glinet,gl-ax1800|\
	glinet,gl-axt1800|\
	netgear,wax214|\
	qihoo,360v6)
		nand_do_upgrade "$1"
		;;
	netgear,wax610|\
	netgear,wax610y)
		remove_oem_ubi_volume wifi_fw
		remove_oem_ubi_volume ubi_rootfs
		nand_do_upgrade "$1"
		;;
	linksys,mr7350|\
	linksys,mr7500)
		boot_part="$(fw_printenv -n boot_part)"
		if [ "$boot_part" -eq "1" ]; then
			fw_setenv boot_part 2
			CI_KERNPART="alt_kernel"
			CI_UBIPART="alt_rootfs"
		else
			fw_setenv boot_part 1
			CI_UBIPART="rootfs"
		fi
		fw_setenv boot_part_ready 3
		fw_setenv auto_recovery yes
		nand_do_upgrade "$1"
		;;
	tplink,eap610od|\
	tplink,eap620hd-v3|\
	tplink,eap623od-hd-v1|\
	tplink,eap625od-hd-v1)
		remove_oem_ubi_volume ubi_rootfs
		tplink_do_upgrade "$1"
		;;
	yuncore,fap650)
		[ "$(fw_printenv -n owrt_env_ver 2>/dev/null)" != "7" ] && yuncore_fap650_env_setup
		local active="$(fw_printenv -n owrt_slotactive 2>/dev/null)"
		if [ "$active" = "1" ]; then
			CI_UBIPART="rootfs"
		else
			CI_UBIPART="rootfs_1"
		fi
		fw_setenv owrt_bootcount 0
		fw_setenv owrt_slotactive $((1 - active))
		nand_do_upgrade "$1"
		;;
	jdcloud,re-cs-02|\
	jdcloud,re-cs-07|\
	jdcloud,re-ss-01|\
	link,nn6000-v1|\
	link,nn6000-v2|\
	philips,ly1800|\
	redmi,ax5-jdcloud|\
	sy,y6010)
		CI_KERNPART="0:HLOS"
		CI_ROOTPART="rootfs"
		emmc_do_upgrade "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}

platform_copy_config() {
	case "$(board_name)" in
	jdcloud,re-ss-01|\
	jdcloud,re-cs-02|\
	jdcloud,re-cs-07|\
	link,nn6000-v1|\
	link,nn6000-v2|\
	redmi,ax5-jdcloud)
		emmc_copy_config
		;;
	esac
}
