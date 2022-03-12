
#!/bin/bash 

TIME=`date +\%F-\%T`
echo 
echo "######## ${TIME} ########"
echo 

# COMBINATION: specify wifi driver build parameter
#	 CHIP	  PLATFORM  WIFI_MODE	OS
declare -a COMBINATION=(
	"mt7622-mt7615	PC			AP	LINUX"
#	"mt7622-mt7915	PC			AP	LINUX"
	"mt7915 	PC                      AP      LINUX"
	"mt7622					MT7622	AP	LINUX"
	"mt7663e				PC			AP	LINUX"
	"mt7663u				PC			AP	LINUX"
	"soc1_0					PC			AP	LINUX"
	"mt7615					PC			STA	LINUX"
	"soc1_0					PC			STA	LINUX"
	"mt7663e				PC			STA	LINUX"
	"mt7663u				PC			STA	LINUX"
	"mt7915					PC			STA	LINUX"
	)

# PATH: for checking build result .ko file
RESULT_PATH="../os/linux/"

# MAIN
for combination in "${COMBINATION[@]}"
do
	# parse build combination
	comb_arr=($combination)
	chip=${comb_arr[0]}
	platform=${comb_arr[1]}
	wifi_mode=${comb_arr[2]}
	os=${comb_arr[3]}

	# compose .ko file name
	driver_name=$chip\_$wifi_mode
	driver_name=${driver_name,,} # cast the string to lower-case

	echo "build $chip $wifi_mode mode on platform:$platform and OS:$os"

	# clean
	make -f Makefile clean

	# make
	make -f Makefile CHIPSET=$chip PLATFORM=$platform WIFI_MODE=$wifi_mode TARGET=$os CCACHE=1 -j8

	# check 
	#   make return code
	#   check ${driver_name}.ko exist
	if [ -f "${RESULT_PATH}${driver_name}.ko" ]; then
		echo "${driver_name} built successfully."
	else
		echo "${driver_name} built FAILED (ko not found)!"
		exit 1
	fi
done

TIME=`date +\%F-\%T`
echo 
echo "######## ${TIME} ########"
echo 
