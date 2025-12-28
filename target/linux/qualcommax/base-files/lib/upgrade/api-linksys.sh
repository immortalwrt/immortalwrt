. /lib/functions.sh
. /lib/upgrade/common.sh
. /lib/functions/bootconfig.sh

linksys_bootconfig_set_primaryboot() {
	local partname=$1
	local tempfile
	local mtdidx

	mtdidx=$(find_mtd_index "$partname")
	[ ! "$mtdidx" ] && {
		echo "cannot find mtd index for $partname"
		return 1
	}

	# No need to cleanup as files in /tmp will be removed upon reboot
	tempfile=/tmp/mtd"$mtdidx".bin
	dd if=/dev/mtd"$mtdidx" of="$tempfile" bs=1 count=336 2>/dev/null
	[ $? -ne 0 ] || [ ! -f "$tempfile" ]&& {
		echo "failed to create a temp copy of /dev/mtd$mtdidx"
		return 1
	}

	set_bootconfig_primaryboot "$tempfile" "0:HLOS" $2
	[ $? -ne 0 ] && {
		echo "failed to toggle primaryboot on 0:HLOS part"
		return 1
	}

	set_bootconfig_primaryboot "$tempfile" "rootfs" $2
	[ $? -ne 0 ] && {
		echo "failed to toggle primaryboot for rootfs part"
		return 1
	}

	mtd write "$tempfile" /dev/mtd"$mtdidx" 2>/dev/null
	[ $? -ne 0 ] && {
		echo "failed to write temp copy back to /dev/mtd$mtdidx"
		return 1
	}
}

linksys_bootconfig_pre_upgrade() {
	local setenv_script="/tmp/fw_env_upgrade"

	CI_UBIPART="rootfs_1"
	boot_part="$(fw_printenv -n boot_part)"
	if [ -n "$UPGRADE_OPT_USE_CURR_PART" ]; then
		CI_UBIPART="rootfs"
	else
		if [ "$boot_part" -eq "1" ]; then
			echo "boot_part 2" >> $setenv_script
			linksys_bootconfig_set_primaryboot "0:bootconfig" 1
			linksys_bootconfig_set_primaryboot "0:bootconfig1" 1
		else
			echo "boot_part 1" >> $setenv_script
			linksys_bootconfig_set_primaryboot "0:bootconfig" 0
			linksys_bootconfig_set_primaryboot "0:bootconfig1" 0
		fi
	fi

	boot_part_ready="$(fw_printenv -n boot_part_ready)"
	if [ "$boot_part_ready" -ne "3" ]; then
		echo "boot_part_ready 3" >> $setenv_script
	fi

	auto_recovery="$(fw_printenv -n auto_recovery)"
	if [ "$auto_recovery" != "yes" ]; then
		echo "auto_recovery yes" >> $setenv_script
	fi

	if [ -f "$setenv_script" ]; then
		fw_setenv -s $setenv_script || {
			echo "failed to update U-Boot environment"
			return 1
		}
	fi
}

linksys_mx_pre_upgrade() {
	local setenv_script="/tmp/fw_env_upgrade"

	CI_UBIPART="rootfs"
	boot_part="$(fw_printenv -n boot_part)"
	if [ -n "$UPGRADE_OPT_USE_CURR_PART" ]; then
		if [ "$boot_part" -eq "2" ]; then
			CI_KERNPART="alt_kernel"
			CI_UBIPART="alt_rootfs"
		fi
	else
		if [ "$boot_part" -eq "1" ]; then
			echo "boot_part 2" >> $setenv_script
			CI_KERNPART="alt_kernel"
			CI_UBIPART="alt_rootfs"
		else
			echo "boot_part 1" >> $setenv_script
		fi
	fi

	boot_part_ready="$(fw_printenv -n boot_part_ready)"
	if [ "$boot_part_ready" -ne "3" ]; then
		echo "boot_part_ready 3" >> $setenv_script
	fi

	auto_recovery="$(fw_printenv -n auto_recovery)"
	if [ "$auto_recovery" != "yes" ]; then
		echo "auto_recovery yes" >> $setenv_script
	fi

	if [ -f "$setenv_script" ]; then
		fw_setenv -s $setenv_script || {
			echo "failed to update U-Boot environment"
			return 1
		}
	fi
}
