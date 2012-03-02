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
#include <glib/gstdio.h>


#define CFGNAME "emuxrc"
#define CFGDIR ".emux51"

#ifndef HOME_VAR
	#define HOME_VAR "HOME"
#endif
#define STRLEN(s) (sizeof(s)/sizeof(s[0]))

const char *known_vars[]={ "module_dir", "hex_dir", NULL };

char _configfile[PATH_MAX+3+STRLEN(CFGNAME)+STRLEN(CFGDIR)];

char *configfile(void)
{
	char *dir=getenv(HOME_VAR);

	if (dir)
		sprintf(_configfile, "%s%c%s%c%s",
			dir, G_DIR_SEPARATOR, CFGDIR, G_DIR_SEPARATOR, CFGNAME);
	else
		sprintf(_configfile, "./.%s", CFGNAME);
	return _configfile;
}

int config_parse(void)
{
	char *buff;
	int len;
	FILE *fr;

	fr=g_fopen(configfile(), "rt");
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

/*		printf("putenving %s\n", buff);*/
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

int config_save(void)
{
	int i;
	char *value;
	FILE *fw;

	/*	open file	*/
	fw=g_fopen(configfile(), "wt");
	if (fw == NULL){
		printf("[emux]\tcan't save config to file %s\n", configfile());
		return -1;
	}
	/*	save all known variables	*/
	for (i=0; known_vars[i]!=NULL; i++) {
		value=getenv(known_vars[i]);
		if (value) {
			fprintf(fw, "%s=%s\n", known_vars[i], value);
		}
	}
	fclose(fw);
	printf("[emux]\tconfig has been saved to file %s\n", configfile());
	return 0;
}


