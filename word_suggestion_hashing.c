#include <stdio.h>
#include <stdlib.h>
#define SIZE 997					//Tablo B�y�kl���.
#define WORD_BUFFER 50				//Al�nan kelime i�in maks boyut.
#define HORNER_NUMBER 11			//Horner metodundaki kat say�.
#define SENTENCE_BUFFER 1000

//S�zl���m�z�n ve yanl�� kelimelerimiz ayn� anda bulunmayacakt�r.
//istedi�iniz gibi 2 Farkl� hashtable olu�turulacak.
//Sadece ekstra tan�mlama yapmamak i�in b�yle olu�turdum.
typedef struct hash {
	char word[WORD_BUFFER];			//Kelime
	char wrongWord[WORD_BUFFER];	//Yanl�� kelime 
}HASH;

typedef struct hashtable {
	HASH table[SIZE];				//Hash tablosu
	float loadfactor;				//doluYer/SIZE
	int indexCounter;				//doluYer
}HASHTABLE;

void checkWord(HASHTABLE* hashTable, char word[WORD_BUFFER], HASHTABLE* wrongHashTable);	//Kelime s�zl�kte var m� kontrol eder
void checkSentence(HASHTABLE* hashTable, char sentence[SENTENCE_BUFFER], HASHTABLE* wrongHashTable);	//C�mleyi par�alar checkWord fonksiyonuna verir.
void prepareHashTable(HASHTABLE* hashTable);					//HashTablosunda alana tan�mlamalar� ve default de�erler ayarlan�r.
int hash1(long unsigned int key);								//1. hash fonksiyonu  h1(key) = key mod M	(index d�ner).
int hash2(long unsigned int key);								//2. hash fonksiyonu  h2(key) = 1 + (key mod MM)	(index d�ner).
int insertToHash(HASHTABLE* hashTable, long unsigned int key, char wrd[WORD_BUFFER]);		//Kelimeleri HashTablosuna almak i�in.	 (Tablonun dolup dolmad��� d�ner.)
long unsigned int horner(char word[WORD_BUFFER]);					//Kelimelerin indexini hesaplarken say�ya d�n���m� i�in.	(kelimenin say� hali d�ner).
void printHashTable(HASHTABLE* hashTable);							//Debug i�in hash tablosu yazd�rma.
HASHTABLE* readFile(FILE* fp);							   	//Dosyadan hash tablosu okuma.	(Hash tablosu d�ner.)
int** initMatrix(int x, int y);								//Edit Distance matrisi i�in alan a�ma	(Matris d�ner)
int findSmallest(int num1, int num2, int num3);				//Minimum distance � se�mek i�in (Minimum distance d�ner)
void insertToWrongHashTable(HASHTABLE* hashTable, unsigned long int key, char word[WORD_BUFFER], char wrongWord[WORD_BUFFER]); //Yanl�� kelimeleri hash tablosuna eklemek i�in
int checkWrongWords(HASHTABLE* hashTable ,char word[WORD_BUFFER]);	//Yanl�� kelimelerin hashtablosu kontrol�.

int main() {

	int choice;		//Men� se�imleri i�in
	int quit = 0;	//��k�� i�in
	char sentence[SENTENCE_BUFFER];	//C�mle
	char temp;		//scanf den ta�anlar� atlamak i�in.
	HASHTABLE* hashTable;			//Hash tablosu
	HASHTABLE* wrongHashTable = (HASHTABLE*)malloc(SIZE * sizeof(HASHTABLE));		//Yanl�� kelimeler icin hash tablosu
	FILE* hashFile = fopen("smallDictionary.txt", "r+");		//Haz�r hash tablosu var m� kontrol�.
	if (hashFile != NULL) {
		printf("Sozluk bulundu bulundu. Yukleniyor.\n");
		hashTable = readFile(hashFile);				//Hash tablosu okunur.
		fclose(hashFile);
	}
	else {
		printf("Sozluk bulunamadi. \n");
		return 0;
	}
	prepareHashTable(wrongHashTable);	//Yanl�� kelimeler i�in HashTablosu tan�mlar�

	while (!quit) {
		printf("\nLoadfactor : %f\n1. Cumle Yazmak\n2.Hash Tablosunu yazdir\n3.Yanlis kelime hash tablosu yazdir.\n4.Cikis\n", hashTable->loadfactor);
		scanf("%d", &choice);
		system("CLS");
		if (choice == 1) {
			scanf("%c", &temp);     // buffer temizlendi
			scanf("%[^\n]", sentence);
			scanf("%c", &temp); 	// buffer temizlendi
			printf("Cumle : %s \n", sentence);

			checkSentence(hashTable, sentence, wrongHashTable);	//C�mle kontrol ediliyor

		}
		else if (choice == 2) {
			printHashTable(hashTable);		//Debug i�in yazd�rma i�lemi
		}
		else if(choice ==3){
			printHashTable(wrongHashTable); //Debug i�in yazd�rma i�lemi
		}
		else {
			quit = 1;					//��k��
		}
	}
	return 0;
}
int findSmallest(int num1, int num2, int num3) {
	if (num1 < num2 && num1 < num3)
	{
		return num1;
	}
	else if (num2 < num3)
	{
		return num2;
	}
	else
	{
		return num3;
	}
}
int** initMatrix(int x, int y) {
	int i;
	int** matrix = (int**)calloc(x, sizeof(int*));
	if (!matrix) {
		printf("Matrix Allocate edilemedi.");
		return NULL;
	}
	for (i = 0; i < x; i++) {
		matrix[i] = (int*)calloc(y, sizeof(int));
	}
	return matrix;
}

int editDistance(char word1[WORD_BUFFER], char word2[WORD_BUFFER]) {
	int i, j;
	int x = strlen(word1) + 1;	//Matrisin row say�s�
	int y = strlen(word2) + 1;	//Matrisin column say�s�
	int** ED = initMatrix(x, y);//Matris olu�turma

	char c1, c2;
	if (!ED) {
		printf("Matrix Allocate edilemedi.");
		return 0;
	}
	for (i = 0; i < y; i++) {
		ED[0][i] = i;			//�lk sat�r� 0 yapma
	}
	for (j = 0; j < x; j++) {
		ED[j][0] = j;			//ilk sutunu 0 yapma
	}

	for (i = 1; i < x; i++) {
		c1 = word1[i - 1];		//ilk kelimenin harfini tutar
		for (j = 1; j < y; j++) {
			c2 = word2[j - 1];	//ikinci kelimenin harfini tutar
			if (c1 == c2) {
				ED[i][j] = ED[i - 1][j - 1];	//E�itse �aprazdan al�r�z.
			}
			else {
				//E�it de�ilse en k���k olan� se�eriz
				ED[i][j] = findSmallest(ED[i - 1][j], ED[i][j - 1], ED[i - 1][j - 1]) + 1;
			}
		}
	}
	return (ED[x-1][y-1]);	//En sa� alt de�er d�ner.
}

void checkWord(HASHTABLE* hashTable, char word[WORD_BUFFER], HASHTABLE* wrongHashTable) {
	int i = 0;	//Double hashinde ilerlemek i�in
	int tmp;	//Distance kar��la�t�rmak i�in
	int counter = 0;	//�nerilen kelimeleri saymak i�in
	char arr[100][SENTENCE_BUFFER];	//�nerilen kelimeleri tutar
	int choice;		//�nerilen kelimeden se�im yapmak.
	int index;		//hash fonksiyonundan d�necek de�eri tutucak.
	long unsigned int key = horner(word); 	//Kelimenin say� kar��l��� al�nd�.
	do {
		index = (hash1(key) + (i * hash2(key))) % SIZE;	//Hash fonksiyonlar�yla indexi bulundu.
		i++;
	} while (strcmp(hashTable->table[index].word, "-") && strcmp(hashTable->table[index].word, word) && i < SIZE);
	//Yukardaki d�ng� bo� bir yer veya ayn� kelimeyi bulana kadar d�ner.

	if (!strcmp(hashTable->table[index].word, "-")) {			//Ayn� kelime tabloda yoksa.
		//Daha �nce se�ilmi� d�zeltme var m� kontol edilir.
		if(checkWrongWords(wrongHashTable, word)){
			return;
		}
		int minDistance = 3;	//3 ten k���k distancelar� almak i�in
		printf("\n %s'e yakin kelimelerden seciniz : ", word);
		for (i = 0; i < SIZE; i++) {
			if (strcmp(hashTable->table[i].word, "-")) {
				tmp = editDistance(word, hashTable->table[i].word);
				if (tmp < minDistance) {
					printf("\n%d. %s  uzaklik = %d ", counter+1, hashTable->table[i].word, tmp);
					strcpy(arr[counter], hashTable->table[i].word);	//Dizide saklar�z �nermeleri.
					counter++;
				}
			}
		}
		if(counter != 0){
			scanf("%d", &choice);	//�nerilen kelimelerden se�im yap�l�r.
			key = horner(word);		//Kelime say�ya �evrilir.
			insertToWrongHashTable(wrongHashTable, key, arr[choice-1], word);	//Hash Table a al�n�r.
		}	
		else{
			printf("Onerilen kelime yok.");
		}
			
	}
	
}

int checkWrongWords(HASHTABLE* hashTable ,char word[WORD_BUFFER]){
	int i = 0;
	int index;										//hash fonksiyonundan d�necek de�eri tutucak.
	long unsigned int key = horner(word); 					//Kelimenin say� kar��l��� al�nd�.
	do {
		index = (hash1(key) + (i * hash2(key))) % SIZE;		//Hash fonksiyonlar�yla indexi bulundu.
		i++;
	} while (strcmp(hashTable->table[index].wrongWord , "-") && strcmp(hashTable->table[index].wrongWord, word) && i < SIZE);
	//Yukardaki d�ng� bo� bir yer veya ayn� kelimeyi bulana kadar d�ner.
	if(!strcmp(hashTable->table[index].wrongWord, word)){
		//Varsa daha �nce se�ilmi� kelimeyi g�sterir.
		printf("%s'e  secilmis onerilen kelime : %s\n", hashTable->table[index].wrongWord, hashTable->table[index].word);
		return 1;
	}
	return 0;
}
void insertToWrongHashTable(HASHTABLE* hashTable, unsigned long int key, char word[WORD_BUFFER], char wrongWord[WORD_BUFFER]){
	
	int i;
	int index;															//Hashten d�nen index.
	if (hashTable->loadfactor < 1) {
		if (hashTable->loadfactor > 0.8) {
			printf("LOADFACTOR 0.8 den b�y�k. UYARI!!!!");				//LoadFactor kontrol�.
		}
		i = 0;
		do {
			index = (hash1(key) + (i * hash2(key))) % SIZE;				//Hash fonksiyonlar�ndan d�nen index.
			i++;
		} while (strcmp(hashTable->table[index].wrongWord, "-") && strcmp(hashTable->table[index].wrongWord, wrongWord) && i < SIZE);	//Bo� alan veya kelimenin ayn�s�n� bulunca ��k�cak.

		if (!strcmp(hashTable->table[index].word, "-")) {
			strcpy(hashTable->table[index].word, word);			//Bo� yer bulundu.
			strcpy(hashTable->table[index].wrongWord, wrongWord);
			(hashTable->indexCounter)++;						//Bo� yeri doldurdu�umuz i�in art�rd�k.
			hashTable->loadfactor = (float)(hashTable->indexCounter) / SIZE;		//Loadfactor g�ncellenir.		
		}
	}
	else {
		printf("TABLO DOLU.");
		return 0;
	}
	return 1;
	
}

void checkSentence(HASHTABLE* hashTable, char sentence[SENTENCE_BUFFER], HASHTABLE* wrongHashTable) {
	int i = 0, j = 0;
	char word[WORD_BUFFER];	//C�mleden ald���m�z kelimeleri tutar
	while (sentence[i] != NULL) {
		if (sentence[i] != ' ') {
			word[j] = sentence[i];	//Bo�luk karakteri gelene kadar word'te biriktiririz.
			j++;
		}
		else {
			//Kelimeyi ald�k
			word[j] = NULL;
			j = 0;
			checkWord(hashTable, word, wrongHashTable);	//Kelime kontrol�
			strcpy(word, "");
		}
		i++;
	}
	word[j] = NULL;
	checkWord(hashTable, word, wrongHashTable);	//Son kelime de kontol edilir.

}
void printHashTable(HASHTABLE* hashTable) {
	int i;
	for (i = 0; i < SIZE; i++) {
		printf("\n%d. %s - %s\n", i + 1, hashTable->table[i].word, hashTable->table[i].wrongWord);
	}
}

void findWord(HASHTABLE* hashTable, char wrd[WORD_BUFFER]) {
	int i = 0;
	int index;										//hash fonksiyonundan d�necek de�eri tutucak.
	long unsigned int key = horner(wrd); 					//Kelimenin say� kar��l��� al�nd�.
	do {
		index = (hash1(key) + (i * hash2(key))) % SIZE;		//Hash fonksiyonlar�yla indexi bulundu.
		i++;
	} while (strcmp(hashTable->table[index].word, "-") && strcmp(hashTable->table[index].word, wrd) && i < SIZE);
}

HASHTABLE* readFile(FILE* fp) {
	int i, key, flag = 1;
	char buffer[WORD_BUFFER];										//Dosyadan okunan kelimeleri tutan de�i�ken.
	HASHTABLE* hashTable = (HASHTABLE*)malloc(sizeof(HASHTABLE));	//Hash tablosu i�in alan a��l�r.
	prepareHashTable(hashTable);
	while (flag && fscanf(fp, "%s", buffer) != EOF) {
		key = horner(buffer);									//Kelime say�ya �evrilir.
		flag = insertToHash(hashTable, key, buffer);	//Hash tablosuna eklenir. Flag hash tablosunun dolup dolmad���n� kontrol eder.
	}
	if (!flag) {
		printf("Tablo doldu. Ta�an veriler : \n");
		while (fscanf(fp, "%s", buffer) != EOF) {		//Ta�an veriler yazd�r�l�r.
			printf("%s", buffer);
		}
	}
	return hashTable;
}

void prepareHashTable(HASHTABLE* hashtable) {
	int i;
	hashtable->indexCounter = 0;
	hashtable->loadfactor = 0;
	for (i = 0; i < SIZE; i++) {
		strcpy(hashtable->table[i].word, "-");		
		strcpy(hashtable->table[i].wrongWord, "-");
	}

}

long unsigned int horner(char word[WORD_BUFFER]) {
	int i = 0;
	long unsigned int total = 0;								//Toplam� tutucak.
	long int multiple = 1;							//Her harfin �arp�lca�� de�i�ken.
	char letter;
	while (word[i] != NULL) {
		if (word[i] >= 65 && word[i] <= 90) {
			letter = word[i] - 'A';					//B�y�k harfse 'A' ��kar�l�r.
		}
		else {
			letter = word[i] - 'a';					//K���k harfse 'a' ��kar�l�r.
		}
		//En sonra olu�ucak say�y� k���ltm�� oldum.
		total = total + (letter * multiple);
		multiple *= HORNER_NUMBER;
		i++;
	}
	return total;
}


int insertToHash(HASHTABLE* hashTable, long unsigned int key, char wrd[WORD_BUFFER]) {
	int i;
	int index;															//Hashten d�nen index.
	if (hashTable->loadfactor < 1) {
		if (hashTable->loadfactor > 0.8) {
			printf("LOADFACTOR 0.8 den b�y�k. UYARI!!!!");				//LoadFactor kontrol�.
		}
		i = 0;
		do {
			index = (hash1(key) + (i * hash2(key))) % SIZE;				//Hash fonksiyonlar�ndan d�nen index.
			i++;
		} while (strcmp(hashTable->table[index].word, "-") && strcmp(hashTable->table[index].word, wrd) && i < SIZE);	//Bo� alan veya kelimenin ayn�s�n� bulunca ��k�cak.

		if (!strcmp(hashTable->table[index].word, "-")) {
			strcpy(hashTable->table[index].word, wrd);			//Bo� yer bulundu.
			(hashTable->indexCounter)++;						//Bo� yeri doldurdu�umuz i�in art�rd�k.
			hashTable->loadfactor = (float)(hashTable->indexCounter) / SIZE;		//Loadfactor g�ncellenir.		
		}
	}
	else {
		printf("TABLO DOLU.");
		return 0;
	}
	return 1;
}


int hash1(long unsigned int key) {
	return (key % SIZE);													//Birinci hash fonksiyonu.
}

int hash2(long unsigned int key) {
	return (1 + (key % (SIZE - 1)));										//�kinci hash fonksiyonu.
}
