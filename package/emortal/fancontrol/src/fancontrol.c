#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>

#define MAX_LENGTH 200
#define MAX_TEMP 120

// 定义全局变量
char thermal_file[MAX_LENGTH] = "/sys/devices/virtual/thermal/thermal_zone0/temp";      // -T
char fan_file[MAX_LENGTH] = "/sys/devices/platform/pwm-fan/hwmon/hwmon0/pwm1";   // -F

int start_speed = 35;   // -s
int start_temp = 45;    // -t
int max_speed = 255;    // -m
int temp_div = 1000;    // -d
int debug_mode = 0;     // -D

/**
 * 底层读文件
 */
static int read_file(const char* path ,char* result ,size_t size) {
	FILE* fp;
	char* line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen(path ,"r");
	if (fp == NULL)
		return -1;

	if (( read = getline(&line ,&len ,fp) ) != -1) {
		if (size != 0)
			memcpy(result ,line ,size);
		else
			memcpy(result ,line ,read - 1);
	}

	fclose(fp);
	if (line)
		free(line);
	return 0;
}

/**
 * 底层写文件
 */
static size_t write_file(const char* path ,char* buf ,size_t len) {
	FILE* fp = NULL;
	size_t size = 0;
	fp = fopen(path ,"w+");
	if (fp == NULL) {
		return 0;
	}
	size = fwrite(buf ,len ,1 ,fp);
	fclose(fp);
	return size;
}

/**
 * 读取温度
 */
int get_temperature(char* thermal_file ,int div) {
	char buf[8] = { 0 };
	if (read_file(thermal_file ,buf ,0) == 0) {
		return atoi(buf) / div;
	}
	return -1;
}

/**
 * 读取风扇速度
 */
int get_fanspeed(char* fan_file) {
	char buf[8] = { 0 };
	if (read_file(fan_file ,buf ,0) == 0) {
		return atoi(buf);
	}
	return -1;
}

/**
 * 设置风扇转速
 */
int set_fanspeed(int fan_speed ,char* fan_file) {
	char buf[8] = { 0 };
	sprintf(buf ,"%d\n" ,fan_speed);
	return write_file(fan_file ,buf ,strlen(buf));
}

/**
 * 计算风扇转速
 */
int calculate_speed(int current_temp ,int max_temp ,int min_temp ,int max_speed ,int min_speed) {
	if (current_temp < min_temp) {
		return 0;
	}
	int fan_speed = ( current_temp - min_temp ) * ( max_speed - min_speed ) / ( max_temp - min_temp ) + min_speed;
	if (fan_speed > max_speed) {
		fan_speed = max_speed;
	}
	return fan_speed;
}

/**
 * 判断文件是否存在方法
 */
static int file_exist(const char* name) {
	struct stat buffer;
	return stat(name ,&buffer);
}

/**
 *  信号处理函数
 */
void handle_termination(int signum) {
	// 设置风扇转速为 0
	set_fanspeed(0 ,fan_file);
	exit(EXIT_SUCCESS); // 优雅地退出程序
}

/**
 * 注册信号处理函数
 */
void register_signal_handlers( ) {
	struct sigaction sa;
	memset(&sa ,0 ,sizeof(sa));
	sa.sa_handler = handle_termination;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT ,&sa ,NULL);
	sigaction(SIGTERM ,&sa ,NULL);
}

/**
 * 主函数
 */
int main(int argc ,char* argv[ ]) {
	// 解析命令行选项
	int opt;
	while (( opt = getopt(argc ,argv ,"T:F:s:t:m:d:D:v:") ) != -1) {
		switch (opt) {
			case 'T':
				snprintf(thermal_file ,sizeof(thermal_file) ,"%s" ,optarg);
				break;
			case 'F':
				snprintf(fan_file ,sizeof(fan_file) ,"%s" ,optarg);
				break;
			case 's':
				start_speed = atoi(optarg);
				break;
			case 't':
				start_temp = atoi(optarg);
				break;
			case 'm':
				max_speed = atoi(optarg);
				break;
			case 'd':
				temp_div = atoi(optarg);
				break;
			case 'D':
				debug_mode = atoi(optarg);
				break;
			default:
				fprintf(stderr ,"Usage: %s [option]\n"
					"          -T sysfs         # temperature sysfs file, default is '%s'\n"
					"          -F sysfs         # fan sysfs file, default is '%s'\n"
					"          -s speed         # initial speed for fan startup, default is %d\n"
					"          -t temperature   # fan start temperature, default is %d°C\n"
					"          -m speed         # fan maximum speed, default is %d\n"
					"          -d div           # temperature divide, default is %d\n"
					"          -v               # verbose\n" ,argv[0] ,thermal_file ,fan_file ,start_speed ,start_temp ,max_speed ,temp_div);
				exit(EXIT_FAILURE);
		}
	}
	// 检测虚拟文件是否存在
	if (file_exist(fan_file) != 0 || file_exist(thermal_file) != 0) {
		fprintf(stderr ,"File: '%s' or '%s' not exist\n" ,fan_file ,thermal_file);
		exit(EXIT_FAILURE);
	}

	// 注册退出信号
	register_signal_handlers( );

	// 监控风扇
	while (1) {
		int temperature = get_temperature(thermal_file ,temp_div);
		// 有效温度时设置风扇速度
		if (temperature > 0) {
			int fan_speed = calculate_speed(temperature ,MAX_TEMP ,start_temp ,max_speed ,start_speed);
			set_fanspeed(fan_speed ,fan_file);
		}
		if (debug_mode) {
			fprintf(stdout ,"Temperature: %d°C, Fan Speed: %d\n" ,get_temperature(thermal_file ,temp_div) ,get_fanspeed(fan_file));
		}
		sleep(5);
	}
	return 0;
}
