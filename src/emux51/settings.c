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

		printf("putenving %s\n", buff);
		if (putenv(buff)){
			/* probably OOM */
			g_free(buff);
			fclose(fr);
			return -2;
		}
	}
	fclose(fr);
	return 0;
}


