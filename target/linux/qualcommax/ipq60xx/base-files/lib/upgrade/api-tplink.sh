. /lib/functions.sh
. /lib/upgrade/common.sh

tplink_get_boot_part() {
	local cur_boot_part
	local args

	# Try to find rootfs from kernel arguments
	read -r args < /proc/cmdline
	for arg in $args; do
		local ubi_mtd_arg=${arg#ubi.mtd=}
		case "$ubi_mtd_arg" in
		rootfs|rootfs_1)
			echo "$ubi_mtd_arg"
			return
		;;
		esac
	done

	# Fallback to u-boot env (e.g. when running initramfs)
	cur_boot_part="$(/usr/sbin/fw_printenv -n tp_boot_idx)"
	case $cur_boot_part in
	1)
		echo rootfs_1
		;;
	0|*)
		echo rootfs
		;;
	esac
}

tplink_do_upgrade() {
	local new_boot_part

	case $(tplink_get_boot_part) in
	rootfs)
		CI_UBIPART="rootfs_1"
		new_boot_part=1
	;;
	rootfs_1)
		CI_UBIPART="rootfs"
		new_boot_part=0
	;;
	esac

	fw_setenv -s - <<-EOF
		tp_boot_idx $new_boot_part
	EOF

	nand_do_upgrade "$1"
}
