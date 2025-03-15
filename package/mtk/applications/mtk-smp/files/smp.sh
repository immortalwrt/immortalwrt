#!/bin/sh

OPTIMIZED_FOR="$1"
CPU_LIST=`cat /proc/interrupts | sed -n '1p'`
NUM_OF_CPU=0; for i in $CPU_LIST; do NUM_OF_CPU=`expr $NUM_OF_CPU + 1`; done;
DEFAULT_RPS=0

. /lib/functions.sh

# $1: CPU#
# $2: irq list for added.
CPU_AFFINITY_ADD()
{
	eval oval=\$CPU${1}_AFFINITY
	eval CPU${1}_AFFINITY=\"\$CPU${1}_AFFINITY $2\"
}

# $1: CPU#
# $2: Interface name for added.
CPU_RPS_ADD()
{
	eval oval=\$CPU${1}_RPS
	eval CPU${1}_RPS=\"\$CPU${1}_RPS $2\"
	dbg2 "CPU${1}_RPS=\"\$CPU${1}_RPS $2\""
}

MT7986_whnat()
{
	num_of_wifi=$1
	is_usbnet=$2
	DEFAULT_RPS=0

	#Physical IRQ# setting
	PCIe0=
	eth_tx=229
	eth_rx0=230
	usb=205
	if [ -d "/proc/warp_ctrl/warp0" ]; then
	wifi1_irq=237
	wifi2_irq=237
	wifi3_irq=
	else
        wifi1_irq=245
        wifi2_irq=245
        wifi3_irq=
	fi

	# Please update the CPU binding in each cases.
	# CPU#_AFFINITY="add binding irq number here"
	# CPU#_RPS="add binding interface name here"
	dbg "[MT7986_whnat]"
	if [ "$num_of_wifi" = "0" ]; then
		CPU0_AFFINITY="$eth_rx0"
		CPU1_AFFINITY="$eth_tx"
		CPU2_AFFINITY="$usb"
		CPU3_AFFINITY=""

		CPU0_RPS=""
		CPU1_RPS="$ethif1 $ethif2"
		CPU2_RPS="$ethif1 $ethif2"
		CPU3_RPS="$ethif1 $ethif2"
	elif [ "$num_of_wifi" = "1" ]; then
		CPU0_AFFINITY="$eth_rx0"
		CPU1_AFFINITY="$eth_tx"
		CPU2_AFFINITY="$usb"
		CPU3_AFFINITY="$wifi1_irq"

		CPU0_RPS="                $wifi1 $wifi1_apcli0"
		CPU1_RPS="$ethif1 $ethif2 $wifi1 $wifi1_apcli0"
		CPU2_RPS="$ethif1 $ethif2 $wifi1 $wifi1_apcli0"
		CPU3_RPS="$ethif1 $ethif2 "
	elif [ "$num_of_wifi" = "2" ]; then
		CPU0_AFFINITY="$eth_rx0"
		CPU1_AFFINITY="$eth_tx"
		CPU2_AFFINITY="$usb"
		CPU3_AFFINITY="$wifi1_irq $wifi2_irq"

		CPU0_RPS="                $wifi1 $wifi2 $wifi1_apcli0 $wifi2_apcli0"
  		[ "$is_usbnet" = "1" ] && CPU0_RPS="$wifi1_apcli0 $wifi2_apcli0"
		CPU1_RPS="$ethif1 $ethif2 $wifi1 $wifi2 $wifi1_apcli0 $wifi2_apcli0"
		CPU2_RPS="$ethif1 $ethif2 $wifi1 $wifi2 $wifi1_apcli0 $wifi2_apcli0"
		CPU3_RPS="$ethif1 $ethif2"
	elif [ "$num_of_wifi" = "3" ]; then
		CPU0_AFFINITY="$eth_tx $eth_rx0"
		CPU1_AFFINITY="$wifi1_irq $wifi2_irq"
		CPU2_AFFINITY="$PCIe0 $wifi3_irq $usb"
		CPU3_AFFINITY=""

		CPU0_RPS=""
		CPU1_RPS="$ethif1 $ethif2                                           $wifi3 $wifi3_apcli0"
		CPU2_RPS="$ethif1 $ethif2 $wifi1 $wifi2 $wifi1_apcli0 $wifi2_apcli0"
		CPU3_RPS="$ethif1 $ethif2 $wifi1 $wifi2 $wifi3 $wifi1_apcli0 $wifi2_apcli0 $wifi3_apcli0"
	else
		dbg "MT7986_whnat with $NUM_OF_WIFI Wi-Fi bands is not support"
	fi
}

MT7986()
{
	num_of_wifi=$1
	storage=$2
	DEFAULT_RPS=0

	#Physical IRQ# setting
	PCIe0=
	eth_tx=229
	eth_rx0=230
	wifi1_irq=245
	wifi2_irq=
	wifi3_irq=

	# Please update the CPU binding in each cases.
	# CPU#_AFFINITY="add binding irq number here"
	# CPU#_RPS="add binding interface name here"
	dbg "[MT7986]"
	if [ "$num_of_wifi" = "0" ]; then
		CPU0_AFFINITY="$eth_rx0"
		CPU1_AFFINITY="$eth_tx"
		CPU2_AFFINITY=""
		CPU3_AFFINITY=""

		CPU0_RPS=""
		CPU1_RPS="$ethif1 $ethif2"
		CPU2_RPS="$ethif1 $ethif2"
		CPU3_RPS="$ethif1 $ethif2"
	elif [ "$num_of_wifi" = "1" ]; then
		CPU0_AFFINITY="$eth_tx $eth_rx0"
		CPU1_AFFINITY="$wifi1_irq"
		CPU2_AFFINITY=""
		CPU3_AFFINITY=""

		CPU0_RPS=""
		CPU1_RPS=""
		CPU2_RPS="$ethif1 $ethif2 $wifi1 $wifi1_apcli0"
		CPU3_RPS="$ethif1 $ethif2 $wifi1 $wifi1_apcli0"
	elif [ "$num_of_wifi" = "2" ]; then
		CPU0_AFFINITY="$eth_tx $eth_rx0"
		CPU1_AFFINITY="$wifi1_irq"
		CPU2_AFFINITY="$PCIe0 $wifi2_irq"
		CPU3_AFFINITY=""

		CPU0_RPS=""
		CPU1_RPS="$ethif1 $ethif2 $wifi2 $wifi2_apcli0"
		CPU2_RPS="$ethif1 $ethif2 $wifi1 $wifi1_apcli0"
		CPU3_RPS="$ethif1 $ethif2 $wifi1 $wifi2 $wifi1_apcli0 $wifi2_apcli0"
	elif [ "$num_of_wifi" = "3" ]; then
		CPU0_AFFINITY="$eth_tx $eth_rx0 $wifi3_irq"
		CPU1_AFFINITY="$PCIe0 $wifi2_irq $wifi1_irq"
		CPU2_AFFINITY=""
		CPU3_AFFINITY=""

		CPU0_RPS=""
		CPU1_RPS=""
		CPU2_RPS="$ethif1 $ethif2 $wifi1 $wifi2 $wifi3 $wifi1_apcli0 $wifi2_apcli0 $wifi3_apcli0"
		CPU3_RPS="$ethif1 $ethif2 $wifi1 $wifi2 $wifi3 $wifi1_apcli0 $wifi2_apcli0 $wifi3_apcli0"
	else
		dbg "MT7986_whnat with $NUM_OF_WIFI Wi-Fi bands is not support"
	fi
}

MT7986_dbdc1()
{
	num_of_wifi=$1
	storage=$2
	DEFAULT_RPS=0

	#Physical IRQ# setting
	PCIe0=
	eth_tx=229
	eth_rx0=230
	# WARP OFF -> wifi irq change from 237 to 245
	wifi1_irq=245
	wifi2_irq=245
	wifi3_irq=

	# Please update the CPU binding in each cases.
	# CPU#_AFFINITY="add binding irq number here"
	# CPU#_RPS="add binding interface name here"
	dbg "[MT7986_dbdc1]"
	if [ "$num_of_wifi" = "2" ]; then
		CPU0_AFFINITY="$eth_tx $eth_rx0"
		CPU1_AFFINITY="$wifi1_irq $wifi2_irq"
		CPU2_AFFINITY=""
		CPU3_AFFINITY=""

		CPU0_RPS=""
		CPU1_RPS=""
		CPU2_RPS="$ethif1 $ethif2 $wifi1 $wifi2 $wifi1_apcli0 $wifi2_apcli0"
		CPU3_RPS="$ethif1 $ethif2 $wifi1 $wifi2 $wifi1_apcli0 $wifi2_apcli0"
	elif [ "$num_of_wifi" = "3" ]; then
		CPU0_AFFINITY="$eth_tx $eth_rx0"
		CPU1_AFFINITY="$wifi1_irq $wifi2_irq"
		CPU2_AFFINITY="$PCIe0 $wifi3_irq"
		CPU3_AFFINITY=""

		CPU0_RPS=""
		CPU1_RPS="$ethif1 $ethif2 $wifi3 $wifi3_apcli0"
		CPU2_RPS="$ethif1 $ethif2 $wifi1 $wifi2 $wifi1_apcli0 $wifi2_apcli0"
		CPU3_RPS="$ethif1 $ethif2 $wifi1 $wifi2 $wifi3 $wifi1_apcli0 $wifi2_apcli0 $wifi3_apcli0"
	else
		dbg "MT7986_dbdc1 with $NUM_OF_WIFI Wi-Fi bands is not support"
	fi
}

MT7981_whnat()
{
	num_of_wifi=$1
	is_usbnet=$2
	DEFAULT_RPS=0

	#Physical IRQ# setting
	PCIe0=
	eth_tx=229
	eth_rx0=230
	usb=205
	if [ -d "/proc/warp_ctrl/warp0" ]; then
		wifi1_irq=237
		wifi2_irq=237
		wifi3_irq=
	else
		wifi1_irq=245
		wifi2_irq=245
		wifi3_irq=
	fi

	# Please update the CPU binding in each cases.
	# CPU#_AFFINITY="add binding irq number here"
	# CPU#_RPS="add binding interface name here"
	dbg "[MT7981_whnat]"
	if [ "$num_of_wifi" = "0" ]; then
		CPU0_AFFINITY="$eth_rx0"
		CPU1_AFFINITY="$eth_tx $usb"

		CPU0_RPS=""
		CPU1_RPS="$ethif1 $ethif2"
	elif [ "$num_of_wifi" = "1" ]; then
		CPU0_AFFINITY="$eth_tx $eth_rx0"
		CPU1_AFFINITY="$wifi1_irq $usb"

		CPU0_RPS="$ethif1 $ethif2 $wifi1 $wifi1_apcli0"
		CPU1_RPS="                $wifi1 $wifi1_apcli0"
	elif [ "$num_of_wifi" = "2" ]; then
		CPU0_AFFINITY="$eth_tx $eth_rx0"
		CPU1_AFFINITY="$wifi1_irq $wifi2_irq $usb"

		CPU0_RPS="$ethif1 $ethif2 $wifi1 $wifi2 $wifi1_apcli0 $wifi2_apcli0"
		CPU1_RPS="                $wifi1 $wifi2 $wifi1_apcli0 $wifi2_apcli0"
		[ "$is_usbnet" = "1" ] && CPU1_RPS="$wifi1_apcli0 $wifi2_apcli0"
	elif [ "$num_of_wifi" = "3" ]; then
		CPU0_AFFINITY="$eth_tx $eth_rx0"
		CPU1_AFFINITY="$PCIe0 $wifi1_irq $wifi2_irq $wifi3_irq $usb"

		CPU0_RPS="$ethif1 $ethif2 $wifi1 $wifi2 $wifi3 $wifi1_apcli0 $wifi2_apcli0 $wifi3_apcli0"
		CPU1_RPS="                $wifi1 $wifi2 $wifi3 $wifi1_apcli0 $wifi2_apcli0 $wifi3_apcli0"
	else
		dbg "MT7981_whnat with $NUM_OF_WIFI Wi-Fi bands is not support"
	fi
}

MT7981()
{
	num_of_wifi=$1
	storage=$2
	DEFAULT_RPS=0

	#Physical IRQ# setting
	PCIe0=
	eth_tx=229
	eth_rx0=230
	wifi1_irq=245
	wifi2_irq=
	wifi3_irq=

	# Please update the CPU binding in each cases.
	# CPU#_AFFINITY="add binding irq number here"
	# CPU#_RPS="add binding interface name here"
	dbg "[MT7981]"
	if [ "$num_of_wifi" = "0" ]; then
		CPU0_AFFINITY="$eth_rx0"
		CPU1_AFFINITY="$eth_tx"

		CPU0_RPS=""
		CPU1_RPS="$ethif1 $ethif2"
	elif [ "$num_of_wifi" = "1" ]; then
		CPU0_AFFINITY="$eth_tx $eth_rx0"
		CPU1_AFFINITY="$wifi1_irq"

		CPU0_RPS="$ethif1 $ethif2 $wifi1 $wifi1_apcli0"
		CPU1_RPS="                $wifi1 $wifi1_apcli0"
	elif [ "$num_of_wifi" = "2" ]; then
		CPU0_AFFINITY="$eth_tx $eth_rx0"
		CPU1_AFFINITY="$PCIe0 $wifi1_irq $wifi2_irq"

		CPU0_RPS="$ethif1 $ethif2 $wifi1 $wifi2 $wifi1_apcli0 $wifi2_apcli0"
		CPU1_RPS="                $wifi1 $wifi2 $wifi1_apcli0 $wifi2_apcli0"
	elif [ "$num_of_wifi" = "3" ]; then
		CPU0_AFFINITY="$eth_tx $eth_rx0 $wifi3_irq"
		CPU1_AFFINITY="$PCIe0 $wifi2_irq $wifi1_irq"

		CPU0_RPS="$ethif1 $ethif2 $wifi1 $wifi2 $wifi3 $wifi1_apcli0 $wifi2_apcli0 $wifi3_apcli0"
		CPU1_RPS="                $wifi1 $wifi2 $wifi3 $wifi1_apcli0 $wifi2_apcli0 $wifi3_apcli0"
	else
		dbg "MT7981_whnat with $NUM_OF_WIFI Wi-Fi bands is not support"
	fi
}

MT7981_dbdc1()
{
	num_of_wifi=$1
	storage=$2
	DEFAULT_RPS=0

	#Physical IRQ# setting
	PCIe0=
	eth_tx=229
	eth_rx0=230
	# WARP OFF -> wifi irq change from 237 to 245
	wifi1_irq=245
	wifi2_irq=245
	wifi3_irq=

	# Please update the CPU binding in each cases.
	# CPU#_AFFINITY="add binding irq number here"
	# CPU#_RPS="add binding interface name here"
	dbg "[MT7981_dbdc1]"
	if [ "$num_of_wifi" = "2" ]; then
		CPU0_AFFINITY="$eth_tx $eth_rx0"
		CPU1_AFFINITY="$wifi1_irq $wifi2_irq"

		CPU0_RPS="$ethif1 $ethif2 $wifi1 $wifi2 $wifi1_apcli0 $wifi2_apcli0"
		CPU1_RPS="                $wifi1 $wifi2 $wifi1_apcli0 $wifi2_apcli0"
	elif [ "$num_of_wifi" = "3" ]; then
		CPU0_AFFINITY="$eth_tx $eth_rx0"
		CPU1_AFFINITY="$PCIe0 $wifi1_irq $wifi2_irq $wifi3_irq"

		CPU0_RPS="$ethif1 $ethif2 $wifi1 $wifi2 $wifi3 $wifi1_apcli0 $wifi2_apcli0 $wifi3_apcli0"
		CPU1_RPS="                $wifi1 $wifi2 $wifi3 $wifi1_apcli0 $wifi2_apcli0 $wifi3_apcli0"
	else
		dbg "MT7981_dbdc1 with $NUM_OF_WIFI Wi-Fi bands is not support"
	fi
}

MT7622_whnat()
{
	num_of_wifi=$1
	storage=$2
	DEFAULT_RPS=0

	#Physical IRQ# setting
	PCIe0=228
	PCIe1=229
	eth_rx1=223
	eth_tx=224
	eth_rx0=225
	wifi1_irq=211
	wifi2_irq=214
	wifi3_irq=215

	# Please update the CPU binding in each cases.
	# CPU#_AFFINITY="add binding irq number here"
	# CPU#_RPS="add binding interface name here"
	dbg "[MT7622_whnat]"
	if [ "$num_of_wifi" = "0" ]; then
		CPU0_AFFINITY="$eth_rx1 $eth_tx"
		CPU1_AFFINITY="$eth_rx0"

		CPU0_RPS=""
		CPU1_RPS=""
	elif [ "$num_of_wifi" = "1" ]; then
		CPU0_AFFINITY="$eth_rx1 $eth_tx $eth_rx0"
		CPU1_AFFINITY="$wifi1_irq $PCIe0 $PCIe1 $wifi2_irq $wifi3_irq"

		CPU0_RPS=""
		CPU1_RPS=""
	elif [ "$num_of_wifi" = "2" ]; then
		CPU0_AFFINITY="$eth_rx1 $eth_tx $eth_rx0 $wifi1_irq"
		CPU1_AFFINITY="$PCIe0 $PCIe1 $wifi2_irq"

		CPU0_RPS="$wifi2"
		CPU1_RPS=""
	elif [ "$num_of_wifi" = "3" ]; then
		CPU0_AFFINITY="$eth_rx1 $eth_tx $eth_rx0 $PCIe1 $wifi3_irq"
		CPU1_AFFINITY="$PCIe0 $wifi2_irq $wifi1_irq"

		CPU0_RPS=""
		CPU1_RPS=""
	else
		dbg "MT7622_whnat with $NUM_OF_WIFI Wi-Fi bands is not support"
	fi
}

MT7622()
{
	num_of_wifi=$1
	storage=$2
	DEFAULT_RPS=0

	#Physical IRQ# setting
	PCIe0=228
	PCIe1=229
	eth_rx1=223
	eth_tx=224
	eth_rx0=225
	wifi1_irq=211
	wifi2_irq=214
	wifi3_irq=215

	# Please update the CPU binding in each cases.
	# CPU#_AFFINITY="add binding irq number here"
	# CPU#_RPS="add binding interface name here"
	dbg "[MT7622]"
	if [ "$num_of_wifi" = "0" ]; then
		CPU0_AFFINITY="$eth_rx1 $eth_tx"
		CPU1_AFFINITY="$eth_rx0"

		CPU0_RPS=""
		CPU1_RPS=""
	elif [ "$num_of_wifi" = "1" ]; then
		CPU0_AFFINITY="$eth_rx1 $eth_tx $eth_rx0"
		CPU1_AFFINITY="$wifi1_irq"

		CPU0_RPS=""
		CPU1_RPS=""
	elif [ "$num_of_wifi" = "2" ]; then
		CPU0_AFFINITY="$eth_rx1 $eth_tx $eth_rx0"
		CPU1_AFFINITY="$wifi1_irq"

		CPU0_RPS="$ethif1 $wifi1 $wifi2"
		CPU1_RPS="$ethif1 "
	elif [ "$num_of_wifi" = "3" ]; then
		CPU0_AFFINITY="$eth_rx1 $eth_tx $eth_rx0 $PCIe1 $wifi3_irq"
		CPU1_AFFINITY="$PCIe0 $wifi2_irq $wifi1_irq"

		CPU0_RPS=""
		CPU1_RPS=""
	else
		dbg "MT7622_whnat with $NUM_OF_WIFI Wi-Fi bands is not support"
	fi
}

MT7622_dbdc1()
{
	num_of_wifi=$1
	storage=$2
	DEFAULT_RPS=0

	#Physical IRQ# setting
	PCIe0=228
	PCIe1=229
	eth_rx1=223
	eth_tx=224
	eth_rx0=225
	wifi1_irq=211
	wifi2_irq=214

	# Please update the CPU binding in each cases.
	# CPU#_AFFINITY="add binding irq number here"
	# CPU#_RPS="add binding interface name here"
	dbg "[MT7622_dbdc1]"
	if [ "$num_of_wifi" = "2" ]; then
		CPU0_AFFINITY="$eth_rx1 $eth_tx $eth_rx0"
		CPU1_AFFINITY="$wifi1_irq"

		CPU0_RPS="$wifi1 $wifi3"
		CPU1_RPS=""
	elif [ "$num_of_wifi" = "3" ]; then
		CPU0_AFFINITY="$eth_rx1 $eth_tx $eth_rx0"
		CPU1_AFFINITY="$PCIe0 $wifi2_irq $wifi1_irq"

		CPU0_RPS=""
		CPU1_RPS=""
	else
		dbg "MT7622_dbdc1 with $NUM_OF_WIFI Wi-Fi bands is not support"
	fi
}

MT7623()
{
	num_of_wifi=$1
	storage=$2
	DEFAULT_RPS=0

	#Physical IRQ# setting
	eth_rx1=200
	eth_tx=199
	eth_rx0=198
	PCIe0=193
	PCIe1=194
	PCIe2=195
	SDXC=40
	USB_P0=196
	USB_P1=197

	# Please update the CPU binding in each cases.
	# CPU#_AFFINITY="add binding irq number here"
	# CPU#_RPS="add binding interface name here"
	dbg "[MT7623]"
	if [ "$num_of_wifi" = "0" ]; then
		CPU0_AFFINITY="$eth_tx"
		CPU1_AFFINITY="$eth_rx1 $eth_rx0"
		CPU2_AFFINITY="$PCIe2 $USB_P1"
		CPU3_AFFINITY="$PCIe1 $SDXC $USB_P0"

		CPU0_RPS="$ethif1 $ethif2"
		CPU1_RPS="$ethif1 $ethif2"
		CPU2_RPS="$ethif1 $ethif2"
		CPU3_RPS="$ethif1 $ethif2"
	elif [ "$num_of_wifi" = "1" ]; then
		CPU0_AFFINITY="$eth_tx"
		CPU1_AFFINITY="$eth_rx1 $eth_rx0"
		CPU2_AFFINITY="$PCIe0 $PCIe2"
		CPU3_AFFINITY="$PCIe1 $SDXC $USB_P0 $USB_P1"

		CPU0_RPS="$ethif1 $ethif2 $wifi1"
		CPU1_RPS="                $wifi1"
		CPU2_RPS="$ethif1 $ethif2"
		CPU3_RPS=""
	elif [ "$num_of_wifi" = "2" ]; then
		CPU0_AFFINITY="$eth_tx"
		CPU1_AFFINITY="$eth_rx1 $eth_rx0"
		CPU2_AFFINITY="$PCIe0 $USB_P0 $USB_P1"
		CPU3_AFFINITY="$PCIe1 $SDXC"

		CPU0_RPS="$ethif1 $ethif2 $wifi2"
		CPU1_RPS="$ethif1 $ethif2"
		CPU2_RPS="$ethif1 $ethif2 $wifi1 $wifi2"
		CPU3_RPS="$ethif1 $ethif2 $wifi1"
	elif [ "$num_of_wifi" = "3" ]; then
		CPU0_AFFINITY="$eth_tx $PCIe0"
		CPU1_AFFINITY="$eth_rx1 $eth_rx0"
		CPU2_AFFINITY="$PCIe2 "
		CPU3_AFFINITY="$PCIe1 $SDXC $USB_P0 $USB_P1"

		CPU0_RPS="$ethif1 $ethif2 $wifi1               $wifi1_apcli0"
		CPU1_RPS="                $wifi1 $wifi2 $wifi3 $wifi1_apcli0 $wifi2_apcli0 $wifi3_apcli0"
		CPU2_RPS="$ethif1 $ethif2        $wifi2                      $wifi2_apcli0"
		CPU3_RPS="$ethif1 $ethif2               $wifi3                             $wifi3_apcli0"
	else
		dbg "MT7623 with $NUM_OF_WIFI Wi-Fi bands is not support"
	fi
}

# $1: The prefix of vifs
# $2: The number of vifs
gen_vifs_to_rps_if()
{
	if [ $# -lt 2 ]; then
		dbg "gen_vifs_to_rps_if requires 2 parameters"
		return
	fi

	vif=$1
	total=$2
	#dbg "gen_vifs_to_rps_if $vif $total"
	i=0
	while [ "$i" -lt "$total" ]; do
		eval prefix=\$$vif
		eval $vif$i=$prefix$i

		RPS_IF_LIST="$RPS_IF_LIST $prefix$i"

		dbg2 "\$$vif$i=$prefix$i"

		i=`expr $i + 1`
	done
}

get_eth_if_name()
{
	ethif1="eth0"
	ethif2="eth1"
	dbg2 "# Ethernet interface list"
	dbg2 "\$ethif1=$ethif1\n\$ethif2=$ethif2"
	RPS_IF_LIST="$RPS_IF_LIST $ethif1 $ethif2"
}

# Try to get Wi-Fi interface name from l1profile
get_wifi_if_name()
{
	l1dat_exist=`l1dat 2>/dev/null`
	if [ -z "$l1dat_exist" ]; then
		dbg "Layer 1 profile does not exist."
		dbg "Please check l1dat "

		wifi1="ra0"
		wifi1_prefix="ra"
		wifi1_apcli="apcli"
		wifi1_wds="wds"
		wifi1_mesh="mesh"
		wifi2="rai0"
		wifi2_prefix="rai"
		wifi2_apcli="apclii"
		wifi2_wds="wdsi"
		wifi2_mesh="meshi"
		wifi3="rae0"
		wifi3_prefix="rae"
		wifi3_apcli="apclie"
		wifi3_wds="wdse"
		wifi3_mesh="meshe"

	else
		#wifi_if1s=`l1dat idx2if 1`
		#wifi_if2s=`l1dat idx2if 2`
		#wifi_if3s=`l1dat idx2if 3`
		wifi_if1s=`l1dat zone2if dev1`
		wifi_if2s=`l1dat zone2if dev2`
		wifi_if3s=`l1dat zone2if dev3`
	
		wifi1=`echo $wifi_if1s | awk '{print $1}'`
		wifi1_prefix=`echo $wifi_if1s | awk '{print $2}'`
		wifi1_apcli=`echo $wifi_if1s | awk '{print $3}'`
		wifi1_wds=`echo $wifi_if1s | awk '{print $4}'`
		wifi1_mesh=`echo $wifi_if1s | awk '{print $5}'`
	
		wifi2=`echo $wifi_if2s | awk '{print $1}'`
		wifi2_prefix=`echo $wifi_if2s | awk '{print $2}'`
		wifi2_apcli=`echo $wifi_if2s | awk '{print $3}'`
		wifi2_wds=`echo $wifi_if2s | awk '{print $4}'`
		wifi2_mesh=`echo $wifi_if2s | awk '{print $5}'`
	
		wifi3=`echo $wifi_if3s | awk '{print $1}'`
		wifi3_prefix=`echo $wifi_if3s | awk '{print $2}'`
		wifi3_apcli=`echo $wifi_if3s | awk '{print $3}'`
		wifi3_wds=`echo $wifi_if3s | awk '{print $4}'`
		wifi3_mesh=`echo $wifi_if3s | awk '{print $5}'`

		# idx = 0 : not a DBDC interface
		# idx = 1 : main(physical) interface of 1st Wi-Fi band
		# idx > 1 : virtual interface of other Wi-Fi band
		# idx = "": Wi-Fi interface does not exist in l1profile
		wifi1_dbdc_idx=`l1dat if2dbdcidx $wifi1`
		wifi2_dbdc_idx=`l1dat if2dbdcidx $wifi2`
		wifi3_dbdc_idx=`l1dat if2dbdcidx $wifi3`
	fi

	dbg2 "# Wi-Fi interface list"
	dbg2 "\$wifi1=$wifi1"
	dbg2 "\$wifi2=$wifi2"
	dbg2 "\$wifi3=$wifi3"

	RPS_IF_LIST="$RPS_IF_LIST $wifi1 $wifi2 $wifi3"

	gen_vifs_to_rps_if "wifi1_apcli" 1
	gen_vifs_to_rps_if "wifi2_apcli" 1
	gen_vifs_to_rps_if "wifi3_apcli" 1
	gen_vifs_to_rps_if "wifi1_mesh" 1
	gen_vifs_to_rps_if "wifi2_mesh" 1
	gen_vifs_to_rps_if "wifi3_mesh" 1
	gen_vifs_to_rps_if "wifi1_wds" 4
	gen_vifs_to_rps_if "wifi2_wds" 4
	gen_vifs_to_rps_if "wifi3_wds" 4

	scan_wifi_num
}

scan_wifi_num()
{
	NUM_OF_WIFI=0
	if [ -n "$wifi1" -a -d "/sys/class/net/$wifi1" ]; then
		NUM_OF_WIFI=`expr $NUM_OF_WIFI + 1`
	fi

	if [ -n "$wifi2" -a -d "/sys/class/net/$wifi2" ];then
		NUM_OF_WIFI=`expr $NUM_OF_WIFI + 1`
	fi

	if [ -n "$wifi3" -a -d "/sys/class/net/$wifi3" ];then
		NUM_OF_WIFI=`expr $NUM_OF_WIFI + 1`
	fi

	dbg "# NUM_OF_WIFI=$NUM_OF_WIFI band(s)"
}

scan_usbnet()
{
	for dev in /sys/class/net/*; do
		[ -d "$dev" ] || continue
		dev_name=$(basename $dev)
		dev_prefix="${dev_name%%[0-9]*}"
		if [ "$dev_prefix" = "usb" ] || [ "$dev_prefix" = "wwan" ] || [ "$dev_prefix" = "rmnet" ] || [ "$dev_prefix" = "eth2" ] || [ "$dev_prefix" = "eth3" ] || [ "$dev_prefix" = "eth4" ] || [ "$dev_prefix" = "eth5" ]; then
			IS_USBNET=1
			return
		fi
	done
}

get_usbnet()
{
	echo $IS_USBNET
}

get_wifi_num()
{
	echo $NUM_OF_WIFI
}

# $1: module name
# return value
#    1: if the module named $1 is built-in or inserted.
#    0: if the module exists but has not been inserted.
#   -1: if the module does not exist.
module_exist()
{
	mpath="/lib/modules/`uname -r`"
	retval=-1

	mod_in_lib=`find $mpath -name "$1".ko > /dev/null 2>&1`
	#echo "find $mpath -name "$1".ko" > /dev/console
	if [ ! -z $mod_in_lib ]; then
		retval=0
	fi

	# TODO find out a way in OpenWRT
	mod_builtin=`grep $1 $mpath/modules.builtin 2>/dev/null`
	if [ ! -z "$mod_builtin" ]; then
		retval=1
	fi

	mod_inserted=`lsmod | grep $1 2>/dev/null`
	if [ ! -z "$mod_inserted" ]; then
		retval=1
	fi

	echo $retval
}

setup_model()
{
	board=$(board_name)
	num_of_wifi=$(get_wifi_num)
	mt_whnat_en=$(module_exist "mt_whnat")
	usbnet=$(get_usbnet)

	logger -t "mtk_smp" "board=${board}, wifi_num=${num_of_wifi}, cpu_num=${NUM_OF_CPU}, usbnet=${usbnet}"

	case $board in
	xiaomi,redmi-router-ax6000* |\
	bananapi,bpi-r3mini* |\
	netcore,n60 |\
	glinet,gl-mt6000|\
	jdcloud,re-cp-03 |\
	tplink,tl-xdr608* |\
	*7986*)
		MT7986_whnat $num_of_wifi $usbnet
		;;
	*mt3000* |\
	glinet,x3000-emmc |\
	*xe3000* |\
	*mt2500* |\
	*zr-3020* |\
	*360,t7* |\
	abt,asr3000* |\
	*clt,r30b1* |\
	cmcc,a10* |\
	xiaomi,mi-router-wr30u* |\
	xiaomi,mi-router-ax3000t* |\
	*rax3000m* |\
	h3c,nx30pro |\
	konka,komi-a31 |\
	*nokia,ea0326gmp* |\
	nradio,wt9103 |\
	*7981*)
		MT7981_whnat $num_of_wifi $usbnet
		;;
	*)
		if [ "$NUM_OF_CPU" = "4" ]; then
			dbg "setup_model:MT7623 wifi#=$num_of_wifi"
			MT7623 $num_of_wifi
		elif [ "$NUM_OF_CPU" = "2" ]; then
			if [ "$mt_whnat_en" = "1" ];then
				MT7622_whnat $num_of_wifi
			else
				if [ "$wifi1_dbdc_idx" = "1" ]; then
					MT7622_dbdc1 $num_of_wifi
				else
					MT7622 $num_of_wifi
				fi
			fi
		fi
		;;
	esac
}

get_virtual_irq()
{
	PHY_POS=`expr $NUM_OF_CPU + 3` #physical irq # position in /proc/interrups may vary with the number of CPU up
	target_phy_irq=$1
	cat /proc/interrupts | sed 's/:/ /g'| awk '$1 ~ /^[0-9]+$/' | while read line 
	do
		set -- $line
		phy_irq=$(eval "echo \$$PHY_POS")
		if [ $phy_irq == $target_phy_irq ]; then 
			echo $1
			return
		fi
	done
}


set_rps_cpu_bitmap()
{
	dbg2 "# Scan binding interfaces of each cpu"
	# suppose the value of interface_var is null or hex
	num=0
	while [ "$num" -lt "$NUM_OF_CPU" ];do
		cpu_bit=$((2 ** $num))
		eval rps_list=\$CPU${num}_RPS
		dbg2 "# CPU$num: rps_list=$rps_list"
		for i in $rps_list; do
			var=${VAR_PREFIX}_${i//-/_}
			eval ifval=\$$var
			dbg2 "[var val before] \$$var=$ifval"
			if [ -z "$ifval" ]; then
				eval $var=$cpu_bit
			else
				eval $var=`expr $ifval + $cpu_bit`
			fi
			eval ifval=\$$var
			dbg2 "[rps val after]$i=$ifval"
		done
		num=`expr $num + 1`
	done
}

# $1: The default rps value. If rps of the interface is not setup, set $1 to it
set_rps_cpus()
{
	dbg2 "# Setup rps of the interfaces, $RPS_IF_LIST."
	for i in $RPS_IF_LIST; do
		var=${VAR_PREFIX}_${i//-/_}
		eval cpu_map=\$$var
		if [ -d /sys/class/net/$i ]; then
			if [ ! -z $cpu_map ]; then
				cpu_map=`printf '%x' $cpu_map`
				dbg "echo $cpu_map > /sys/class/net/$i/queues/rx-0/rps_cpus"
				echo $cpu_map > /sys/class/net/$i/queues/rx-0/rps_cpus
			elif [ ! -z $1 ]; then
				dbg "echo $1 > /sys/class/net/$i/queues/rx-0/rps_cpus"
				echo $1 > /sys/class/net/$i/queues/rx-0/rps_cpus
			fi
		fi
	done
}

set_smp_affinity()
{
	dbg2 "# Setup affinity of each physical irq."
	num=0
	while [ "$num" -lt "$NUM_OF_CPU" ];do
		eval smp_list=\$CPU${num}_AFFINITY
		for i in $smp_list; do
			cpu_bit=$((2 ** $num))
			virq=$(get_virtual_irq $i)
			dbg2 "irq p2v $i --> $virq"
			if [ ! -z $virq ] && [ -d /proc/irq/$virq ]; then
				dbg "echo $cpu_bit > /proc/irq/$virq/smp_affinity"
				echo $cpu_bit > /proc/irq/$virq/smp_affinity
			fi
		done
		num=`expr $num + 1`
	done
}

if [ "$1" = "dbg" ]; then
	DBG=1
elif [ "$1" = "dbg2" ]; then
	DBG=2
else
	DBG=0
fi

# Usage: dbg "the output string"
dbg()
{
	if [ "$DBG" -ge "1" ]; then
		echo -e $1
	fi
}

# Usage: dbg2 "the output string"
dbg2()
{
	if [ "$DBG" -ge "2" ]; then
		echo -e $1
	fi
}

#NUM_OF_CPU=2	#7622
#NUM_OF_CPU=4	#7623
dbg "# RPS and AFFINITY Setting"
dbg "# NUM_OF_CPU=$NUM_OF_CPU"
VAR_PREFIX="autogen"
#IRQ_LIST=""	# setup by every model
RPS_IF_LIST=""	# setup by getEthIfName/getWiFiIfName/every model
get_eth_if_name
get_wifi_if_name	# It will add all wifi interfaces into $RPS_IF_LIST
dbg2 "# default RPS_IF_LIST=$RPS_IF_LIST"
IS_USBNET=0
scan_usbnet
setup_model
set_rps_cpu_bitmap
set_rps_cpus $DEFAULT_RPS
set_smp_affinity
#end of file
