/*
 * Copyright (c) [2020], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/
/*
 ***************************************************************************
 ***************************************************************************

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define PATH_OF_MCU_BIN_IN "/bin/"
#define PATH_OF_MCU_BIN_OUT "/embedded/include/mcu/"
#define PATH_OF_EEPROM_IN "/bin/"
#define PATH_OF_EEPROM_OUT "/embedded/include/eeprom/"
#define PATH_OF_ROM_PATCH_IN "/bin/"
#define PATH_OF_ROM_PATCH_OUT "/embedded/include/mcu/"


int bin2h(char *infname, char *outfname, char *fw_name, const char *mode)
{
	int ret = 0;
	FILE *infile, *outfile;
	unsigned char c;
	int i = 0;

	infile = fopen(infname, "r");

	if (infile == (FILE *) NULL) {
		printf("Can't read file %s\n", infname);
		return -1;
	}

	outfile = fopen(outfname, mode);

	if (outfile == (FILE *) NULL) {
		printf("Can't open write file %s\n", outfname);
		fclose(infile);
		return -1;
	}

	fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */\n", outfile);
	fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */\n", outfile);
	fputs("\n", outfile);
	fputs("\n", outfile);
	fprintf(outfile, "UCHAR %s[] = {\n", fw_name);

	while (1) {
		char cc[3];

		c = getc(infile);

		if (feof(infile))
			break;

		memset(cc, 0, 2);

		if (i >= 16) {
			fputs("\n", outfile);
			i = 0;
		}

		if (i == 0)
			fputs("\t", outfile);
		else if (i < 16)
			fputs(" ", outfile);

		fputs("0x", outfile);
		sprintf(cc, "%02x", c);
		fputs(cc, outfile);
		fputs(",", outfile);
		i++;
	}

	fputs("\n};\n", outfile);
	fclose(infile);
	fclose(outfile);

	return ret;
}

int main(int argc, char *argv[])
{
	char input_fw_path[512];
	char infname[512], ine2pname[512], in_rom_patch[512], in_rom_patch_e2[512], in_rom_patch_e3[512], in_emi_ram_ilm[512], in_emi_ram_dlm[512];
	char infname1[512], infname_e2[512], infname_e3[512];
	char outfname[512], oute2pname[512], out_rom_patch[512], out_rom_patch_e2[512], out_rom_patch_e3[512], out_emi_ram[512];
	char outfname1[512], outfname_e2[512], outfname_e3[512];
	char chipsets[1024];
	char fpga_mode[20], rx_cut_through_mode[20];
	char fw_name[128], e2p_name[128], rom_patch_name[128], rom_patch_name_e2[128], rom_patch_name_e3[128], emi_ram_ilm_name[128], emi_ram_dlm_name[128];
	char fw_name_e2[128], fw_name_e3[128];
	char fw_name1[128];
	char *rt28xxdir, *rt28xxbin_dir;
	char *chipset, *token;
	char *wow, *rt28xx_mode;
	char *fpga, *rx_cut_through;
	int is_bin2h_fw = 0, is_bin2h_rom_patch = 0, is_bin2h_e2p = 0, is_bin2h_rom_patch_e2 = 0, is_bin2h_rom_patch_e3 = 0, is_b2h_emi_ram = 0;
	char ine2pname2[512], ine2pname3[512], e2p_name2[128], e2p_name3[128];
	char *adie;
	char *sku;

	rt28xxdir = (char *)getenv("RT28xx_DIR");
	rt28xxbin_dir = (char *)getenv("RT28xx_BIN_DIR");
	chipset = (char *)getenv("CHIPSET");
	adie = (char *)getenv("ADIE");
	sku = (char *)getenv("SKU");
	wow = (char *)getenv("HAS_WOW_SUPPORT");
	fpga = (char *)getenv("HAS_FPGA_MODE");
	rx_cut_through = (char *)getenv("HAS_RX_CUT_THROUGH");
	rt28xx_mode = (char *)getenv("RT28xx_MODE");

	if (!rt28xxdir) {
		printf("Environment value \"RT28xx_DIR\" not export\n");
		return -1;
	}

	if (!rt28xxbin_dir) {
		printf("Environment value \"RT28xx_BIN_DIR\" not export\n");
		return -1;
	}

	if (!chipset) {
		printf("Environment value \"CHIPSET\" not export\n");
		return -1;
	}

	if (rt28xx_mode)
		printf("Build %s %s\n", chipset, rt28xx_mode);

	memset(chipsets, 0, sizeof(chipsets));
	memcpy(chipsets, chipset, strlen(chipset));
	chipsets[strlen(chipset)] = '\0';

	if (!fpga) {
		printf("Environment value \"HAS_FPGA_MODE\" not export\n");
		return -1;
	}

	if (strlen(fpga) > (sizeof(fpga_mode) - 1)) {
		printf("Environment value \"HAS_FPGA_MODE\" is too long, need less than 20\n");
		return -1;
	}

	memset(fpga_mode, 0, sizeof(fpga_mode));
	memcpy(fpga_mode, fpga, strlen(fpga));
	fpga_mode[strlen(fpga)] = '\0';

	if (!rx_cut_through) {
		printf("Environment value \"HAS_RX_CUT_THROUGH\" not export\n");
		return -1;
	}

	if (strlen(rx_cut_through) > (sizeof(rx_cut_through_mode) - 1)) {
		printf("Environment value \"HAS_RX_CUT_THROUGH\" is too long, need less than 20\n");
		return -1;
	}

	memcpy(rx_cut_through_mode, rx_cut_through, strlen(rx_cut_through));
	rx_cut_through_mode[strlen(rx_cut_through)] = '\0';

	if (strlen(rt28xxdir) > (sizeof(infname) - 100)) {
		printf("Environment value \"RT28xx_DIR\" is too long!\n");
		return -1;
	}

	if (strlen(rt28xxbin_dir) > (sizeof(infname) - 100)) {
		printf("Environment value \"RT28xx_BIN_DIR\" is too long!\n");
		return -1;
	}

	chipsets[strlen(chipset)] = '\0';
	chipset = strtok(chipsets, "-, ");

	while (chipset != NULL) {
		printf("chipset = %s\n", chipset);
		memset(input_fw_path, 0, 512);
		memset(infname, 0, 512);
		memset(infname_e2, 0, 512);
		memset(infname_e3, 0, 512);
		memset(infname1, 0, 512);
		memset(ine2pname, 0, 512);
		memset(ine2pname2, 0, 512);
		memset(ine2pname3, 0, 512);
		memset(outfname, 0, 512);
		memset(outfname_e2, 0, 512);
		memset(outfname_e3, 0, 512);
		memset(outfname1, 0, 512);
		memset(oute2pname, 0, 512);
		memset(fw_name, 0, 128);
		memset(fw_name_e2, 0, 128);
		memset(fw_name_e3, 0, 128);
		memset(fw_name1, 0, 128);
		memset(e2p_name, 0, 128);
		memset(e2p_name2, 0, 128);
		memset(e2p_name3, 0, 128);
		memset(in_rom_patch, 0, 512);
		memset(in_rom_patch_e2, 0, 512);
		memset(in_rom_patch_e3, 0, 512);
		memset(out_rom_patch, 0, 512);
		memset(out_rom_patch_e2, 0, 512);
		memset(out_rom_patch_e3, 0, 512);
		memset(rom_patch_name, 0, 128);
		memset(rom_patch_name_e2, 0, 128);
		memset(rom_patch_name_e3, 0, 128);
		memset(in_emi_ram_ilm, 0, 512);
		memset(in_emi_ram_dlm, 0, 512);
		memset(out_emi_ram, 0, 512);
		memset(emi_ram_ilm_name, 0, 128);
		memset(emi_ram_dlm_name, 0, 128);

		strcat(input_fw_path, rt28xxbin_dir);
		strcat(input_fw_path, PATH_OF_MCU_BIN_IN);
		strcat(input_fw_path, chipset);
		strcat(input_fw_path, "/");
		
		strcat(infname, rt28xxbin_dir);
		strcat(infname_e2, rt28xxbin_dir);
		strcat(infname_e3, rt28xxbin_dir);
		strcat(infname1, rt28xxbin_dir);
		strcat(ine2pname, rt28xxbin_dir);
		strcat(in_rom_patch, rt28xxbin_dir);
		strcat(in_rom_patch_e2, rt28xxbin_dir);
		strcat(in_rom_patch_e3, rt28xxbin_dir);
		strcat(outfname, rt28xxdir);
		strcat(outfname_e2, rt28xxdir);
		strcat(outfname_e3, rt28xxdir);
		strcat(outfname1, rt28xxdir);
		strcat(oute2pname, rt28xxdir);
		strcat(out_rom_patch, rt28xxdir);
		strcat(out_rom_patch_e2, rt28xxdir);
		strcat(out_rom_patch_e3, rt28xxdir);
		strcat(in_emi_ram_ilm, rt28xxbin_dir);
		strcat(in_emi_ram_dlm, rt28xxbin_dir);
		strcat(out_emi_ram, rt28xxdir);

		is_bin2h_fw = 0;
		is_bin2h_rom_patch = 0;
		is_bin2h_e2p = 0;
		is_b2h_emi_ram = 0;
		strcat(infname, PATH_OF_MCU_BIN_IN);
		strcat(infname_e2, PATH_OF_MCU_BIN_IN);
		strcat(infname_e3, PATH_OF_MCU_BIN_IN);
		strcat(outfname, PATH_OF_MCU_BIN_OUT);
		strcat(outfname_e2, PATH_OF_MCU_BIN_OUT);
		strcat(outfname_e3, PATH_OF_MCU_BIN_OUT);
		strcat(ine2pname, PATH_OF_EEPROM_IN);
		strcat(oute2pname, PATH_OF_EEPROM_OUT);
		strcat(in_rom_patch, PATH_OF_ROM_PATCH_IN);
		strcat(in_rom_patch_e2, PATH_OF_ROM_PATCH_IN);
		strcat(in_rom_patch_e3, PATH_OF_ROM_PATCH_IN);
		strcat(out_rom_patch, PATH_OF_ROM_PATCH_OUT);
		strcat(out_rom_patch_e2, PATH_OF_ROM_PATCH_OUT);
		strcat(out_rom_patch_e3, PATH_OF_ROM_PATCH_OUT);
		strcat(infname1, PATH_OF_MCU_BIN_IN);
		strcat(outfname1, PATH_OF_MCU_BIN_OUT);
		strcat(in_emi_ram_ilm, PATH_OF_MCU_BIN_IN);
		strcat(in_emi_ram_dlm, PATH_OF_MCU_BIN_IN);
		strcat(out_emi_ram, PATH_OF_MCU_BIN_OUT);

		/* For bin input, select chipset folder inside /bin */
		strcat(infname, chipset);
		strcat(infname_e2, chipset);
		strcat(infname_e3, chipset);
		strcat(infname1, chipset);
		strcat(ine2pname, chipset);
		strcat(in_rom_patch, chipset);
		strcat(in_rom_patch_e2, chipset);
		strcat(in_rom_patch_e3, chipset);
		strcat(in_emi_ram_ilm, chipset);
		strcat(in_emi_ram_dlm, chipset);

		strcat(infname, "/");
		strcat(infname_e2, "/");
		strcat(infname_e3, "/");
		strcat(infname1, "/");
		strcat(ine2pname, "/");
		strcat(in_rom_patch, "/");
		strcat(in_rom_patch_e2, "/");
		strcat(in_rom_patch_e3, "/");
		strcat(in_emi_ram_ilm, "/");
		strcat(in_emi_ram_dlm, "/");

		if ((strncmp(chipset, "mt7603e", 7) == 0)
				   || (strncmp(chipset, "mt7603u", 7) == 0)) {
			strcat(infname1, "WIFI_RAM_CODE_MT7603_e2.bin");
			strcat(outfname1, "mt7603_e2_firmware.h");
			strcat(fw_name1, "MT7603_e2_FirmwareImage");
			strcat(e2p_name, "MT7603_E2PImage");
			strcat(ine2pname, "MT7603E_EEPROM.bin");
			strcat(oute2pname, "mt7603_e2p.h");
			is_bin2h_fw = 1;
			is_bin2h_e2p = 1;
		} else if (strncmp(chipset, "mt7628", 7) == 0) {
			strcat(infname, "WIFI_RAM_CODE_MT7628_e1.bin");
			strcat(outfname, "mt7628_firmware.h");
			strcat(fw_name, "MT7628_FirmwareImage");
			strcat(e2p_name, "MT7628_E2PImage");
			strcat(ine2pname, "MT7603E1E2_EEPROM_layout_20140226.bin");
			strcat(oute2pname, "mt7628_e2p.h");
			is_bin2h_fw = 1;
			is_bin2h_e2p = 1;
		} else if (strncmp(chipset, "mt7615", 7) == 0) {
			strcat(in_rom_patch_e3, "mt7615_patch_e3_hdr.bin"); /* mt7615_patch_e3_hdr.bin */
			strcat(out_rom_patch_e3, "mt7615_rom_patch.h");
			strcat(rom_patch_name_e3, "mt7615_rom_patch");
			strcat(infname_e3, "WIFI_RAM_CODE_MT7615.bin");
			strcat(outfname_e3, "mt7615_firmware.h");
			strcat(fw_name_e3, "MT7615_FirmwareImage");

			if ((strncmp(rx_cut_through, "y", 1) == 0))
				strcat(infname1, "MT7615_cr4.bin");
			else
				strcat(infname1, "MT7615_cr4_noReOrdering.bin");

			strcat(outfname1, "mt7615_cr4_firmware.h");
			strcat(fw_name1, "MT7615_CR4_FirmwareImage");
			is_bin2h_rom_patch = 1;
			is_bin2h_rom_patch_e3 = 1;
			is_bin2h_fw = 1;
			/* iPAiLNA */
			strncpy(ine2pname2, ine2pname, 512 - 1);
			ine2pname2[511] = '\0';
			strncpy(ine2pname3, ine2pname, 512 - 1);
			ine2pname3[511] = '\0';
			strcat(e2p_name, "MT7615_E2PImage1_iPAiLNA");
			strcat(e2p_name2, "MT7615_E2PImage2_iPAiLNA");
			strcat(e2p_name3, "MT7615_E2PImage3_iPAiLNA");
			strcat(ine2pname, "iPAiLNA/MT7615_EEPROM1.bin");
			strcat(ine2pname2, "iPAiLNA/MT7615_EEPROM2.bin");
			strcat(ine2pname3, "iPAiLNA/MT7615_EEPROM3.bin");
			strcat(oute2pname, "mt7615_e2p_iPAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			bin2h(ine2pname2, oute2pname, e2p_name2, "a");
			bin2h(ine2pname3, oute2pname, e2p_name3, "a");
			/* iPAeLNA */
			memset(ine2pname, 0, 512);
			memset(ine2pname2, 0, 512);
			memset(ine2pname3, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			memset(e2p_name2, 0, 128);
			memset(e2p_name3, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strncpy(ine2pname2, ine2pname, 512 - 1);
			ine2pname2[511] = '\0';
			strncpy(ine2pname3, ine2pname, 512 - 1);
			ine2pname3[511] = '\0';
			strcat(e2p_name, "MT7615_E2PImage1_iPAeLNA");
			strcat(e2p_name2, "MT7615_E2PImage2_iPAeLNA");
			strcat(e2p_name3, "MT7615_E2PImage3_iPAeLNA");
			strcat(ine2pname, "iPAeLNA/MT7615_EEPROM1.bin");
			strcat(ine2pname2, "iPAeLNA/MT7615_EEPROM2.bin");
			strcat(ine2pname3, "iPAeLNA/MT7615_EEPROM3.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7615_e2p_iPAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			bin2h(ine2pname2, oute2pname, e2p_name2, "a");
			bin2h(ine2pname3, oute2pname, e2p_name3, "a");
			/* ePAeLNA */
			memset(ine2pname, 0, 512);
			memset(ine2pname2, 0, 512);
			memset(ine2pname3, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			memset(e2p_name2, 0, 128);
			memset(e2p_name3, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strncpy(ine2pname2, ine2pname, 512 - 1);
			ine2pname2[511] = '\0';
			strncpy(ine2pname3, ine2pname, 512 - 1);
			ine2pname3[511] = '\0';
			strcat(e2p_name, "MT7615_E2PImage1_ePAeLNA");
			strcat(e2p_name2, "MT7615_E2PImage2_ePAeLNA");
			strcat(e2p_name3, "MT7615_E2PImage3_ePAeLNA");
			strcat(ine2pname, "ePAeLNA/MT7615_EEPROM1.bin");
			strcat(ine2pname2, "ePAeLNA/MT7615_EEPROM2.bin");
			strcat(ine2pname3, "ePAeLNA/MT7615_EEPROM3.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7615_e2p_ePAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			bin2h(ine2pname2, oute2pname, e2p_name2, "a");
			bin2h(ine2pname3, oute2pname, e2p_name3, "a");
			/* ePAiLNA */
			memset(ine2pname, 0, 512);
			memset(ine2pname2, 0, 512);
			memset(ine2pname3, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			memset(e2p_name2, 0, 128);
			memset(e2p_name3, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strncpy(ine2pname2, ine2pname, 512 - 1);
			ine2pname2[511] = '\0';
			strncpy(ine2pname3, ine2pname, 512 - 1);
			ine2pname3[511] = '\0';
			strcat(e2p_name, "MT7615_E2PImage1_ePAiLNA");
			strcat(e2p_name2, "MT7615_E2PImage2_ePAiLNA");
			strcat(e2p_name3, "MT7615_E2PImage3_ePAiLNA");
			strcat(ine2pname, "ePAiLNA/MT7615_EEPROM1.bin");
			strcat(ine2pname2, "ePAiLNA/MT7615_EEPROM2.bin");
			strcat(ine2pname3, "ePAiLNA/MT7615_EEPROM3.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7615_e2p_ePAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			bin2h(ine2pname2, oute2pname, e2p_name2, "a");
			bin2h(ine2pname3, oute2pname, e2p_name3, "a");
			/* is_bin2h_e2p = 1;   //b2h is already done */
		} else if (strncmp(chipset, "mt7622", 7) == 0) {
			strcat(in_rom_patch_e2, "mt7622_patch_e2_hdr.bin");
			strcat(out_rom_patch_e2, "mt7622_rom_patch_e2.h");
			strcat(rom_patch_name_e2, "mt7622_rom_patch_e2");
			strcat(infname_e2, "WIFI_RAM_CODE_MT7622_E2.bin");

			strcat(outfname_e2, "mt7622_firmware_e2.h");
			strcat(fw_name_e2, "MT7622_FirmwareImage_E2");
			is_bin2h_fw = 1;
			is_bin2h_rom_patch = 1;
			is_bin2h_rom_patch_e2 = 1;

			/* iPAiLNA */
			strcat(e2p_name, "MT7622_E2PImage_iPAiLNA");
			strcat(ine2pname, "iPAiLNA/MT7622_EEPROM.bin");
			strcat(oute2pname, "mt7622_e2p_iPAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* iPAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7622_E2PImage_iPAeLNA");
			strcat(ine2pname, "iPAeLNA/MT7622_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7622_e2p_iPAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7622_E2PImage_ePAeLNA");
			strcat(ine2pname, "ePAeLNA/MT7622_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7622_e2p_ePAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
		} else if (strncmp(chipset, "p18", 7) == 0) {
			strcat(in_rom_patch, "p18_patch_e1_hdr.bin");
			strcat(out_rom_patch, "p18_rom_patch_e1.h");
			strcat(rom_patch_name, "p18_rom_patch_e1");

			if ((strncmp(fpga, "y", 1) == 0))
				strcat(infname, "rebb/WIFI_RAM_CODE_P18_FPGA.bin");
			else
				strcat(infname, "rebb/WIFI_RAM_CODE_P18.bin");

			strcat(outfname, "p18_firmware.h");
			strcat(fw_name, "P18_FirmwareImage");
			strcat(e2p_name, "P18_E2PImage");
			strcat(ine2pname, "rebb/P18_EEPROM.bin");
			strcat(oute2pname, "p18_e2p.h");
			is_bin2h_fw = 1;
			is_bin2h_e2p = 1;
			is_bin2h_rom_patch = 1;
                } else if (strncmp(chipset, "soc1_0", 7) == 0) {
                        strcat(in_rom_patch, "rebb/soc1_0_patch_e1_hdr.bin");
                        strcat(out_rom_patch, "p18_rom_patch_e1.h");
                        strcat(rom_patch_name, "p18_rom_patch_e1");

                        if ((strncmp(fpga, "y", 1) == 0))
                                strcat(infname, "rebb/WIFI_RAM_CODE_soc1_0_FPGA.bin");
                        else
                                strcat(infname, "rebb/WIFI_RAM_CODE_soc1_0.bin");

                        strcat(outfname, "p18_firmware.h");
                        strcat(fw_name, "P18_FirmwareImage");
                        strcat(e2p_name, "P18_E2PImage");
                        strcat(ine2pname, "rebb/soc1_0_EEPROM.bin");
                        strcat(oute2pname, "p18_e2p.h");
                        is_bin2h_fw = 1;
                        is_bin2h_e2p = 1;
                        is_bin2h_rom_patch = 1;
		} else if (strncmp(chipset, "mt7663", 7) == 0 || strncmp(chipset, "mt7663e", 7) == 0 || strncmp(chipset, "mt7663u", 7) == 0) {
			strcat(in_rom_patch, "rebb/mt7663_patch_e1_hdr.bin");
			strcat(out_rom_patch, "mt7663_rom_patch_e1.h");
			strcat(rom_patch_name, "mt7663_rom_patch_e1");
                        strcat(in_rom_patch_e2, "rebb/mt7663_patch_e2_hdr.bin");
                        strcat(out_rom_patch_e2, "mt7663_rom_patch_e2.h");
                        strcat(rom_patch_name_e2, "mt7663_rom_patch_e2");

			if ((strncmp(fpga, "y", 1) == 0))
				strcat(infname, "rebb/WIFI_RAM_CODE_MT7663_FPGA.bin");
			else
				strcat(infname, "rebb/WIFI_RAM_CODE_MT7663.bin");

			strcat(outfname, "mt7663_firmware.h");
			strcat(fw_name, "MT7663_FirmwareImage");
			strcat(infname_e2, "rebb/WIFI_RAM_CODE_MT7663_E2.bin");
			strcat(outfname_e2, "mt7663_firmware_e2.h");
			strcat(fw_name_e2, "MT7663_FirmwareImage_E2");
			strcat(e2p_name, "MT7663_E2PImage");
			strcat(ine2pname, "rebb/MT7663_EEPROM.bin");
			strcat(oute2pname, "mt7663_e2p.h");
			is_bin2h_fw = 1;
			is_bin2h_e2p = 1;
			is_bin2h_rom_patch = 1;
			is_bin2h_rom_patch_e2 = 1;
		} else if (strncmp(chipset, "axe", 7) == 0) {
			strcat(in_rom_patch, "rebb/mtaxe_patch_e1_hdr.bin");
			strcat(out_rom_patch, "axe_rom_patch_e1.h");
			strcat(rom_patch_name, "axe_rom_patch_e1");

			if ((strncmp(fpga, "y", 1) == 0))
				strcat(infname, "rebb/WIFI_RAM_CODE_AXE_FPGA.bin");
			else
				strcat(infname, "rebb/WIFI_RAM_CODE_axe.bin");

			strcat(outfname, "axe_firmware.h");
			strcat(fw_name, "AXE_FirmwareImage");
			strcat(e2p_name, "AXE_E2PImage");
			strcat(ine2pname, "rebb/axe_EEPROM.bin");
			strcat(oute2pname, "axe_e2p.h");
			is_bin2h_fw = 1;
			is_bin2h_e2p = 1;
			is_bin2h_rom_patch = 1;
		} else if (strncmp(chipset, "mt7626", 7) == 0) {
			strcat(in_rom_patch, "rebb/mt7626_patch_e1_hdr.bin");
			strcat(out_rom_patch, "mt7626_rom_patch_e1.h");
			strcat(rom_patch_name, "mt7626_rom_patch_e1");

			if ((strncmp(fpga, "y", 1) == 0))
				strcat(infname, "rebb/WIFI_RAM_CODE_7626_FPGA.bin");
			else
				strcat(infname, "rebb/WIFI_RAM_CODE_7626.bin");

			strcat(in_emi_ram_ilm, "rebb/WIFI_RAM_CODE_iemi.bin"); /* EMI RAM ILM */
			strcat(in_emi_ram_dlm, "rebb/WIFI_RAM_CODE_demi.bin"); /* EMI RAM DLM */
			strcat(out_emi_ram, "mt7626_emi_bin2h.h");
			strcat(emi_ram_ilm_name, "wifi_emi_ram_ilm");
			strcat(emi_ram_dlm_name, "wifi_emi_ram_dlm");

			strcat(outfname, "mt7626_firmware.h");
			strcat(fw_name, "MT7626_FirmwareImage");
			strcat(e2p_name, "MT7626_E2PImage");
			strcat(ine2pname, "rebb/MT7626_EEPROM.bin");
			strcat(oute2pname, "mt7626_e2p.h");
			is_bin2h_fw = 1;
			is_bin2h_e2p = 1;
			is_bin2h_rom_patch = 1;
			is_b2h_emi_ram = 1;
		} else if (adie != NULL && (strncmp(adie, "mt7976", 7) == 0) &&
				(strncmp(chipset, "mt7915", 7) == 0)) {
			printf("MT7976 A-Die!\n");
			strcat(in_rom_patch, "rebb/mt7915_6e_patch_e1_hdr.bin");
			strcat(out_rom_patch, "mt7915_6e_rom_patch_e1.h");
			strcat(rom_patch_name, "mt7915_6e_rom_patch_e1");
/*
			strcat(in_rom_patch_e2, "rebb/mt7915_patch_e2_hdr.bin");
			strcat(out_rom_patch_e2, "mt7915_rom_patch_e2.h");
			strcat(rom_patch_name_e2, "mt7915_rom_patch_e2");
*/
			strcat(infname, "rebb/WIFI_RAM_CODE_7915_6e.bin");
			strcat(outfname, "mt7915_6e_firmware.h");
			strcat(fw_name, "MT7915_6e_FirmwareImage_E1");
/*
			strcat(infname_e2, "rebb/WIFI_RAM_CODE_7915_6e_E2.bin");
			strcat(outfname_e2, "mt7915_6e_firmware_e2.h");
			strcat(fw_name_e2, "MT7915_6e_FirmwareImage_E2");
*/
			strcat(infname1, "rebb/7915_6e_WACPU_RAM_CODE_release.bin");
			strcat(outfname1, "mt7915_6e_WA_firmware.h");
			strcat(fw_name1, "MT7915_6e_WA_FirmwareImage");

			is_bin2h_fw = 1;
			is_bin2h_rom_patch = 1;
			is_bin2h_rom_patch_e2 = 1;

			/* iPAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7915_6e_E2PImage_iPAiLNA");
			strcat(ine2pname, "rebb/MT7915_MT7976_iPAiLNA_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7915_6e_e2p_iPAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* is_bin2h_e2p = 1; b2h is already done */
		} else if (strncmp(chipset, "mt7915", 7) == 0) {
			strcat(in_rom_patch, "rebb/mt7915_patch_e1_hdr.bin");
			strcat(out_rom_patch, "mt7915_rom_patch_e1.h");
			strcat(rom_patch_name, "mt7915_rom_patch_e1");

			strcat(in_rom_patch_e2, "rebb/mt7915_patch_e2_hdr.bin");
			strcat(out_rom_patch_e2, "mt7915_rom_patch_e2.h");
			strcat(rom_patch_name_e2, "mt7915_rom_patch_e2");

			strcat(infname, "rebb/WIFI_RAM_CODE_7915.bin");
			strcat(outfname, "mt7915_firmware.h");
			strcat(fw_name, "MT7915_FirmwareImage_E1");

			strcat(infname_e2, "rebb/WIFI_RAM_CODE_7915_E2.bin");
			strcat(outfname_e2, "mt7915_firmware_e2.h");
			strcat(fw_name_e2, "MT7915_FirmwareImage_E2");

			strcat(infname1, "rebb/7915_WACPU_RAM_CODE_release.bin");
			strcat(outfname1, "mt7915_WA_firmware.h");
			strcat(fw_name1, "MT7915_WA_FirmwareImage");

			is_bin2h_fw = 1;
			is_bin2h_rom_patch = 1;
			is_bin2h_rom_patch_e2 = 1;

			/* iPAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7915_E2PImage_iPAiLNA");
			strcat(ine2pname, "rebb/MT7915_iPAiLNA_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7915_e2p_iPAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* iPAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7915_E2PImage_iPAeLNA");
			strcat(ine2pname, "rebb/MT7915_iPAeLNA_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7915_e2p_iPAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7915_E2PImage_ePAeLNA");
			strcat(ine2pname, "rebb/MT7915_ePAeLNA_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7915_e2p_ePAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strncpy(ine2pname2, ine2pname, 512 - 1);
			ine2pname2[511] = '\0';
			strncpy(ine2pname3, ine2pname, 512 - 1);
			ine2pname3[511] = '\0';
			strcat(e2p_name, "MT7915_E2PImage_ePAiLNA");
			strcat(ine2pname, "rebb/MT7915_ePAiLNA_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7915_e2p_ePAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* is_bin2h_e2p = 1; b2h is already done */
		} else if (strncmp(chipset, "mt7986", 7) == 0 && sku != NULL && adie != NULL) {
			int i = 0;

			printf("sku=%s, adie=%s\n", sku, adie);

			for(i = 0;i < strlen(sku); i++)
				sku[i] = tolower(sku[i]);
			for(i = 0;i < strlen(adie); i++)
				adie[i] = tolower(adie[i]);

			if (strncmp(adie, "mt7975", 6) == 0) {
				strcat(infname, "rebb/WIFI_RAM_CODE_MT7986_MT7975.bin");
				strcat(in_rom_patch, "rebb/mt7986_patch_e1_hdr_mt7975.bin");
			}else if (strncmp(adie, "mt7976", 6) == 0) {
				strcat(infname, "rebb/WIFI_RAM_CODE_MT7986.bin");
				strcat(in_rom_patch, "rebb/mt7986_patch_e1_hdr.bin");
			}else
				printf("unknown adie:%s", adie);

			strcat(out_rom_patch, "mt7986_rom_patch_e1.h");
			strcat(rom_patch_name, "mt7986_rom_patch_e1");

			strcat(outfname, "mt7986_firmware.h");
			strcat(fw_name, "MT7986_FirmwareImage_E1");

			strcat(infname1, "rebb/7986_WACPU_RAM_CODE_release.bin");
			strcat(outfname1, "mt7986_WA_firmware.h");
			strcat(fw_name1, "MT7986_WA_FirmwareImage");

			is_bin2h_fw = 1;
			is_bin2h_rom_patch = 1;
			is_bin2h_rom_patch_e2 = 0;

			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7986_E2PImage_iPAiLNA");
			strcat(ine2pname, "rebb/MT7986_iPAiLNA_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7986_e2p_iPAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");

			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");

			if ((strncmp(sku, "ax6000", 6) == 0) || (strncmp(sku, "ax8400", 6) == 0)) {
				if (strncmp(adie, "mt7975", 6) == 0)
					strcat(ine2pname, "rebb/MT7986_iPAiLNA_EEPROM_AX6000.bin");
				else if (strncmp(adie, "mt7976", 6) == 0)
					strcat(ine2pname, "rebb/MT7986_ePAeLNA_EEPROM_AX6000.bin");
				else
					printf("unknown ADIE:%s", adie);
			} else if (strncmp(sku, "ax7800", 6) == 0) {
				if (strncmp(adie, "mt7975", 6) == 0)
					strcat(ine2pname, "rebb/MT7986_iPAiLNA_EEPROM_AX7800.bin");
				else if (strncmp(adie, "mt7976", 6) == 0)
					strcat(ine2pname, "rebb/MT7986_ePAeLNA_EEPROM_AX7800.bin");
				else
					printf("unknown ADIE:%s", adie);
			} else if (strncmp(sku, "ax4200", 6) == 0) {
				if (strncmp(adie, "mt7976", 6) == 0)
					strcat(ine2pname, "rebb/MT7986_ePAeLNA_EEPROM_ONEADIE_DBDC.bin");
				else
					printf("unknown ADIE:%s", adie);
			} else
				printf("unknown SKU:%s", sku);

			strcat(e2p_name, "MT7986_E2PImage");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7986_e2p.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* is_bin2h_e2p = 1; b2h is already done */
		} else if (strncmp(chipset, "mt7916", 7) == 0) {
			strcat(in_rom_patch, "rebb/mt7916_patch_e1_hdr.bin");
			strcat(out_rom_patch, "mt7916_rom_patch_e1.h");
			strcat(rom_patch_name, "mt7916_rom_patch_e1");

			strcat(infname, "rebb/WIFI_RAM_CODE_MT7916.bin");
			strcat(outfname, "mt7916_firmware.h");
			strcat(fw_name, "MT7916_FirmwareImage_E1");

			strcat(infname1, "rebb/7916_WACPU_RAM_CODE_release.bin");
			strcat(outfname1, "mt7916_WA_firmware.h");
			strcat(fw_name1, "MT7916_WA_FirmwareImage");

			is_bin2h_fw = 1;
			is_bin2h_rom_patch = 1;
			is_bin2h_rom_patch_e2 = 0;

			/* iPAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7916_E2PImage_iPAiLNA");
			strcat(ine2pname, "rebb/MT7916_iPAiLNA_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7916_e2p_iPAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* iPAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7916_E2PImage_iPAeLNA");
			strcat(ine2pname, "rebb/MT7916_iPAeLNA_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7916_e2p_iPAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7916_E2PImage_ePAeLNA");
			strcat(ine2pname, "rebb/MT7916_ePAeLNA_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7916_e2p_ePAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strncpy(ine2pname2, ine2pname, 512);
			strncpy(ine2pname3, ine2pname, 512);
			strcat(e2p_name, "MT7916_E2PImage_ePAiLNA");
			strcat(ine2pname, "rebb/MT7916_ePAiLNA_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7916_e2p_ePAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* is_bin2h_e2p = 1; b2h is already done */
		} else if (strncmp(chipset, "mt7981", 7) == 0) {
			strcat(in_rom_patch, "rebb/mt7981_patch_e1_hdr.bin");
			strcat(out_rom_patch, "mt7981_rom_patch_e1.h");
			strcat(rom_patch_name, "mt7981_rom_patch_e1");

			strcat(infname, "rebb/WIFI_RAM_CODE_MT7981.bin");
			strcat(outfname, "mt7981_firmware.h");
			strcat(fw_name, "MT7981_FirmwareImage_E1");

			strcat(infname1, "rebb/7981_WACPU_RAM_CODE_release.bin");
			strcat(outfname1, "mt7981_WA_firmware.h");
			strcat(fw_name1, "MT7981_WA_FirmwareImage");

			is_bin2h_fw = 1;
			is_bin2h_rom_patch = 1;
			is_bin2h_rom_patch_e2 = 0;

			/* iPAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7981_E2PImage_iPAiLNA");
			strcat(ine2pname, "rebb/MT7981_iPAiLNA_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7981_e2p_iPAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* iPAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7981_E2PImage_iPAeLNA");
			strcat(ine2pname, "rebb/MT7981_iPAeLNA_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7981_e2p_iPAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7981_E2PImage_ePAeLNA");
			strcat(ine2pname, "rebb/MT7981_ePAeLNA_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7981_e2p_ePAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAiLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strncpy(ine2pname2, ine2pname, 512);
			strncpy(ine2pname3, ine2pname, 512);
			strcat(e2p_name, "MT7981_E2PImage_ePAiLNA");
			strcat(ine2pname, "rebb/MT7981_ePAiLNA_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7981_e2p_ePAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* is_bin2h_e2p = 1; b2h is already done */
		} else
			printf("unknown chipset = %s\n", chipset);

		if (is_bin2h_fw) {
			if (strncmp(infname, input_fw_path, strlen(infname)) != 0)
				bin2h(infname, outfname, fw_name, "w");/* N9 E1 */

			if (strncmp(infname_e2, input_fw_path, strlen(infname_e2)) != 0)
				bin2h(infname_e2, outfname_e2, fw_name_e2, "w");/* N9 E2 */

			if (strncmp(infname_e3, input_fw_path, strlen(infname_e3)) != 0)
				bin2h(infname_e3, outfname_e3, fw_name_e3, "w");/* N9 E3 */

			if (strncmp(infname1, input_fw_path, strlen(infname1)) != 0)
				bin2h(infname1, outfname1, fw_name1, "w");/* CR4 */
		}

		if (is_bin2h_rom_patch)
			if (strncmp(in_rom_patch, input_fw_path, strlen(in_rom_patch)) != 0)
				bin2h(in_rom_patch, out_rom_patch, rom_patch_name, "w");

		if (is_bin2h_rom_patch_e2)
			if (strncmp(in_rom_patch_e2, input_fw_path, strlen(in_rom_patch_e2)) != 0)
				bin2h(in_rom_patch_e2, out_rom_patch_e2, rom_patch_name_e2, "w");

		if (is_bin2h_rom_patch_e3)
			if (strncmp(in_rom_patch_e3, input_fw_path, strlen(in_rom_patch_e3)) != 0)
				bin2h(in_rom_patch_e3, out_rom_patch_e3, rom_patch_name_e3, "w");

		if (is_b2h_emi_ram) {
			bin2h(in_emi_ram_ilm, out_emi_ram, emi_ram_ilm_name, "w");
			bin2h(in_emi_ram_dlm, out_emi_ram, emi_ram_dlm_name, "a");
		}

		if (is_bin2h_e2p) {
			bin2h(ine2pname, oute2pname, e2p_name, "w");

			if (e2p_name2[0])
				bin2h(ine2pname2, oute2pname, e2p_name2, "a");

			if (e2p_name3[0])
				bin2h(ine2pname3, oute2pname, e2p_name3, "a");
		}

		chipset = strtok(NULL, " ");
	}

	exit(0);
}
