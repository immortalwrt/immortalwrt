RAMFS_COPY_BIN='mkfs.f2fs blkid blockdev fw_printenv fw_setenv'
RAMFS_COPY_DATA="/etc/fw_env.config /var/lock/fw_printenv.lock"

nand_remove_ubiblock() {
	local ubivol="$1"

	local ubiblk="ubiblock${ubivol:3}"
	if [ -e "/dev/$ubiblk" ]; then
		umount "/dev/$ubiblk" && echo "unmounted /dev/$ubiblk" || :
		if ! ubiblock -r "/dev/$ubivol"; then
			echo "cannot remove $ubiblk"
			return 1
		fi
	fi
}

xiaomi_mt7981_nand_upgrade_tar()
{
	CI_UBIPART=ubi
	local tar_file="$1"
	local board_dir="$(tar tf "$tar_file" | grep -m 1 '^sysupgrade-.*/$')"
	board_dir="${board_dir%/}"

	local kernel_length=$( (tar xf "$tar_file" "$board_dir/kernel" -O | wc -c) 2> /dev/null)
	[ "$kernel_length" = 0 ] && kernel_length=

	local rootfs_length=$( (tar xf "$tar_file" "$board_dir/root" -O | wc -c) 2> /dev/null)
	[ "$rootfs_length" = 0 ] && rootfs_length=

	local rootfs_type
	[ "$rootfs_length" ] && rootfs_type="$(identify_tar "$tar_file" "$board_dir/root")"

	[ -n "$rootfs_length" -o -n "$kernel_length" ] || return 1

	local mtdnum="$( find_mtd_index "$CI_UBIPART" )"
	if [ ! "$mtdnum" ]; then
		echo "cannot find ubi mtd partition ubi"
		return 1
	fi
	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	#cleanup old data volume if exist
	if [ "$ubidev" ] && [ "$( nand_find_volume $ubidev data )" ]; then
		ubidetach -m "$mtdnum"
		ubiformat /dev/mtd$mtdnum -y
		ubiattach -m "$mtdnum"
		ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	fi
	if [ ! "$ubidev" ]; then
		ubiattach -m "$mtdnum"
		ubidev="$( nand_find_ubi "$CI_UBIPART" )"
		if [ ! "$ubidev" ]; then
			ubiformat /dev/mtd$mtdnum -y
			ubiattach -m "$mtdnum"
			ubidev="$( nand_find_ubi "$CI_UBIPART" )"

			if [ ! "$ubidev" ]; then
				echo "cannot attach ubi mtd partition ubi"
				return 1
			fi
		fi
	fi

	local kern_mtdnum="$( find_mtd_index "ubi_kernel" )"
	if [ ! "$kern_mtdnum" ]; then
		echo "cannot find ubi_kernel mtd partition ubi_kernel"
		return 1
	fi
	local kern_ubidev="$( nand_find_ubi "ubi_kernel" )"
	if [ ! "$kern_ubidev" ]; then
		ubiattach -m "$kern_mtdnum"
		kern_ubidev="$( nand_find_ubi "ubi_kernel" )"
		if [ ! "$kern_ubidev" ]; then
			ubiformat /dev/mtd$kern_mtdnum -y
			ubiattach -m "$kern_mtdnum"
			kern_ubidev="$( nand_find_ubi "ubi_kernel" )"
			if [ ! "$kern_ubidev" ]; then
				echo "cannot attach ubi_kernel mtd partition ubi_kernel"
				return 1
			fi
		fi
	fi

	local kern_ubivol="$( nand_find_volume $kern_ubidev "kernel" )"
	local root_ubivol="$( nand_find_volume $ubidev "rootfs" )"
	local data_ubivol="$( nand_find_volume $ubidev rootfs_data )"

	[ "$kern_ubivol" ] && { nand_remove_ubiblock $kern_ubivol || return 1; }
	[ "$root_ubivol" ] && { nand_remove_ubiblock $root_ubivol || return 1; }
	[ "$data_ubivol" ] && { nand_remove_ubiblock $data_ubivol || return 1; }

	[ "$data_ubivol" ] && ubirmvol /dev/$ubidev -N rootfs_data || :
	[ "$root_ubivol" ] && ubirmvol /dev/$ubidev -N "rootfs" || :
	ubirmvol /dev/$kern_ubidev -N rootfs_data 2>/dev/null || :
	ubirmvol /dev/$kern_ubidev -N rootfs 2>/dev/null || :
	[ "$kern_ubivol" ] && ubirmvol /dev/$kern_ubidev -N "kernel" || :

	# create kernel vol in ubi_kernel
	if ! ubimkvol /dev/$kern_ubidev -N "kernel" -s $kernel_length; then
		echo "cannot create kernel volume"
		return 1
	fi

	# create rootfs vol in ubi
	local rootfs_size_param
	if [ "$rootfs_type" = "ubifs" ]; then
		rootfs_size_param="-m"
	else
		rootfs_size_param="-s $rootfs_length"
	fi
	if ! ubimkvol /dev/$ubidev -N "rootfs" $rootfs_size_param; then
		echo "cannot create rootfs volume"
		return 1;
	fi

	# create rootfs_data vol for non-ubifs rootfs in ubi
	if [ "$rootfs_type" != "ubifs" ]; then
		local rootfs_data_size_param="-m"
		if ! ubimkvol /dev/$ubidev -N rootfs_data $rootfs_data_size_param; then
			if ! ubimkvol /dev/$ubidev -N rootfs_data -m; then
				echo "cannot initialize rootfs_data volume"
				return 1
			fi
		fi
	fi

	root_ubivol="$( nand_find_volume $ubidev "rootfs" )"
	if [ "$root_ubivol" ]; then
		tar xf "$tar_file" "$board_dir/root" -O | \
			ubiupdatevol /dev/$root_ubivol -s $rootfs_length -
	fi

	kern_ubivol="$( nand_find_volume $kern_ubidev "kernel" )"
	if [ "$kern_ubivol" ]; then
		tar xf "$tar_file" "$board_dir/kernel" -O | \
			ubiupdatevol /dev/$kern_ubivol -s $kernel_length -
	fi

	nand_do_upgrade_success
}

xiaomi_mt7981_initial_setup()
{
	# initialize UBI and setup uboot-env if it's running on initramfs
	[ "$(rootfs_type)" = "tmpfs" ] || return 0

	local mtdnum="$( find_mtd_index ubi )"
	if [ ! "$mtdnum" ]; then
		echo "unable to find mtd partition ubi"
		return 1
	fi

	local kern_mtdnum="$( find_mtd_index ubi_kernel )"
	if [ ! "$kern_mtdnum" ]; then
		echo "unable to find mtd partition ubi_kernel"
		return 1
	fi

	ubidetach -m "$mtdnum"
	ubiformat /dev/mtd$mtdnum -y

	ubidetach -m "$kern_mtdnum"
	ubiformat /dev/mtd$kern_mtdnum -y

	if ! fw_printenv -n flag_try_sys2_failed &>/dev/null; then
		echo "failed to access u-boot-env. skip env setup."
		return 0
	fi

	fw_setenv boot_wait on
	fw_setenv uart_en 1
	fw_setenv flag_boot_rootfs 0
	fw_setenv flag_last_success 1
	fw_setenv flag_boot_success 1
	fw_setenv flag_try_sys1_failed 8
	fw_setenv flag_try_sys2_failed 8

	local board=$(board_name)
	case "$board" in
	xiaomi,mi-router-ax3000t-stock|\
	xiaomi,mi-router-wr30u-stock)
		fw_setenv mtdparts "nmbm0:1024k(bl2),256k(Nvram),256k(Bdata),2048k(factory),2048k(fip),256k(crash),256k(crash_log),34816k(ubi),34816k(ubi1),32768k(overlay),12288k(data),256k(KF)"
		;;
	esac
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	xiaomi,mi-router-wr30u-112m|\
	xiaomi,mi-router-ax3000t|\
	*mt3000* |\
	glinet,x3000-emmc |\
	*xe3000* |\
	*mt2500* |\
	*zr-3020* |\
	*360,t7* |\
	*abt,asr3000* |\
	*cetron,ct3003* |\
	*clt,r30b1* |\
	*imou,lc-hx3001* |\
	*jcg,q30* |\
	nradio,wt9103 |\
	cmcc,a10 |\
	cmcc,rax3000m |\
	h3c,nx30pro |\
	*konka,komi-a31* |\
	*nokia,ea0326gmp* |\
	*snand*)
		nand_do_upgrade "$1"
		;;
	cmcc,rax3000m-emmc |\
	*emmc*)
		CI_KERNPART="kernel"
		CI_ROOTPART="rootfs"
		emmc_do_upgrade "$1"
		;;
	xiaomi,mi-router-ax3000t-stock|\
	xiaomi,mi-router-wr30u-stock)
		xiaomi_mt7981_nand_upgrade_tar "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}

PART_NAME=firmware

platform_check_image() {
	local board=$(board_name)
	local magic="$(get_magic_long "$1")"

	[ "$#" -gt 1 ] && return 1

	case "$board" in
	*mt3000* |\
	glinet,x3000-emmc |\
	*xe3000* |\
	*mt2500* |\
	*zr-3020* |\
	*360,t7* |\
	xiaomi,mi-router-wr30u-stock|\
	xiaomi,mi-router-wr30u-112m|\
	xiaomi,mi-router-ax3000t* |\
	*abt,asr3000* |\
	*cetron,ct3003* |\
	*clt,r30b1* |\
	*imou,lc-hx3001* |\
	*jcg,q30* |\
	cmcc,a10 |\
	cmcc,rax3000m* |\
	h3c,nx30pro |\
	*konka,komi-a31* |\
	nradio,wt9103 |\
	*nokia,ea0326gmp* |\
	*snand* |\
	*emmc*)
		# tar magic `ustar`
		magic="$(dd if="$1" bs=1 skip=257 count=5 2>/dev/null)"

		[ "$magic" != "ustar" ] && {
			echo "Invalid image type."
			return 1
		}

		return 0
		;;
	*)
		[ "$magic" != "d00dfeed" ] && {
			echo "Invalid image type."
			return 1
		}
		return 0
		;;
	esac

	return 0
}

platform_pre_upgrade() {
	local board=$(board_name)

	case "$board" in
	xiaomi,mi-router-ax3000t-stock|\
	xiaomi,mi-router-wr30u-stock)
		xiaomi_mt7981_initial_setup
		;;
	esac
}
