. /lib/functions.sh
. /lib/upgrade/common.sh

buffalo_upgrade_prepare() {
	local ubi_rootdev ubi_propdev

	if ! ubi_rootdev="$(nand_attach_ubi rootfs)" || ! ubi_propdev="$(nand_attach_ubi user_property)"; then
		echo "failed to attach UBI volume \"rootfs\" or \"user_property\", rebooting..."
		reboot -f
	fi

	ubirmvol /dev/$ubi_rootdev -N ubi_rootfs &> /dev/null || true
	ubirmvol /dev/$ubi_rootdev -N fw_hash &> /dev/null || true

	ubirmvol /dev/$ubi_propdev -N user_property_ubi &> /dev/null || true
	ubirmvol /dev/$ubi_propdev -N extra_property &> /dev/null || true
}

buffalo_upgrade_optvol() {
	local ubi_rootdev ubi_rcvrdev
	local hashvol_root hashvol_rcvr

	if ! ubi_rootdev="$(nand_attach_ubi rootfs)" || ! ubi_rcvrdev="$(nand_attach_ubi rootfs_recover)"; then
		echo "failed to attach UBI volume \"rootfs\" or \"rootfs_recover\", rebooting..."
		reboot -f
	fi

	ubimkvol /dev/$ubi_rootdev -N ubi_rootfs -S 1
	ubimkvol /dev/$ubi_rootdev -N fw_hash -S 1 -t static

	if ! hashvol_root="$(nand_find_volume $ubi_rootdev fw_hash)" || ! hashvol_rcvr="$(nand_find_volume $ubi_rcvrdev fw_hash)"; then
		echo "\"fw_hash\" volume in \"rootfs\" or \"rootfs_recover\" not found, rebooting..."
		reboot -f
	fi

	echo -n "00000000000000000000000000000000" > /tmp/dummyhash.txt
	ubiupdatevol /dev/$hashvol_root /tmp/dummyhash.txt
	ubiupdatevol /dev/$hashvol_rcvr /tmp/dummyhash.txt
}
