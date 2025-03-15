#!/bin/sh

[ ! -x /usr/sbin/switch ] && return

# power off ports
# $1 ports list, ex: "1 2 3"
sw_poweroff_ports() {
	local ori_value
	local set_value
	[ -z "$1" ] && return 1
	for p in $1; do
		# read original value of register 0
		ori_value=$(switch phy cl22 r $p 0 | awk -F'=' '{print $3}')

		# register 0, bit 11 is power down control bit, set to 1
		set_value=$(($ori_value | 0x800))
		switch phy cl22 w $p 0 $set_value >/dev/null 2>&1
	done
}

# power on ports
# $1 ports list, ex: "1 2 3"
sw_poweron_ports() {
	local ori_value
	local set_value
	[ -z "$1" ] && return 1
	for p in $1; do
		# read original value of register 0
		ori_value=$(switch phy cl22 r $p 0 | awk -F'=' '{print $3}')

		# register 0, bit 11 is power down control bit, set to 0
		set_value=$(($ori_value & ~0x800))
		switch phy cl22 w $p 0 $set_value >/dev/null 2>&1
	done
}
# restart Auto-neg on ports
# $1 ports list, ex: "1 2 3"
sw_restart_port() {
	local ori_value
	local set_value
	[ -z "$1" ] && return 1
	for p in $1; do
		# read original value of register 0
		ori_value=$(switch phy cl22 r $p 0 | awk -F'=' '{print $3}')

		# register 0, bit 9 is Restart Auto-Negotiation bit, set to 1
		set_value=$(($ori_value | 0x200))
		switch phy cl22 w $p 0 $set_value >/dev/null 2>&1
	done
}
