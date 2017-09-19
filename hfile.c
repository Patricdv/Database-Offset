#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
	The Data schema is organized as follows:
 - the file starts with an int that represent de dataset size.
 - the next int * size  represents the vector os spaces in file.
 - Then, the next 15 characters bytes represent field name counting \0
 - Next 1 character byte the type of the field (S string, C character and I integer)
 - Next int bytes, the lenght of the field

	The schema is located in the header of data file and a header can have at most 10 field definitions.
	In the case that there are less than 10 field definitions, the next empty definition will have character # in the name of the field.
*/

#define MFIELD 10
#define REGISTERS 100

void buildHeader() {
	char fname[15], ftype;
	int flen, count, ffalse = 0;
	FILE *f;
	f = fopen("database", "w+");
	if (f == NULL) {
		exit(0);
  }

	// first field (attribute Código)
  strcpy(fname, "codigo");
  ftype = 'S';
  flen = 5;
  fwrite(fname, 15, 1, f);
  fwrite(&ftype, 1, 1, f);
  fwrite(&flen, sizeof(int), 1, f);

	// second field (attribute name)
  strcpy(fname, "nome");
  ftype = 'S';
  flen = 20;
  fwrite(fname, 15, 1, f);
  fwrite(&ftype, 1, 1, f);
  fwrite(&flen, sizeof(int), 1, f);

	// third field (attribute age)
  strcpy(fname, "idade");
  ftype = 'I';
  flen = sizeof(int);
  fwrite(fname, 15, 1, f);
  fwrite(&ftype, 1, 1, f);
  fwrite(&flen, sizeof(int), 1, f);

	// The other 7 must have the flag #
  strcpy(fname, "#");
  fwrite(fname, 15 + 1 + sizeof(int), MFIELD - 3, f);

	// Write on file the information about the fields
	fwrite(&ffalse, sizeof(int), 1, f);
	for (count = 0; count < REGISTERS; count++) {
		fwrite(&ffalse, sizeof(bool), 1, f);
		fwrite(&ffalse, sizeof(int), 1, f);
	}

	fclose(f);
}

struct theader {
	char name[15];
	char type;
	int  len;
};

struct tdataset {
	int entries;
	int dataset[REGISTERS];
};

union tint {
	char cint[sizeof(int)];
	int vint;
};

struct theader *readHeader() {
	FILE *f;
	struct theader *th = (struct theader*) malloc(sizeof(struct theader) * MFIELD);
	int i;

	f = fopen("database", "r");
	if (f == NULL) {
		printf("File not found\n");
		exit(0);
  }

	for (i = 0; i < MFIELD; i++) {
		fread(th[i].name, 15, 1, f);
		fread(&th[i].type, 1, 1, f);
		fread(&th[i].len, sizeof(int), 1, f);
	}

	fclose(f);
	return th;
}

struct tdataset *readDataset() {
	FILE *f;
	struct tdataset *tds = (struct tdataset*) malloc(sizeof(struct tdataset));
	int i;

	f = fopen("database", "r");
	if (f == NULL) {
		printf("File not found\n");
		exit(0);
  }

	fread(&tds->entries, sizeof(int), 1, f);
	for (i = 0; i < REGISTERS; i++) {
		fread(&tds->dataset[i], sizeof(bool), 1, f);
	}

	fclose(f);
	return tds;
}

void insert() {
  FILE *f;
	struct theader *t;
	struct tdataset *tds;
	int i;
	char opt, buf[100], c;
	union tint eint;

	t = readHeader();
	tds = readDataset();
	f = fopen("database", "w+");
	if (f == NULL){
		printf("File not found\n");
		exit(0);
  }
  
	do {
		i = 0;
		while (i < 10 && t[i].name[0] != '#') {
			printf("\n%s :", t[i].name);
			switch (t[i].type) {
				case 'S':
					__fpurge(stdin);
					fgets(buf, t[i].len + 1, stdin);
				  if (buf[strlen(buf) - 1] != '\n') {
						c = getchar();
					}	else {
						buf[strlen(buf) - 1] = 0;
					}

				  fwrite(buf, t[i].len, 1, f);
				  break;

				case 'C':
					fflush(stdin);
					buf[0] = fgetc(stdin);
				  while((c = getchar()) != '\n' && c != EOF); /// garbage collector
				  fwrite(buf, t[i].len, 1, f);
				  break;

				case 'I':
					scanf("%d", &eint.vint);
				  while((c = getchar()) != '\n' && c != EOF); /// garbage collector
				  fwrite (&eint.vint, t[i].len, 1, f);
				  break;
		  }

		  i++;
	  }

	  printf("Continuar (S/N): ");
		opt = getchar();
	  while((c = getchar()) != '\n' && c != EOF); /// garbage collector
	} while (opt=='S' || opt=='s');

	fclose(f);
}

void selectAll() {
  FILE *f;
	struct theader *t;
	struct tdataset *tds;
	int hlen, i, j, space;
	char buf[100];
	union tint eint;

	t = readHeader();
	tds = readDataset();

	f = fopen("database", "r");
	if (f == NULL) {
		printf("File not found\n");
		exit(0);
  }

	// read record a record
  i = 0;
  while (i < 10 && t[i].name[0] != '#') {
  	printf("%s ", t[i].name);
    space = t[i].len - strlen(t[i].name);
    for (j = 1; j <= space; j++) {
      printf(" ");
		}

		i++;
  }

	printf("\n");

	hlen = (MFIELD * (15 + 1 + sizeof(int))) + sizeof(int) + (REGISTERS * (sizeof(bool)));

	/// skip file's header
  fseek(f, hlen, SEEK_SET);
  do {
		i = 0;
		while (i < 10 && t[i].name[0] != '#') {
  		if (!fread(buf, t[i].len, 1, f)) break;
			switch (t[i].type) {
				case 'S':
					for (j = 0; j < t[i].len && buf[j] != 0; j++) {
						printf("%c", buf[j]);
					}

			    space = t[i].len - j;
          for (j = 0; j <= space; j++) {
            printf(" ");
					}
				  break;
				case 'C':
					printf("%c ", buf[0]);
				  break;
				case 'I':
					for (j = 0; j < t[i].len; j++) {
						eint.cint[j] = buf[j];
					}
					printf("%d",eint.vint);
					break;
		  }

			i++;
	  }
		printf("\n");
  } while (!feof(f));
}

void menu() {
	printf("\n\n|--------- Here's the menu: -------|");
	printf("\n1 - Create Database *use only on the first run*");
	printf("\n2 - Select all records");
	printf("\n3 - Insert a record");
	printf("\n4 - Delete a record");
	printf("\n5 - Exit program\n");
}

int main() {
	int option = 0;

	while (true) {
		menu();
		scanf("%d", &option);

		switch(option) {
			case 1:
				buildHeader();
				printf("\nDatabase created successfully!\n");
				break;

			case 2:
				printf("\n------------------------------\n");
				selectAll();
				break;

			case 3:
				printf("\n------------------------------\n");
				insert();
				break;

			case 4:
				printf("\n------------------------------\n");
				break;

			case 5:
				return 0;

			default:
				printf("\nTry again!\n");
		}
	}
}
