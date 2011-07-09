#include <stdio.h>
#include <hex.h>

/*	TODO: checksum, segments and other Intel HEX features	*/
int load_hex(const char *file, unsigned char *dest, unsigned int dest_len)
{
	FILE *fr;
	char buff[80];

	int ok;
	unsigned size;
	unsigned offset;
	unsigned type;
	unsigned segment=0;

	int i;
	unsigned int data;
	unsigned int sum=0;
	/*	source file and destination index	*/
	if (!(file && dest))
		return -1;
	printf("[emux]\tfile == %s\n", file);
	fr=fopen(file, "rt");
	if (fr == NULL){
		perror("[emux]\tfopen");
		return -2;
	}

	while (1 == fscanf(fr, "%s", buff)){
		if (buff[0] != ':') {
			fprintf(stderr, "[emux]\tbad format\n");
			return -3;
		}
		ok=sscanf(buff, ":%2x%4x%2x", &size, &offset, &type);
		if (ok != 3) {
			fprintf(stderr, "[emux]\tbad format\n");
			return -4;
		}
		switch(type){
			case 0:
			sum=0;
			if (16*segment+offset+size >= dest_len){
				fprintf(stderr, "[emux]\tout of bounds\n");
				return -1;
			}
			for (i=0; i<size; i++) {
				sscanf(buff+9+2*i, "%2x", &data);
				sum+=data;
				dest[16*segment+offset+i]=data;


			}
			break;
			case 1:
				return 0;
			default:
				return 3;
		}
	}
	return 2;
}
