. /lib/functions.sh
. /lib/upgrade/common.sh
. /lib/functions/bootconfig.sh

qihoo_bootconfig_toggle_rootfs() {
	local partname=$1
	local tempfile
	local mtdidx

	mtdidx=$(find_mtd_index "$partname")
	[ ! "$mtdidx" ] && {
		echo "cannot find mtd index for $partname"
		return 1
	}

	tempfile=/tmp/mtd"$mtdidx".bin
	dd if=/dev/mtd"$mtdidx" of="$tempfile" bs=1 count=336 2>/dev/null
	[ $? -ne 0 ] || [ ! -f "$tempfile" ] && {
		echo "failed to create a temp copy of /dev/mtd$mtdidx"
		return 1
	}

	toggle_bootconfig_primaryboot "$tempfile" "rootfs"
	[ $? -ne 0 ] && {
		echo "failed to toggle primaryboot for rootfs partition"
		return 1
	}

	mtd write "$tempfile" /dev/mtd"$mtdidx" 2>/dev/null
	[ $? -ne 0 ] && {
		echo "failed to write temp copy back to /dev/mtd$mtdidx"
		return 1
	}

	# Update bootconfig1 if exists
	local mtdidx1=$(find_mtd_index "${partname}1")
	[ -n "$mtdidx1" ] && mtd write "$tempfile" /dev/mtd"$mtdidx1" 2>/dev/null

	return 0
}
