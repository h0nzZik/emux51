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
	sprintf(_configfile, "%s/%s/%s", getenv("HOME"), CFGDIR, CFGNAME);
	return _configfile;
}

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
