#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <stdint.h>


#ifdef WIN32
#include <io.h>
#include <Windows.h>
#include "getopt.h"

typedef struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
} timezone;

int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	// This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
	// until 00:00:00 January 1, 1970 
	static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

	SYSTEMTIME  system_time;
	FILETIME    file_time;
	uint64_t    time;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	time = ((uint64_t)file_time.dwLowDateTime);
	time += ((uint64_t)file_time.dwHighDateTime) << 32;

	tp->tv_sec = (long)((time - EPOCH) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
	return 0;
}

#pragma warning(disable: 4996)
#endif

#if defined(__unix__) || defined(__APPLE__)
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

void Sleep(unsigned int millis)
{
	usleep(millis * 1000);
}

#endif

// global variables for the names list
char** first_names = NULL;
char** last_names = NULL;
int num_first_names = 0;
int num_last_names = 0;

void printl(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv, &tz);
	time_t t = tv.tv_sec;
	struct tm* pt = localtime(&t);
	printf("[%02d:%02d:%02d.%03d] ", pt->tm_hour, pt->tm_min, pt->tm_sec, (uint32_t)tv.tv_usec/1000);
	vprintf(fmt, args);
	va_end(args);
}

int load_from_file(const char* filename, char** target, int count)
{
	printl("Loading names from %s...\n", filename);
	FILE* fp = fopen(filename, "rt");
	if (!fp) {
		printl("error opening: %s\n", filename);
		return 0;
	}

	// load up to count
	int n = 0;
	char buf[1024];
	while (fgets(buf, sizeof(buf)-1, fp)) {
		char* p = strtok(buf, " ");
		if (p && *p) {
			target[n] = strdup(p);
			if (++n >= count)
				break;
		}
	}
	fclose(fp);
	printl("Loading %d names\n", n);

	return n;
}

void load_names_database()
{
	const int MaxNames = 2048;
	last_names = (char**)malloc(sizeof(char*) * MaxNames);
	first_names = (char**)malloc(sizeof(char*) * MaxNames);
	memset(last_names, 0, sizeof(char*) * MaxNames);
	memset(first_names, 0, sizeof(char*) * MaxNames);

	const char* LastNamesFile = "dist.all.last.txt";
	const char* MaleFirstNamesFile = "dist.male.first.txt";
	const char* FemaleFirstNamesFile = "dist.female.first.txt";

	// these lists were taken from the US census data found here:
	// https://www.census.gov/topics/population/genealogy/data/1990_census/1990_census_namefiles.html
	if (access(LastNamesFile, 0) == 0) 
		num_last_names = load_from_file(LastNamesFile, last_names, MaxNames);

	if (num_last_names == 0) {
		printl("Loaded 0 names from %s. Using default set.\n", LastNamesFile);
		// use following as default set of last names
		char* def_last_names[] = { "SMITH", "JOHNSON", "WILLIAMS", "JONES", "BROWN", "DAVIS", "MILLER",
			"WILSON", "MOORE", "TAYLOR", "ANDERSON", "THOMAS", "JACKSON", "WHITE", "HARRIS", "MARTIN" };
		num_last_names = sizeof(def_last_names) / sizeof(def_last_names[0]);
		for (int i = 0; i < num_last_names; i++)
			last_names[i] = strdup(def_last_names[i]);
	}

	if (access(MaleFirstNamesFile, 0) == 0) 
		num_first_names = load_from_file(MaleFirstNamesFile, first_names, MaxNames/2);

	if (access(FemaleFirstNamesFile, 0) == 0) 
		num_first_names += load_from_file(FemaleFirstNamesFile, &first_names[num_first_names], MaxNames/2);

	if (num_first_names == 0) {
		printl("Loaded 0 names from %s or %s. Using default set.\n", MaleFirstNamesFile, FemaleFirstNamesFile);
		// use following as default set of last names
		char* def_first_names[] = { "JAMES", "JOHN", "ROBERT", "MICHAEL", "WILLIAM", "DAVID", "RICHARD",
			"CHARLES", "JOSEPH", "THOMAS", "CHRISTOPHER", "DANIEL", "PAUL", "MARK", "DONALD", "GEORGE" };
		num_first_names = sizeof(def_first_names) / sizeof(def_first_names[0]);
		for (int i = 0; i < num_first_names; i++)
			first_names[i] = strdup(def_first_names[i]);
	}
}

void free_names()
{
	for (int i = 0; i < num_first_names; i++)
		free(first_names[i]);
	free(first_names);

	for (int i = 0; i < num_last_names; i++)
		free(last_names[i]);
	free(last_names);
}

const char* generate_pan(char prefix, int length)
{
	static char buf[80];
	if (length > sizeof(buf) - 1 || length < 6)
		return NULL;    // invalid
	buf[length - 1] = '0';
	buf[length] = '\0';
	// generate random PAN except check digit
	char* p = buf;
	*p++ = prefix + '0';
	for (int n = length - 2; n > 0; --n)
		*p++ = (rand() % 10) + '0';
	// compute check digit
	int sum = 0;
	const char* pend = &buf[length - 1];
	for (int i = 0; i < length; ++i, --pend) {
		char val = *pend - '0';
		if (i % 2) {
			val *= 2;
			// if the result is > 9, subtract 9
			if (val > 9)
				val -= 9;
		}
		sum += val;
	}
	sum *= 9;
	char check_digit = (sum % 10) + '0';
	buf[length - 1] = check_digit;
	return buf;
}

const char* generate_track1(char prefix, int length)
{
	static char buf[256];
	const char* pan = generate_pan(prefix, length);
	int fn = rand() % num_first_names;
	int ln = rand() % num_last_names;
	const char* discretionary_data = "00000019301000000877000000";
	int year = 16 + (rand() % 14);
	int month = (rand() % 12) + 1;
	int service_code = (((rand() % 2) + 1) * 100) + 1;
	sprintf(buf, "%%B%s^%s %s^%02d%02d%03d%s?", pan, first_names[fn], last_names[ln], year, month, service_code, discretionary_data);
	return buf;
}

const char* generate_track2(char prefix, int length)
{
	static char buf[256];
	const char* pan = generate_pan(prefix, length);
	const char* discretionary_data = "00000019301000000877000000";
	int year = 16 + (rand() % 14);
	int month = (rand() % 12) + 1;
	int service_code = (((rand() % 2) + 1) * 100) + 1;
	sprintf(buf, ";%s=%02d%02d%03d%s?", pan, year, month, service_code, discretionary_data);
	return buf;
}

const char* generate_track12(char prefix, int length)
{
	static char buf[256];
	const char* pan = generate_pan(prefix, length);
	int fn = rand() % num_first_names;
	int ln = rand() % num_last_names;
	const char* discretionary_data = "00000019301000000877000000";
	int year = 16 + (rand() % 14);
	int month = (rand() % 12) + 1;
	int service_code = (((rand() % 2) + 1) * 100) + 1;

	sprintf(buf, "%%B%s^%s %s^%02d%02d%03d%s?;%s=%02d%02d%03d%s?",
		pan, first_names[fn], last_names[ln], year, month, service_code, discretionary_data,
		pan, year, month, service_code, discretionary_data);
	return buf;
}

int main(int argc, char* argv[])
{
	const char* usage =	"Usage:\n"
		"\tccgen (-1 | -2) [-s <seed>] [-h|-?]\n"
		"\n"
		"\t-1           : generate track1 data (or specify both)\n"
		"\t-2           : generate track2 data (or specify both)\n"
		"\t-s <seed>    : seed for the random number generator\n"
		"\t-d <delay>   : delay in milliseconds (default: 100)\n"
		"\t-h | -?      : this help\n"
		"\n";

	int gen_track1 = 0;
	int gen_track2 = 0;
	int delay = 100;
	srand((unsigned int)time(NULL));
	int opt = 0;
	while ((opt = getopt(argc, argv, "Hh?12s:d:")) != -1) {
		switch (opt) {
		case 'H': case 'h': case '?':
			printf("%s\n", usage);
			break;
		case '1':
			gen_track1 = 1;
			break;

		case '2':
			gen_track2 = 1;
			break;

		case 's':
			srand(atoi(optarg));
			break;

		case 'd':
			delay = atoi(optarg);
		}
	}

	if (!gen_track1 && !gen_track2) {
		printf("%s\nerror: please specify either -1 or -2, or both\n\n", usage);
		exit(EXIT_FAILURE);
	}

	load_names_database();

	printf("Generating %s%sdata.\n", gen_track1 ? "Track1 " : "", gen_track2 ? "Track2 " : "");

	// generate various types of credit cards (specified by their prefix)
	// the length is dependent on the prefix
	struct prefix_length {
		char prefix;
		int length;
	} prefix_lengths[4] = {
		{ 3, 15 },
		{ 4, 16 },
		{ 5, 16 },
		{ 6, 16 },
	};

	// generate some credit card numbers in memory. Some malwares will try and read process memory to see
	// if something resembles a credit-card and will do things like luhn code checking
	int max_len = 256;
	char* circ_buf[1024];
	for (int i = 0; i < 1024; ++i)
		circ_buf[i] = (char*)malloc(max_len);
	for (unsigned int idx = 0; ; idx++) {
		int n = rand() % 4;
		char* p = circ_buf[idx % (sizeof(circ_buf) / sizeof(circ_buf[0]))];
		if (gen_track1) {
			strcpy(p, generate_track1(prefix_lengths[n].prefix, prefix_lengths[n].length));
            if (gen_track2)
                strcat(p, generate_track2(prefix_lengths[n].prefix, prefix_lengths[n].length));
			printl("%s\n", p);
		}
        else if (gen_track2) {
			p = circ_buf[idx % (sizeof(circ_buf) / sizeof(circ_buf[0]))];
            strcpy(p, generate_track2(prefix_lengths[n].prefix, prefix_lengths[n].length));
			printl("%s\n", p);
		}
		Sleep(delay);
	}
	return EXIT_SUCCESS;
}
