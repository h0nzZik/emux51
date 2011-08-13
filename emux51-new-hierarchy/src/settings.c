/*
 *	WARNING: This file is written quick&dirty.
 */

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


#define CFGNAME "emuxrc"
#define CFGDIR ".emux51"

char _configfile[PATH_MAX+3+strlen(CFGNAME)+strlen(CFGDIR)];

char *configfile(void)
{
	char *dir=getenv("HOME");

	if (dir)
		sprintf(_configfile, "%s/%s/%s", dir, CFGDIR, CFGNAME);
	else
		sprintf(_configfile, "./.%s", CFGNAME);
	return _configfile;
}
#if 1
/*	returns value of 'variable'	*/
char *config_read(char *variable)
{
	FILE *fr;
	char buff[80];
	char *data=NULL;
	int found=0;
	int len;
	int i;

	fr=fopen(configfile(), "rt");
	if (fr == NULL)
		return NULL;

	while(!found && fgets(buff, 80, fr)) {
		len=strlen(buff);
		if (len<1)
			break;

		/*	ignore comments	*/
		if (buff[0] == '#')
			continue;

		/*	parse line	*/
		for(i=0; i<len; i++) {
			/*	not equal, go to next line	*/
			if (variable[i] != buff[i])
				break;
		}

		/*	no match	*/
		if (i != strlen(variable))
			continue;

		/*	match	*/
		found=1;
		/*	cut spaces and '='s	*/
		while(i<len && (isspace(buff[i]) || buff[i] == '='))
			i++;
		/*	cut spaces from back	*/
		while (isspace(buff[len-1]))
			len--;
		buff[len]='\0';

		data=malloc(1+strlen(buff+i));
		if (data) {
			strcpy(data, buff+i);
		}
	}

	fclose(fr);
	return(data);
}
#endif
#if 0
char *config_read(char *variable)
{
	char *value;

	value=getenv(variable);
	printf("%s=%s\n", variable, value);
	return(value);
}

int config_load(void)
{
	char line[80];
	int i;
	FILE *fr;

	fr=fopen(configfile(), "rt");
	if (fr == NULL)
		return -1;
	printf("opened\n");
	while (fgets(line, 80, fr) != NULL) {
		i=strlen(line);
		printf("len == %d\n", i);
		while (isspace(line[i-1])) {
			line[--i]='\0';
		}
		printf("putenv(\"%s\")\n", line);
		if (putenv(line)){
			fprintf(stderr,\
			"[emux51]\twarning:\tcannot putenv()");
		}
	}
	fclose(fr);
}
#endif
