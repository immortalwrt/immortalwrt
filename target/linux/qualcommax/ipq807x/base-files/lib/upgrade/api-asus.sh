. /lib/functions.sh
. /lib/upgrade/common.sh

asus_initial_setup() {
	# Remove existing linux and jffs2 volumes
	[ "$(rootfs_type)" = "tmpfs" ] || return 0

	ubirmvol /dev/ubi0 -N linux
	ubirmvol /dev/ubi0 -N jffs2
}
