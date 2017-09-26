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

void buildStandardHeader() {
	char fname[15], ftype;
	int flen;
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

	fclose(f);
}

void buildManualHeader() {
	char fname[15], ftype, buf[100], c;
	int flen, count, option;
	FILE *f;
	f = fopen("database", "w+");
	if (f == NULL) {
		exit(0);
  }

	for(count = 1; count <= 10; count++) {
		__fpurge(stdin);
		printf("\nType the attribute name:");
		fgets(buf, 15 + 1, stdin);
		if (buf[strlen(buf) - 1] != '\n') {
			c = getchar();
		}	else {
			buf[strlen(buf) - 1] = 0;
		}
		fwrite(buf, 15, 1, f);

		fflush(stdin);
		printf("\nType the attribute type:");
		buf[0] = fgetc(stdin);
		while((c = getchar()) != '\n' && c != EOF); /// garbage collector
		fwrite(buf, 1, 1, f);

		if (buf[0] == 'C') {
			flen = 1;
		} else if (buf[0] == 'I') {
			flen = sizeof(int);
		} else if (buf[0] == 'S') {
			printf("\nType the attribute length:");
			scanf("%d", &flen);
		}
	  fwrite(&flen, sizeof(int), 1, f);

		if (count != 10) {
			printf("\nWant to continue? (0 - Yes | 1 - No)");
			scanf("%d", &option);
			if (option == 1) {
				break;
			}
		}
	}

	// The other 7 must have the flag #
  strcpy(fname, "#");
  fwrite(fname, 15 + 1 + sizeof(int), MFIELD - count, f);

	fclose(f);
}

struct theader {
	char name[15];
	char type;
	int  len;
};

struct tdataset {
	int datasetSize;
	bool dataset[];
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
	struct tdataset *tds;
	int i, hlen, datasetSize;

	f = fopen("database", "r");
	if (f == NULL) {
		printf("File not found\n");
		exit(0);
  }

	hlen = MFIELD * (15 + 1 + sizeof(int));
	fseek(f, hlen, SEEK_SET);

	fread(&datasetSize, sizeof(int), 1, f);
	tds = (struct tdataset*) malloc(sizeof(struct tdataset) + datasetSize * sizeof(bool));
	tds->datasetSize = datasetSize;

	for (i = 0; i < datasetSize; i++) {
		fread(&tds->dataset[i], sizeof(bool), 1, f);
	}

	fclose(f);
	return tds;
}

int getRegisterLenght(struct theader *t){
	int i, length = 0;

	for (i = 0; i < MFIELD; i++) {
		if (t[i].name[0] == '#') {
			break;
		}
		length += t[i].len;
	}

	return length;
}

void buildHeader() {
	FILE *f;
	int hlen, choice, count, fieldsSize, fieldsQuantity, utilSpace, spaceUsed, datasetSize;
	struct theader *t;
	bool ffalse = false;

	printf("\n\nLoad a standard header? (0 - Yes | 1 - No)\n");
	scanf("%d", &choice);

	if (choice == 0) {
		buildStandardHeader();
	} else {
		buildManualHeader();
	}

	t = readHeader();
	fieldsSize = getRegisterLenght(t);
	spaceUsed = (MFIELD * (15 + 1 + sizeof(int))) + sizeof(int);
	utilSpace = (4096 - spaceUsed);
	fieldsQuantity = utilSpace / fieldsSize;
	spaceUsed += (fieldsSize * fieldsQuantity);

	while (true) {
		datasetSize = fieldsQuantity * sizeof(bool);
		if ((spaceUsed + datasetSize) <= 4096) {
			break;
		}
		fieldsQuantity--;
		spaceUsed -= fieldsSize;
	}

	f = fopen("database", "rb+");
	if (f == NULL) {
		printf("File not found\n");
		exit(0);
	}

	hlen = MFIELD * (15 + 1 + sizeof(int));
	fseek(f, hlen, SEEK_SET);

	fwrite(&fieldsQuantity, sizeof(int), 1, f);
	for (count = 0; count < fieldsQuantity; count++) {
		fwrite(&ffalse, sizeof(bool), 1, f);
	}

	fclose(f);
}

void insert() {
  FILE *f;
	struct theader *t;
	struct tdataset *tds;
	int i, registerLength, hlen, count = 0;
	char opt, buf[100], c;
	union tint eint;

	t = readHeader();
	tds = readDataset();
	registerLength = getRegisterLenght(t);

	f = fopen("database", "rb+");
	if (f == NULL){
		printf("File not found\n");
		exit(0);
  }

	hlen = (MFIELD * (15 + 1 + sizeof(int))) + sizeof(int) + (tds->datasetSize * sizeof(bool));
	fseek(f, hlen, SEEK_SET);

	do {
		for (; count < tds->datasetSize; count++) {
			if (tds->dataset[count] == false) {
				tds->dataset[count] = true;
				fseek(f, hlen, SEEK_SET);
				printf("\n %d -> %ld", count, ftell(f));
				break;
			}

			hlen += registerLength;
		}

		for (i = 0; i < 10 && t[i].name[0] != '#'; i++) {
			printf("\n%s: ", t[i].name);
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
	  }

	  printf("Continuar (s/n): ");
		opt = getchar();
	  while((c = getchar()) != '\n' && c != EOF); /// garbage collector
	} while (opt=='S' || opt=='s');

	hlen = (MFIELD * (15 + 1 + sizeof(int))) + sizeof(int);
	fseek(f, hlen, SEEK_SET);

	for (count = 0; count < tds->datasetSize; count++) {
		fwrite(&tds->dataset[count], sizeof(bool), 1, f);
	}

	fclose(f);
}

void selectAll() {
  FILE *f;
	struct theader *t;
	struct tdataset *tds;
	int hlen, registerLength, count, i, j, space;
	char buf[100];
	union tint eint;

	t = readHeader();
	tds = readDataset();
	registerLength = getRegisterLenght(t);

	f = fopen("database", "r");
	if (f == NULL) {
		printf("File not found\n");
		exit(0);
  }

	// read record a record
  for (i = 0; i < MFIELD && t[i].name[0] != '#'; i++) {
  	printf("%s ", t[i].name);
    space = t[i].len - strlen(t[i].name);
    for (j = 1; j <= space; j++) {
      printf(" ");
		}
  }

	printf("\n");

	hlen = (MFIELD * (15 + 1 + sizeof(int))) + sizeof(int) + (tds->datasetSize * sizeof(bool));

	/// skip file's header
  fseek(f, hlen, SEEK_SET);

  for (count = 0; count < tds->datasetSize; count++) {
		if (tds->dataset[count] == true) {
			for (i = 0; i < 10 && t[i].name[0] != '#'; i++) {
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
			}

			printf("\n");
		}
		hlen += registerLength;
		fseek(f, hlen, SEEK_SET);
	}
}

void deleteRegister() {
	FILE *f;
	struct theader *t;
	struct tdataset *tds;
	int registerLength, hlen, count, record = 0;
	char opt, buf[100], c;
	union tint eint;

	t = readHeader();
	tds = readDataset();
	registerLength = getRegisterLenght(t);

	f = fopen("database", "rb+");
	if (f == NULL){
		printf("File not found\n");
		exit(0);
  }

	while (true) {
		printf("\nType the number of the record to be deleted:");
		scanf("%d", &record);

		if (record > 0 && record <= tds->datasetSize) {
			break;
		}
	}

	tds->dataset[record - 1] = false;

	hlen = (MFIELD * (15 + 1 + sizeof(int))) + sizeof(int);
	fseek(f, hlen, SEEK_SET);

	for (count = 0; count < tds->datasetSize; count++) {
		fwrite(&tds->dataset[count], sizeof(bool), 1, f);
	}

	fclose(f);
}

void menu() {
	printf("\n\n|--------- Here's the menu: -------|");
	printf("\n1 - Create Database *use only on the first run*");
	printf("\n2 - Insert data");
	printf("\n3 - Select all records");
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
				printf("\n\n\nDatabase created successfully!\n");
				break;

			case 2:
				printf("\n\n\n----- Insert Fields: ------\n");
				insert();
				break;

			case 3:
				printf("\n\n\n----- Selecting All: -----\n");
				selectAll();
				break;

			case 4:
				printf("\n\n\n----- Delete a Register: -------\n");
				deleteRegister();
				break;

			case 5:
				return 0;

			default:
				printf("\nTry again!\n");
		}
	}
}
