. /lib/functions.sh
. /lib/upgrade/common.sh

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
