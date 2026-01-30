#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

extern unsigned long crc32 (unsigned int crc, const unsigned char *p, unsigned int len);

#define MAX_LINE_LENGTH   (4096)

int main(int argc, char *argv[])
{
	FILE *fp = NULL;
	FILE *fp_input = NULL;
	char *buf = NULL;
	int size = MAX_LINE_LENGTH;
	long int totalhiflashsize = -1;
	long int totalnandflashsize = 256 * 1024;
	unsigned long crc;
	char line[MAX_LINE_LENGTH];
	char bootargs_input[1024] = "bootargs_input.txt";
	char bootargs_bin[1024] = "bootargs.bin";
	char *strsrc;
	int offset = 0;
	int oc = 0;
	char *tmp = NULL;
	int len = 0;

	if (argc < 2) {
		printf("\r\nUsage: mkbootargs [-s Size] [-r Input File] [-o Output File] :\n"
		       "       -r Input File: A TXT file that describes the Boot Env config, default 'bootargs_input.txt'\n"
		       "       -o Output File: The output bin file, default 'bootargs.bin'\n"
		       "       -s Size: Boot Env size in KB, usually, equal to the bootargs partition size. default '512'.\n"
		       "\r\nmkbootargs will read 'Input File' and create 'Output File'.\n");
		printf("\r\nExample:./makebootargs -s 64 -r bootargs_input.txt -o bootargs.bin\n\n");
		return 1;
	}

	while ((oc = getopt(argc, argv, "r:o:s:p:n:")) != -1) {
		switch (oc) {
		case 'r':
			sprintf(bootargs_input, "%s", optarg);
			printf("optarg:%s\n", optarg);
			break;
		case 'o':
			sprintf(bootargs_bin, "%s", optarg);
			printf("optarg:%s\n", optarg);
			break;
		case 's':
			size = atoi(optarg);
			printf("optarg:%s\n", optarg);
			break;
		default:
			printf("unknow args\n");
			break;
		}
	}

	if (size > 1) {
		printf("flash size is %dKB.\n", size);
		size = size * 1024;
	} else
		printf("flash size too small.\n");

	fp = fopen(bootargs_bin, "w+b");
	if (NULL == fp) {
		printf("open 'Output File' error.\n");
		return -1;
	}

	fp_input = fopen(bootargs_input, "rb");
	if (NULL == fp_input) {
		printf("no 'Input File'!\n");
		fclose(fp);
		return -1;
	}

	buf = malloc(size);
	if (NULL == buf) {
		printf("open bootargs out of memory\n");
		fclose(fp);
		fclose(fp_input);
		return -1;
	}
	strsrc = malloc(size * sizeof(char));
	if (NULL == strsrc) {
		printf("open bootargs out of memory\n");
		fclose(fp);
		fclose(fp_input);
		free(buf);
		return -1;
	}

	memset(buf, 0x00, size);
	memset(strsrc, 0x00, size);

	while (fgets(line, size-1, fp_input) != NULL) {
		strcat(strsrc, line);
		tmp = line;
		while (*tmp != '\0') {
			if (*tmp == '\n' || (*tmp == 0xd))
				*tmp = '\0';

			tmp++;
		}

		len = strlen(line);
		printf("%d--%s\n", len, line);
		memcpy(buf + 4 + offset, line, len);
		offset += len + 1;
		memset(line, 512, 0);
	}

	crc = crc32(0, buf + 4, size - 4);
	memcpy(buf, (unsigned char *)&crc, 4);

	/* init flash data */
	fwrite(buf, size, 1, fp);

	free(buf);
	free(strsrc);
	fclose(fp);

	if (fp_input != stdin)
		fclose(fp_input);

	return 0;
}
