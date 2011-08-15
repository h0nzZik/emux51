/*
 *	settings.c - simple support for loading settings.
 *	WARNING: This file is written quick&dirty.
 */

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <glib.h>


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

#if 0
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
int config_parse(void)
{
	char *buff;
	int len;
	FILE *fr;

	fr=fopen(configfile(), "rt");
	if (fr == NULL)
		return -1;

	 while(1){
		buff=g_malloc(80);
		if(!fgets(buff, 80, fr))
			break;
		len=strlen(buff);
		/*	ignore comments	*/
		if(buff[0] == '#') {
			g_free(buff);
			continue;
		}
		/*	EOL and so..	*/
		while(isspace(buff[--len]))
			buff[len]='\0';

		if (putenv(buff)){
			/* probably OOM */
			g_free(buff);
			return -2;
		}
	}
	return 0;
}


