################################################################
# Platform Related definitions
################################################################
ifeq ($(PLATFORM),RMI)
	WFLAGS += -DRT_BIG_ENDIAN
endif

ifeq ($(PLATFORM),UBICOM_IPX8)
	WFLAGS += -DRT_BIG_ENDIAN -DUNALIGNMENT_SUPPORT -DPLATFORM_UBM_IPX8 -DNO_CONSISTENT_MEM_SUPPORT -DCACHE_LINE_32B
endif

ifeq ($(PLATFORM),BL2348)
	WFLAGS += -DRT_BIG_ENDIAN
endif

ifeq ($(PLATFORM),BL23570)
	WFLAGS += -DRT_BIG_ENDIAN
endif

ifeq ($(PLATFORM),BLUBB)
	WFLAGS += -DRT_BIG_ENDIAN
endif

ifeq ($(PLATFORM),BLPMP)
	WFLAGS += -DRT_BIG_ENDIAN
endif

ifeq ($(PLATFORM),RMI_64)
	WFLAGS += -DRT_BIG_ENDIAN
endif

ifeq ($(PLATFORM),IXP)
	WFLAGS += -DRT_BIG_ENDIAN
endif

ifeq ($(PLATFORM),IKANOS_V160)
	WFLAGS += -DRT_BIG_ENDIAN -DIKANOS_VX_1X0
endif

ifeq ($(PLATFORM),IKANOS_V180)
	WFLAGS += -DRT_BIG_ENDIAN -DIKANOS_VX_1X0
endif

ifeq ($(PLATFORM),INF_TWINPASS)
	WFLAGS += -DRT_BIG_ENDIAN -DINF_TWINPASS
endif

ifeq ($(PLATFORM),INF_DANUBE)
	ifneq (,$(findstring 2.4,$(LINUX_SRC)))
	# Linux 2.4
	WFLAGS += -DINF_DANUBE -DRT_BIG_ENDIAN
	else
	# Linux 2.6
	WFLAGS += -DRT_BIG_ENDIAN
	endif
endif

ifeq ($(PLATFORM),INF_AR9)
	WFLAGS += -DRT_BIG_ENDIAN -DINF_AR9
	# support MAPI function for AR9.
	#WFLAGS += -DAR9_MAPI_SUPPORT
endif

ifeq ($(PLATFORM),INF_VR9)
	WFLAGS += -DRT_BIG_ENDIAN -DINF_AR9 -DINF_VR9
endif

ifeq ($(PLATFORM),CAVM_OCTEON)
	WFLAGS += -DRT_BIG_ENDIAN
endif

ifeq ($(PLATFORM),BRCM_6358)
	WFLAGS += -DRT_BIG_ENDIAN -DBRCM_6358
endif

ifeq ($(PLATFORM),FREESCALE8377)
	#EXTRA_CFLAGS := -v -I$(RT28xx_DIR)/include -I$(LINUX_SRC)/include $(WFLAGS)-O2 -Wall -Wstrict-prototypes -Wno-trigraphs
	#export EXTRA_CFLAGS
	WFLAGS += -DRT_BIG_ENDIAN
	EXTRA_CFLAGS := $(WFLAGS) -I$(RT28xx_DIR)/include
endif

ifeq ($(PLATFORM),ST)
	WFLAGS += -DST
endif

ifeq ($(PLATFORM),STAR)
	# build 2.4 move to Makefile outside LINUX_SRC := /opt/star/kernel/linux-2.4.27-star
	CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing -fno-common -Uarm -fno-common -pipe -mapcs-32 -D__LINUX_ARM_ARCH__=4 -march=armv4  -mshort-load-bytes -msoft-float -Uarm -DMODULE -DMODVERSIONS -include $(LINUX_SRC)/include/linux/modversions.h $(WFLAGS)
	export CFLAGS
endif

ifeq ($(PLATFORM),UBICOM_IPX8)
	EXTRA_CFLAGS += $(WFLAGS)
	export EXTRA_CFLAGS
endif

ifeq ($(PLATFORM),SIGMA)
	CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm/gcc -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -DEM86XX_CHIP=EM86XX_CHIPID_TANGO2 -DEM86XX_REVISION=6 -I$(LINUX_SRC)/include/asm-mips/mach-generic -I$(RT2860_DIR)/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O2     -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe  -mabi=32 -march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -DMODULE $(WFLAGS) -DSIGMA863X_PLATFORM
	export CFLAGS
endif

ifeq ($(PLATFORM),SIGMA_8622)
	CFLAGS := -D__KERNEL__ -I$(CROSS_COMPILE_INCLUDE)/include -I$(LINUX_SRC)/include -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing -fno-common -fno-common -pipe -fno-builtin -D__linux__ -DNO_MM -mapcs-32 -march=armv4 -mtune=arm7tdmi -msoft-float -DMODULE -mshort-load-bytes -nostdinc -iwithprefix -DMODULE $(WFLAGS)
	export CFLAGS
endif

ifeq ($(PLATFORM),IKANOS_V160)
	CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm/gcc -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-generic -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O2 -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe -march=lx4189 -Wa, -DMODULE $(WFLAGS)
	export CFLAGS
endif

ifeq ($(PLATFORM),IKANOS_V180)
	CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm/gcc -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-generic -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O2 -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe -mips32r2 -Wa, -DMODULE $(WFLAGS)
	export CFLAGS
endif

ifeq ($(PLATFORM),INF_TWINPASS)
	CFLAGS := -D__KERNEL__ -DMODULE -I$(LINUX_SRC)/include -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -fomit-frame-pointer -fno-strict-aliasing -fno-common -G 0 -mno-abicalls -fno-pic -march=4kc -mips32 -Wa,--trap -pipe -mlong-calls $(WFLAGS)
	export CFLAGS
endif

ifeq ($(PLATFORM),INF_DANUBE)
	ifneq (,$(findstring 2.4,$(LINUX_SRC)))
	CFLAGS := $(WFLAGS) -Wundef -fno-strict-aliasing -fno-common -ffreestanding -Os -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe -msoft-float  -mabi=32 -march=mips32 -Wa,-32 -Wa,-march=mips32 -Wa,-mips32 -Wa,--trap -I$(LINUX_SRC)/include/asm-mips/mach-generic
	else
	CFLAGS := $(WFLAGS) -Wundef -fno-strict-aliasing -fno-common -ffreestanding -Os -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe -msoft-float  -mabi=32 -march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -I$(LINUX_SRC)/include/asm-mips/mach-generic
	endif
	export CFLAGS
endif

ifeq ($(PLATFORM),INF_AR9)
	CFLAGS := $(WFLAGS) -Wundef -fno-strict-aliasing -fno-common -fno-pic -ffreestanding -Os -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe -msoft-float  -mabi=32 -mlong-calls -march=mips32r2 -mtune=34kc -march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -I$(LINUX_SRC)/include/asm-mips/mach-generic
	export CFLAGS
endif

ifeq ($(PLATFORM),INF_VR9)
	CFLAGS := $(WFLAGS) -Wundef -fno-strict-aliasing -fno-common -fno-pic -ffreestanding -Os -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe -msoft-float  -mabi=32 -mlong-calls -march=mips32r2 -march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -I$(LINUX_SRC)/include/asm-mips/mach-generic
	export CFLAGS
endif

ifeq ($(PLATFORM),BRCM_6358)
	CFLAGS := $(WFLAGS) -nostdinc -iwithprefix include -D__KERNEL__ -Wall -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -I $(LINUX_SRC)/include/asm/gcc -G 0 -mno-abicalls -fno-pic -pipe  -finline-limit=100000 -mabi=32 -march=mips32 -Wa,-32 -Wa,-march=mips32 -Wa,-mips32 -Wa,--trap -I$(LINUX_SRC)/include/asm-mips/mach-bcm963xx -I$(LINUX_SRC)/include/asm-mips/mach-generic  -Os -fomit-frame-pointer -Wdeclaration-after-statement  -DMODULE -mlong-calls
	export CFLAGS
endif

ifeq ($(PLATFORM),ST)
	CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -Wall -O2 -Wundef -Wstrict-prototypes -Wno-trigraphs -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-aliasing -fno-common -fomit-frame-pointer -ffreestanding -m4-nofpu -o $(WFLAGS)
	export CFLAGS
endif

ifeq ($(PLATFORM),PC)
	# Linux 2.6
	EXTRA_CFLAGS := $(WFLAGS)
endif

ifeq ($(PLATFORM),INTELP6)
	EXTRA_CFLAGS := $(WFLAGS)
endif

ifeq ($(PLATFORM),RMI)
	#If the kernel version of RMI is newer than 2.6.27, please change "CFLAGS" to "EXTRA_FLAGS"
	EXTRA_CFLAGS := -D__KERNEL__ -DMODULE=1 -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm-mips/mach-generic -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing -fno-common -DCONFIG_IFX_ALG_QOS -DCONFIG_WAN_VLAN_SUPPORT -fomit-frame-pointer -DIFX_PPPOE_FRAME -G 0 -fno-pic -mno-abicalls -mlong-calls -pipe -finline-limit=100000 -mabi=32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -march=xlr -ffreestanding  -march=xlr -Wa,--trap, -nostdinc -iwithprefix include $(WFLAGS)
	export EXTRA_CFLAGS
endif

ifeq ($(PLATFORM),RMI_64)
	EXTRA_CFLAGS := -D__KERNEL__ -DMODULE=1 -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm-mips/mach-generic -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing -fno-common -DCONFIG_IFX_ALG_QOS -DCONFIG_WAN_VLAN_SUPPORT -fomit-frame-pointer -DIFX_PPPOE_FRAME -G 0 -fno-pic -mno-abicalls -mlong-calls -pipe -finline-limit=100000 -mabi=64 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -march=xlr -ffreestanding  -march=xlr -Wa,--trap, -nostdinc -iwithprefix include $(WFLAGS)
	export EXTRA_CFLAGS
endif

ifeq ($(PLATFORM),IXP)
	CFLAGS := -v -D__KERNEL__ -DMODULE -I$(LINUX_SRC)/include -mbig-endian -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing -fno-common -Uarm -fno-common -pipe -mapcs-32 -D__LINUX_ARM_ARCH__=5 -mcpu=xscale -mtune=xscale -malignment-traps -msoft-float $(WFLAGS)
        EXTRA_CFLAGS := -v $(WFLAGS) -mbig-endian
	export CFLAGS
endif

ifeq ($(PLATFORM),SMDK)
	EXTRA_CFLAGS := $(WFLAGS)
endif

ifeq ($(PLATFORM),CAVM_OCTEON)
	EXTRA_CFLAGS := $(WFLAGS) -mabi=64 $(WFLAGS)
	export CFLAGS
endif

ifeq ($(PLATFORM),BB_SOC)
	WFLAGS += -DRT_BIG_ENDIAN -DBB_SOC -DWIFI_MODULE -DCONFIG_RA_NAT_NONE
	ifeq ($(CONFIG_MIPS_TC3262),y)
	WFLAGS += -DMULTI_CORE_SUPPORT
	endif
	ifeq ($(RALINK_PCIE_ADDR_SWAP),1)
	WFLAGS += -DBB_PCIE_ADDR_SWAP
	endif
	ifeq ($(TCSUPPORT_HWNAT_WIFI),1)
	WFLAGS += -DBB_RA_HWNAT_WIFI
	endif
	EXTRA_CFLAGS += -I$(RT28xx_DIR)/include $(WFLAGS)
	export EXTRA_CFLAGS
endif

ifeq ($(PLATFORM),DM6446)
	CFLAGS := -nostdinc -iwithprefix include -D__KERNEL__ -I$(LINUX_SRC)/include  -Wall -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Os -fno-omit-frame-pointer -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mlittle-endian -mabi=apcs-gnu -D__LINUX_ARM_ARCH__=5 -march=armv5te -mtune=arm9tdmi -msoft-float -Uarm -Wdeclaration-after-statement -c -o $(WFLAGS)
	export CFLAGS
endif

ifeq ($(PLATFORM),BL2348)
	CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm/gcc -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -DEM86XX_CHIP=EM86XX_CHIPID_TANGO2 -DEM86XX_REVISION=6 -I$(LINUX_SRC)/include/asm-mips/mach-generic -I$(RT2860_DIR)/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O2     -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe  -mabi=32 -march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -DMODULE $(WFLAGS) -DSIGMA863X_PLATFORM -DEXPORT_SYMTAB -DPLATFORM_BL2348
	export CFLAGS
endif

ifeq ($(PLATFORM),BL23570)
	EXTRA_CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm/gcc -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -DEM86XX_CHIP=EM86XX_CHIPID_TANGO2 -DEM86XX_REVISION=6 -I$(LINUX_SRC)/include/asm-mips/mach-generic -I$(RT2860_DIR)/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O2     -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe  -mabi=32 -march=74kc -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -DMODULE $(WFLAGS) -DSIGMA863X_PLATFORM -DEXPORT_SYMTAB -DPLATFORM_BL23570
	export EXTRA_CFLAGS
endif

ifeq ($(PLATFORM),BLUBB)
	CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm/gcc -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -DEM86XX_CHIP=EM86XX_CHIPID_TANGO2 -DEM86XX_REVISION=6 -I$(LINUX_SRC)/include/asm-mips/mach-generic -I$(RT2860_DIR)/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O2     -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe  -mabi=32 -march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -DMODULE $(WFLAGS) -DSIGMA863X_PLATFORM -DEXPORT_SYMTAB -DPLATFORM_BL2348
	export CFLAGS
endif

ifeq ($(PLATFORM),BLPMP)
	CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -I$(LINUX_SRC)/include/asm/gcc -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -I$(LINUX_SRC)/include/asm-mips/mach-tango2 -DEM86XX_CHIP=EM86XX_CHIPID_TANGO2 -DEM86XX_REVISION=6 -I$(LINUX_SRC)/include/asm-mips/mach-generic -I$(RT2860_DIR)/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O2     -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe  -mabi=32 -march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -DMODULE $(WFLAGS) -DSIGMA863X_PLATFORM -DEXPORT_SYMTAB
	export CFLAGS
endif

ifeq ($(PLATFORM),MT85XX)
	ifneq (,$(findstring 2.4,$(LINUX_SRC)))
	# Linux 2.4
	CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -O2 -fomit-frame-pointer -fno-strict-aliasing -fno-common -pipe -mpreferred-stack-boundary=2 -march=i686 -DMODULE -DMODVERSIONS -include $(LINUX_SRC)/include/linux/modversions.h $(WFLAGS)
	export CFLAGS
	else
	# Linux 2.6
	EXTRA_CFLAGS += $(WFLAGS)
	EXTRA_CFLAGS += -D _NO_TYPEDEF_BOOL_ \
	                -D _NO_TYPEDEF_UCHAR_ \
	                -D _NO_TYPEDEF_UINT8_ \
	                -D _NO_TYPEDEF_UINT16_ \
	                -D _NO_TYPEDEF_UINT32_ \
	                -D _NO_TYPEDEF_UINT64_ \
	                -D _NO_TYPEDEF_CHAR_ \
	                -D _NO_TYPEDEF_INT32_ \
	                -D _NO_TYPEDEF_INT64_
    endif
endif

ifeq ($(PLATFORM),NXP_TV550)
    ifneq (,$(findstring 2.4,$(LINUX_SRC)))
        # Linux 2.4
        CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -O2 -fomit-frame-pointer -fno-strict-aliasing -fno-common -pipe -mpreferred-stack-boundary=2 -march=mips -DMODULE -DMODVERSIONS -include $(LINUX_SRC)/include/linux/modversions.h $(WFLAGS)
        export CFLAGS
    else
        # Linux 2.6
        EXTRA_CFLAGS := $(WFLAGS)
    endif
endif

ifeq ($(PLATFORM),MVL5)
CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -O3 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mno-thumb-interwork -D__LINUX_ARM_ARCH__=5 -march=armv5te -mtune=arm926ej-s --param max-inline-insns-single=40000  -Uarm -Wdeclaration-after-statement -Wno-pointer-sign -DMODULE $(WFLAGS)
export CFLAGS
endif

ifeq ($(PLATFORM),MT7620)
        EXTRA_CFLAGS := $(WFLAGS)
endif

ifeq ($(PLATFORM),MT7621)
        EXTRA_CFLAGS := $(WFLAGS)
endif

ifeq ($(PLATFORM),MT7628)
        EXTRA_CFLAGS := $(WFLAGS)
endif

ifeq ($(PLATFORM),MT7622)
        EXTRA_CFLAGS := $(WFLAGS)
endif

ifeq ($(PLATFORM),MT6890)
        WFLAGS += -DMT6890
        EXTRA_CFLAGS := $(WFLAGS)
endif

ifeq ($(PLATFORM),MT6990)
        WFLAGS += -DMT6990
        EXTRA_CFLAGS := $(WFLAGS)
endif
