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

/*******************************************************************************
 *    INCLUDED FILES
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*******************************************************************************
 *    DEFINITIONS
 ******************************************************************************/
#define PATH_OF_SKU_TABLE_IN   "/txpwr/sku_tables/"
#define PATH_OF_SKU_TABEL_OUT  "/include/txpwr/"

/*******************************************************************************
 *    TYPES
 ******************************************************************************/


/*******************************************************************************
 *    PUBLIC DATA
 ******************************************************************************/

#define MAX_SKUTABLE_NUM             20

/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/
int dat2h(char *infname, char *outfname, char *varname, char *deffname, const char *mode)
{
	int ret = 0;
	FILE *infile, *outfile, *definfile;
	int c;
	/* int i=0; */
	unsigned int fgDefTable = 0;
	/* Open input file */
	infile = fopen(infname, "r");

	/* Check open file status for input file */
	if (infile == (FILE *) NULL) {
		printf("Can't read file %s\n", infname);
		printf("System would automatically apply default table !!\n");
		/* Flag for use Default SKU table */
		fgDefTable = 1;
		/* Open default input file */
		definfile = fopen(deffname, "r");

		/* Check open file status for default file */
		if (definfile == (FILE *) NULL) {
			printf("Can't read def file %s\n", deffname);
			return -1;
		}
	}

	outfile = fopen(outfname, mode);

	/* Check open file status for output file */
	if (outfile == (FILE *) NULL) {
		printf("Can't open write file %s\n", outfname);

		/* Close input file or default input file */
		if (fgDefTable == 0)
			fclose(infile);
		else
			fclose(definfile);

		return -1;
	}

	/* Comment in header files */
	fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */\n", outfile);
	fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */\n", outfile);
	fputs("\n", outfile);
	fputs("\n", outfile);
	/* Contents in header file */
	fprintf(outfile, "UCHAR %s[] = \"", varname);

	while (1) {
		char cc[2];

		if (fgDefTable == 0)
			c = getc(infile);
		else
			c = getc(definfile);

		/* backward compatibility for old Excel SKU table */
		if (c == '#') {
			c = '!';
			snprintf(cc, sizeof(cc), "%c", c);
			fputs(cc, outfile);
		}

		if (fgDefTable == 0) {
			if (feof(infile))
				break;
		} else {
			if (feof(definfile))
				break;
		}

		if (c == '\r')
			continue;
		else if (c == '\n') {
			c = '\t';
			snprintf(cc, sizeof(cc), "%c", c);
			fputs(cc, outfile);
			c = '\"';
			snprintf(cc, sizeof(cc), "%c", c);
			fputs(cc, outfile);
			c = '\n';
			snprintf(cc, sizeof(cc), "%c", c);
			fputs(cc, outfile);
			c = '\"';
			snprintf(cc, sizeof(cc), "%c", c);
			fputs(cc, outfile);
		} else {
			snprintf(cc, sizeof(cc), "%c", c);
			fputs(cc, outfile);
		}
	}

	fputs("\";\n", outfile);

	/* close input file or default input file */
	if (fgDefTable == 0)
		fclose(infile);
	else
		fclose(definfile);

	/* close output file */
	fclose(outfile);
	return 0;
}

int main(int argc, char *argv[])
{
	char infname[512];
	char outfname[512];
	char deffname[512];
	char varname[128];
	char *rt28xxdir;
	int  SKUTableIdx;
	char cc[20];

	/* get directory path from environemnt variable */
	rt28xxdir = (char *)getenv("RT28xx_DIR");
	if (rt28xxdir == NULL)
		return -1;

	/* Trasform SKU table data file to header file */
	for (SKUTableIdx = 1; SKUTableIdx <= MAX_SKUTABLE_NUM; SKUTableIdx++) {
		/* configure input file address and file name */
		memset(infname, 0, 512);
		snprintf(infname, 512, "%s", rt28xxdir);
		snprintf(infname + strlen(infname), 512 - strlen(infname), "%s", PATH_OF_SKU_TABLE_IN);
		snprintf(infname + strlen(infname), 512 - strlen(infname), "%s", "7615_SingleSKU_");
		sprintf(cc, "%d", SKUTableIdx);
		snprintf(infname + strlen(infname), 512 - strlen(infname), "%s", cc);
		snprintf(infname + strlen(infname), 512 - strlen(infname), "%s", ".dat");
		printf("Input: [%d] %s\n", SKUTableIdx, infname);
		/* configure output file address and file name */
		memset(outfname, 0, 512);
		snprintf(outfname, 512, "%s", rt28xxdir);
		snprintf(outfname + strlen(outfname), 512 - strlen(outfname), "%s", PATH_OF_SKU_TABEL_OUT);
		snprintf(outfname + strlen(outfname), 512 - strlen(outfname), "%s", "SKUTable_");
		sprintf(cc, "%d", SKUTableIdx);
		snprintf(outfname + strlen(outfname), 512 - strlen(outfname), "%s", cc);
		snprintf(outfname + strlen(outfname), 512 - strlen(outfname), "%s", ".h");
		printf("Output: [%d] %s\n", SKUTableIdx, outfname);
		/* configure default input file address and file name */
		memset(deffname, 0, 512);
		snprintf(deffname, 512, "%s", rt28xxdir);
		snprintf(deffname + strlen(deffname), 512 - strlen(deffname), "%s", PATH_OF_SKU_TABLE_IN);
		snprintf(deffname + strlen(deffname), 512 - strlen(deffname), "%s", "7615_SingleSKU_default.dat");
		printf("Def Input: [%d] %s\n", SKUTableIdx, deffname);
		/* Configure variable name for SKU contents in header file */
		memset(varname, 0, 128);
		snprintf(varname, 128, "%s", "SKUvalue_");
		sprintf(cc, "%d", SKUTableIdx);
		snprintf(varname + strlen(varname), 128 - strlen(varname), "%s", cc);
		/* Transform data file to header file */
		dat2h(infname, outfname, varname, deffname, "w");
	}

	/* Trasform BF Backoff table data file to header file */
	for (SKUTableIdx = 1; SKUTableIdx <= MAX_SKUTABLE_NUM; SKUTableIdx++) {
		/* configure input file address and file name */
		memset(infname, 0, 512);
		snprintf(infname, 512, "%s", rt28xxdir);
		snprintf(infname + strlen(infname), 512 - strlen(infname), "%s", PATH_OF_SKU_TABLE_IN);
		snprintf(infname + strlen(infname), 512 - strlen(infname), "%s", "7615_SingleSKU_BF_");
		sprintf(cc, "%d", SKUTableIdx);
		snprintf(infname + strlen(infname), 512 - strlen(infname), "%s", cc);
		snprintf(infname + strlen(infname), 512 - strlen(infname), "%s", ".dat");
		printf("Input: [%d] %s\n", SKUTableIdx, infname);
		/* configure output file address and file name */
		memset(outfname, 0, 512);
		snprintf(outfname, 512, "%s", rt28xxdir);
		snprintf(outfname + strlen(outfname), 512 - strlen(outfname), "%s", PATH_OF_SKU_TABEL_OUT);
		snprintf(outfname + strlen(outfname), 512 - strlen(outfname), "%s", "BFBackoffTable_");
		sprintf(cc, "%d", SKUTableIdx);
		snprintf(outfname + strlen(outfname), 512 - strlen(outfname), "%s", cc);
		snprintf(outfname + strlen(outfname), 512 - strlen(outfname), "%s", ".h");
		printf("Output: [%d] %s\n", SKUTableIdx, outfname);
		/* configure default input file address and file name */
		memset(deffname, 0, 512);
		snprintf(deffname, 512, "%s", rt28xxdir);
		snprintf(deffname + strlen(deffname), 512 - strlen(deffname), "%s", PATH_OF_SKU_TABLE_IN);
		snprintf(deffname + strlen(deffname), 512 - strlen(deffname), "%s", "7615_SingleSKU_BF_default.dat");
		printf("Def Input: [%d] %s\n", SKUTableIdx, deffname);
		/* Configure variable name for SKU contents in header file */
		memset(varname, 0, 128);
		snprintf(varname, 128, "%s", "BFBackoffvalue_");
		sprintf(cc, "%d", SKUTableIdx);
		snprintf(varname + strlen(varname), 128 - strlen(varname), "%s", cc);
		/* Transform data file to header file */
		dat2h(infname, outfname, varname, deffname, "w");
	}
}
