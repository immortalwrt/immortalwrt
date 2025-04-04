#!/bin/sh

# Release.sh can be run with some parameters.
# sh Release.sh $WiFiMode $ChipName $OS_TARGET $BuildMode
# Ex: sh Release.sh AP mt7663 LINUX auto_build

if [ "${1}" == "AP" ]; then
    WiFiMode=AP
elif [ "${1}" == "STA" ]; then
    WiFiMode=STA
elif [ "${1}" == "APSTA" ]; then
    WiFiMode=APSTA
else
    echo "Driver Release: wifi mode is not specified, default wifi mode is AP"
    WiFiMode=AP
fi

if [ "${2}" == "mt7663" -o "${2}" == "MT7663" ]; then
    ChipName=mt7663
elif [ "${2}" == "mt7615" -o "${2}" == "MT7615" ]; then
    ChipName=mt7615
elif [ "${2}" == "mt7626" -o "${2}" == "MT7626" ]; then
    ChipName=mt7626
elif [ "${2}" == "mt7629" -o "${2}" == "MT7629" ]; then
    ChipName=mt7629
else
    echo "Driver Release: wifi chipset is not specified, default wifi chipset is mt7663"
    ChipName=mt7663
fi

if [ "${3}" == "vxworks" -o "${3}" == "VXWORKS" ]; then
    echo "Driver Release: specify the OS is vxworks"
    OS_TARGET=VXWORKS
else
    OS_TARGET=LINUX
fi

if [ "${4}" == "auto_build" ]; then
    BuildMode="auto_build"
else
    BuildMode=""
fi

OSABL=NO
EmbeddedDir=`pwd`
WiFiDir=../
BaseCodeDir=`dirname $EmbeddedDir`
BinDir=`dirname $BaseCodeDir`/bin
#DriverVersion="V4.1.0.0"
DriverVersion=`grep AP_DRIVER_VERSION ${WiFiDir}/include/os/rt_linux.h | head -1 | sed -e 's/.*\t//g; s/"//g'`
Release="DPA"
#Note=$1
Description="Formal release."
release_profile=Release.log
###### Fixed Settings ######
Day=`date +%d`
Month=`date +%m`
Year=`date +%Y`
Date=$Year$Month$Day
Hour=`date +%H`
Minute=`date +%M`
HomeDir=`dirname $BaseCodeDir`
#ModuleName=mt$ChipName\_wifi_$DriverVersion
ModulePrefix=`echo $ChipName | tr '[:lower:]' '[:upper:]'`_LinuxAP
ModuleName=$ModulePrefix\_$DriverVersion
WorkDir=$HomeDir/release
TargetDir=$WorkDir/$ModuleName\_$Date\_$Hour$Minute
############################


if [ ! -d $WorkDir ]; then
    mkdir $WorkDir
fi

BaseCodeDir=`readlink -f $BaseCodeDir` #get full path
if [ ! -d $BaseCodeDir ] ; then
    echo "Error: BaseCodeDir ($BaseCodeDir) does not exist."
	exit 1;
fi

cp -a $BaseCodeDir $TargetDir

#rm/copy the bin folder
if [ -d $WorkDir/bin ] ; then
    echo "bin folder ($WorkDir/bin) exist, del first!."
    rm -rf $WorkDir/bin
fi

cp -a $BinDir $WorkDir

if [ ! -d $TargetDir ] ; then
    echo "Error: TargetDir ($TargetDir) does not exist."
    exit 1;
fi

cd $TargetDir

if [ -f /home/haipin/script/chmod_644.sh ]; then
	/home/haipin/script/chmod_644.sh
fi

if [ -f /home/haipin/script/clean.sh ]; then
	/home/haipin/script/clean.sh
fi

for i in $Release; do
	cd $TargetDir/embedded
	####### Remove unwanted files that do not processed by strip tool ####### // TODO
	rm os/linux/Makefile.libautoprovision.6 common/rt2860.bin common/rt2870_sw_ch_offload.bin common/RT85592.bin
	rm tools/mt7662e_ap.sh tools/mt7662e_sta.sh tools/i.sh tools/trace.sh
	rm ../eeprom/MT7601*.bin ../eeprom/MT7603*.bin ../eeprom/MT7628*.bin ../eeprom/MT7637*.bin ../eeprom/MT7662*.bin
	rm ../eeprom/MT7636*.bin
	rm ../mcu/bin/*_FPGA*.bin ../mcu/bin/*_plain*.bin ../mcu/bin/*_test*.bin
	rm ../mcu/bin/*7601* ../mcu/bin/*7603* ../mcu/bin/*7612* ../mcu/bin/*7636* ../mcu/bin/*7637*
	rm ../mcu/bin/*7610* ../mcu/bin/*7650* ../mcu/bin/*7662* ../mcu/bin/*7628*
	rm ../os/linux/Kconfig.ap* ../os/linux/Kconfig.sta* ../os/linux/Kconfig.rlt* ../os/linux/Kconfig.wifi
	rm ../os/linux/Makefile.ap* ../os/linux/Makefile.sta* ../os/linux/Makefile.2880* ../os/linux/Makefile.rlt*
	#########################################################################

	## Regenerate Firmware ##
	#rm -rf include/mcu/mt7628_firmware.h
	#rm -rf include/mcu/mt7628_e2_firmware.h
	if [ $ChipName == "mt7629" ]; then
		make build_tools CHIPSET=mt7626
	else
		make build_tools CHIPSET=$ChipName
	fi
	## Regenerate SKU tables ##
	#make build_sku_tables CHIPSET=$ChipName

if [ "$BuildMode" != "auto_build" ]; then
	make release WIFI_MODE=$WiFiMode CHIPSET=$ChipName TARGET=$OS_TARGET RELEASE=$i
else
	make release WIFI_MODE=$WiFiMode CHIPSET=$ChipName TARGET=$OS_TARGET RELEASE=$i AUTO_BUILD=y
fi

	if [ -d $i ]; then
		lower=`echo $i | tr A-Z a-z`  #transfer "DPA DPB DPC DPD" to lower case
    	#ReleaseDir=$ModuleName\_$lower\_$Date
    	ReleaseDir=$ModuleName\_$i\_$Date
    	#ReleaseDir=$ModuleName\_$Date
		#ReleaseDir=$ModulePrefix
    	mv $i $TargetDir/$ReleaseDir
		ReleaseDir=$TargetDir/$ReleaseDir

		#if [ "$i" == "DPA" ]; then
			cd $ReleaseDir/embedded

			find . -name "*Card.dat" -exec rm -rf {} \; #// TODO
			find . -name "*2870*.dat" -exec rm -rf {} \;
			rm -rf doc/*History*.txt doc/*README* doc/RT_WIFI_Revision_History_2010_April.xls doc/RT2860card.readme
			rm -rf Makefile.OSABL threadX.MakeFile vxworks.makefile History.txt RT2860AP.dat
			rm -rf os/linux/Makefile.2880.*
			rm -rf Release.sh load.4 load.6 load unload
			#rm -rf Makefile
			cd $TargetDir/embedded
		#fi
		### Generate profile for release ###
		echo $ModulePrefix > $ReleaseDir/$release_profile
		echo $DriverVersion >> $ReleaseDir/$release_profile
		echo $Date >> $ReleaseDir/$release_profile
		cp Pack_Release.sh $ReleaseDir/embedded
		##Copy sku_tables .dat to release package##
		#cp -R $TargetDir/txpwr/sku_tables $ReleaseDir/txpwr/
		cd $ReleaseDir/embedded
		sh Pack_Release.sh $ChipName $BuildMode

    	#tar -jcvf $ReleaseDir.tar.bz2 $ReleaseDir
	else
		echo "Error: ReleaseDir ($i) does not exist"
		exit 1
	fi

done

#####Log File#####
#if [ "$1" != "auto_build" ]; then
#    cd $TargetDir/embedded
#    echo "OSABL = $OSABL" > Release.log
#    echo "BaseCodeDir = $BaseCodeDir" >> Release.log
#    echo "ChipName = $ChipName" >> Release.log
#    echo "DriverVersion = $DriverVersion" >> Release.log
    #echo "BuildDir = $BuildDir" >> Release.log
#    echo "Release = $Release" >> Release.log
#    if [ "$Description" != "" ]; then
#        echo "Description = $Description" >> Release.log
#    fi
#    echo "Note: $Note" >> Release.log
#fi
##################
