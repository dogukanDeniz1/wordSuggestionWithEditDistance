#include <stdio.h>
#include <stdlib.h>
#define SIZE 997					//Tablo Büyüklüðü.
#define WORD_BUFFER 50				//Alýnan kelime için maks boyut.
#define HORNER_NUMBER 11			//Horner metodundaki kat sayý.
#define SENTENCE_BUFFER 1000

//Sözlüðümüzün ve yanlýþ kelimelerimiz ayný anda bulunmayacaktýr.
//istediðiniz gibi 2 Farklý hashtable oluþturulacak.
//Sadece ekstra tanýmlama yapmamak için böyle oluþturdum.
typedef struct hash {
	char word[WORD_BUFFER];			//Kelime
	char wrongWord[WORD_BUFFER];	//Yanlýþ kelime 
}HASH;

typedef struct hashtable {
	HASH table[SIZE];				//Hash tablosu
	float loadfactor;				//doluYer/SIZE
	int indexCounter;				//doluYer
}HASHTABLE;

void checkWord(HASHTABLE* hashTable, char word[WORD_BUFFER], HASHTABLE* wrongHashTable);	//Kelime sözlükte var mý kontrol eder
void checkSentence(HASHTABLE* hashTable, char sentence[SENTENCE_BUFFER], HASHTABLE* wrongHashTable);	//Cümleyi parçalar checkWord fonksiyonuna verir.
void prepareHashTable(HASHTABLE* hashTable);					//HashTablosunda alana tanýmlamalarý ve default deðerler ayarlanýr.
int hash1(long unsigned int key);								//1. hash fonksiyonu  h1(key) = key mod M	(index döner).
int hash2(long unsigned int key);								//2. hash fonksiyonu  h2(key) = 1 + (key mod MM)	(index döner).
int insertToHash(HASHTABLE* hashTable, long unsigned int key, char wrd[WORD_BUFFER]);		//Kelimeleri HashTablosuna almak için.	 (Tablonun dolup dolmadýðý döner.)
long unsigned int horner(char word[WORD_BUFFER]);					//Kelimelerin indexini hesaplarken sayýya dönüþümü için.	(kelimenin sayý hali döner).
void printHashTable(HASHTABLE* hashTable);							//Debug için hash tablosu yazdýrma.
HASHTABLE* readFile(FILE* fp);							   	//Dosyadan hash tablosu okuma.	(Hash tablosu döner.)
int** initMatrix(int x, int y);								//Edit Distance matrisi için alan açma	(Matris döner)
int findSmallest(int num1, int num2, int num3);				//Minimum distance ý seçmek için (Minimum distance döner)
void insertToWrongHashTable(HASHTABLE* hashTable, unsigned long int key, char word[WORD_BUFFER], char wrongWord[WORD_BUFFER]); //Yanlýþ kelimeleri hash tablosuna eklemek için
int checkWrongWords(HASHTABLE* hashTable ,char word[WORD_BUFFER]);	//Yanlýþ kelimelerin hashtablosu kontrolü.

int main() {

	int choice;		//Menü seçimleri için
	int quit = 0;	//Çýkýþ için
	char sentence[SENTENCE_BUFFER];	//Cümle
	char temp;		//scanf den taþanlarý atlamak için.
	HASHTABLE* hashTable;			//Hash tablosu
	HASHTABLE* wrongHashTable = (HASHTABLE*)malloc(SIZE * sizeof(HASHTABLE));		//Yanlýþ kelimeler icin hash tablosu
	FILE* hashFile = fopen("smallDictionary.txt", "r+");		//Hazýr hash tablosu var mý kontrolü.
	if (hashFile != NULL) {
		printf("Sozluk bulundu bulundu. Yukleniyor.\n");
		hashTable = readFile(hashFile);				//Hash tablosu okunur.
		fclose(hashFile);
	}
	else {
		printf("Sozluk bulunamadi. \n");
		return 0;
	}
	prepareHashTable(wrongHashTable);	//Yanlýþ kelimeler için HashTablosu tanýmlarý

	while (!quit) {
		printf("\nLoadfactor : %f\n1. Cumle Yazmak\n2.Hash Tablosunu yazdir\n3.Yanlis kelime hash tablosu yazdir.\n4.Cikis\n", hashTable->loadfactor);
		scanf("%d", &choice);
		system("CLS");
		if (choice == 1) {
			scanf("%c", &temp);     // buffer temizlendi
			scanf("%[^\n]", sentence);
			scanf("%c", &temp); 	// buffer temizlendi
			printf("Cumle : %s \n", sentence);

			checkSentence(hashTable, sentence, wrongHashTable);	//Cümle kontrol ediliyor

		}
		else if (choice == 2) {
			printHashTable(hashTable);		//Debug için yazdýrma iþlemi
		}
		else if(choice ==3){
			printHashTable(wrongHashTable); //Debug için yazdýrma iþlemi
		}
		else {
			quit = 1;					//Çýkýþ
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
	int x = strlen(word1) + 1;	//Matrisin row sayýsý
	int y = strlen(word2) + 1;	//Matrisin column sayýsý
	int** ED = initMatrix(x, y);//Matris oluþturma

	char c1, c2;
	if (!ED) {
		printf("Matrix Allocate edilemedi.");
		return 0;
	}
	for (i = 0; i < y; i++) {
		ED[0][i] = i;			//Ýlk satýrý 0 yapma
	}
	for (j = 0; j < x; j++) {
		ED[j][0] = j;			//ilk sutunu 0 yapma
	}

	for (i = 1; i < x; i++) {
		c1 = word1[i - 1];		//ilk kelimenin harfini tutar
		for (j = 1; j < y; j++) {
			c2 = word2[j - 1];	//ikinci kelimenin harfini tutar
			if (c1 == c2) {
				ED[i][j] = ED[i - 1][j - 1];	//Eþitse çaprazdan alýrýz.
			}
			else {
				//Eþit deðilse en küçük olaný seçeriz
				ED[i][j] = findSmallest(ED[i - 1][j], ED[i][j - 1], ED[i - 1][j - 1]) + 1;
			}
		}
	}
	return (ED[x-1][y-1]);	//En sað alt deðer döner.
}

void checkWord(HASHTABLE* hashTable, char word[WORD_BUFFER], HASHTABLE* wrongHashTable) {
	int i = 0;	//Double hashinde ilerlemek için
	int tmp;	//Distance karþýlaþtýrmak için
	int counter = 0;	//Önerilen kelimeleri saymak için
	char arr[100][SENTENCE_BUFFER];	//Önerilen kelimeleri tutar
	int choice;		//Önerilen kelimeden seçim yapmak.
	int index;		//hash fonksiyonundan dönecek deðeri tutucak.
	long unsigned int key = horner(word); 	//Kelimenin sayý karþýlýðý alýndý.
	do {
		index = (hash1(key) + (i * hash2(key))) % SIZE;	//Hash fonksiyonlarýyla indexi bulundu.
		i++;
	} while (strcmp(hashTable->table[index].word, "-") && strcmp(hashTable->table[index].word, word) && i < SIZE);
	//Yukardaki döngü boþ bir yer veya ayný kelimeyi bulana kadar döner.

	if (!strcmp(hashTable->table[index].word, "-")) {			//Ayný kelime tabloda yoksa.
		//Daha önce seçilmiþ düzeltme var mý kontol edilir.
		if(checkWrongWords(wrongHashTable, word)){
			return;
		}
		int minDistance = 3;	//3 ten küçük distancelarý almak için
		printf("\n %s'e yakin kelimelerden seciniz : ", word);
		for (i = 0; i < SIZE; i++) {
			if (strcmp(hashTable->table[i].word, "-")) {
				tmp = editDistance(word, hashTable->table[i].word);
				if (tmp < minDistance) {
					printf("\n%d. %s  uzaklik = %d ", counter+1, hashTable->table[i].word, tmp);
					strcpy(arr[counter], hashTable->table[i].word);	//Dizide saklarýz önermeleri.
					counter++;
				}
			}
		}
		if(counter != 0){
			scanf("%d", &choice);	//Önerilen kelimelerden seçim yapýlýr.
			key = horner(word);		//Kelime sayýya çevrilir.
			insertToWrongHashTable(wrongHashTable, key, arr[choice-1], word);	//Hash Table a alýnýr.
		}	
		else{
			printf("Onerilen kelime yok.");
		}
			
	}
	
}

int checkWrongWords(HASHTABLE* hashTable ,char word[WORD_BUFFER]){
	int i = 0;
	int index;										//hash fonksiyonundan dönecek deðeri tutucak.
	long unsigned int key = horner(word); 					//Kelimenin sayý karþýlýðý alýndý.
	do {
		index = (hash1(key) + (i * hash2(key))) % SIZE;		//Hash fonksiyonlarýyla indexi bulundu.
		i++;
	} while (strcmp(hashTable->table[index].wrongWord , "-") && strcmp(hashTable->table[index].wrongWord, word) && i < SIZE);
	//Yukardaki döngü boþ bir yer veya ayný kelimeyi bulana kadar döner.
	if(!strcmp(hashTable->table[index].wrongWord, word)){
		//Varsa daha önce seçilmiþ kelimeyi gösterir.
		printf("%s'e  secilmis onerilen kelime : %s\n", hashTable->table[index].wrongWord, hashTable->table[index].word);
		return 1;
	}
	return 0;
}
void insertToWrongHashTable(HASHTABLE* hashTable, unsigned long int key, char word[WORD_BUFFER], char wrongWord[WORD_BUFFER]){
	
	int i;
	int index;															//Hashten dönen index.
	if (hashTable->loadfactor < 1) {
		if (hashTable->loadfactor > 0.8) {
			printf("LOADFACTOR 0.8 den büyük. UYARI!!!!");				//LoadFactor kontrolü.
		}
		i = 0;
		do {
			index = (hash1(key) + (i * hash2(key))) % SIZE;				//Hash fonksiyonlarýndan dönen index.
			i++;
		} while (strcmp(hashTable->table[index].wrongWord, "-") && strcmp(hashTable->table[index].wrongWord, wrongWord) && i < SIZE);	//Boþ alan veya kelimenin aynýsýný bulunca çýkýcak.

		if (!strcmp(hashTable->table[index].word, "-")) {
			strcpy(hashTable->table[index].word, word);			//Boþ yer bulundu.
			strcpy(hashTable->table[index].wrongWord, wrongWord);
			(hashTable->indexCounter)++;						//Boþ yeri doldurduðumuz için artýrdýk.
			hashTable->loadfactor = (float)(hashTable->indexCounter) / SIZE;		//Loadfactor güncellenir.		
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
	char word[WORD_BUFFER];	//Cümleden aldýðýmýz kelimeleri tutar
	while (sentence[i] != NULL) {
		if (sentence[i] != ' ') {
			word[j] = sentence[i];	//Boþluk karakteri gelene kadar word'te biriktiririz.
			j++;
		}
		else {
			//Kelimeyi aldýk
			word[j] = NULL;
			j = 0;
			checkWord(hashTable, word, wrongHashTable);	//Kelime kontrolü
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
	int index;										//hash fonksiyonundan dönecek deðeri tutucak.
	long unsigned int key = horner(wrd); 					//Kelimenin sayý karþýlýðý alýndý.
	do {
		index = (hash1(key) + (i * hash2(key))) % SIZE;		//Hash fonksiyonlarýyla indexi bulundu.
		i++;
	} while (strcmp(hashTable->table[index].word, "-") && strcmp(hashTable->table[index].word, wrd) && i < SIZE);
}

HASHTABLE* readFile(FILE* fp) {
	int i, key, flag = 1;
	char buffer[WORD_BUFFER];										//Dosyadan okunan kelimeleri tutan deðiþken.
	HASHTABLE* hashTable = (HASHTABLE*)malloc(sizeof(HASHTABLE));	//Hash tablosu için alan açýlýr.
	prepareHashTable(hashTable);
	while (flag && fscanf(fp, "%s", buffer) != EOF) {
		key = horner(buffer);									//Kelime sayýya çevrilir.
		flag = insertToHash(hashTable, key, buffer);	//Hash tablosuna eklenir. Flag hash tablosunun dolup dolmadýðýný kontrol eder.
	}
	if (!flag) {
		printf("Tablo doldu. Taþan veriler : \n");
		while (fscanf(fp, "%s", buffer) != EOF) {		//Taþan veriler yazdýrýlýr.
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
	long unsigned int total = 0;								//Toplamý tutucak.
	long int multiple = 1;							//Her harfin çarpýlcaðý deðiþken.
	char letter;
	while (word[i] != NULL) {
		if (word[i] >= 65 && word[i] <= 90) {
			letter = word[i] - 'A';					//Büyük harfse 'A' çýkarýlýr.
		}
		else {
			letter = word[i] - 'a';					//Küçük harfse 'a' çýkarýlýr.
		}
		//En sonra oluþucak sayýyý küçültmüþ oldum.
		total = total + (letter * multiple);
		multiple *= HORNER_NUMBER;
		i++;
	}
	return total;
}


int insertToHash(HASHTABLE* hashTable, long unsigned int key, char wrd[WORD_BUFFER]) {
	int i;
	int index;															//Hashten dönen index.
	if (hashTable->loadfactor < 1) {
		if (hashTable->loadfactor > 0.8) {
			printf("LOADFACTOR 0.8 den büyük. UYARI!!!!");				//LoadFactor kontrolü.
		}
		i = 0;
		do {
			index = (hash1(key) + (i * hash2(key))) % SIZE;				//Hash fonksiyonlarýndan dönen index.
			i++;
		} while (strcmp(hashTable->table[index].word, "-") && strcmp(hashTable->table[index].word, wrd) && i < SIZE);	//Boþ alan veya kelimenin aynýsýný bulunca çýkýcak.

		if (!strcmp(hashTable->table[index].word, "-")) {
			strcpy(hashTable->table[index].word, wrd);			//Boþ yer bulundu.
			(hashTable->indexCounter)++;						//Boþ yeri doldurduðumuz için artýrdýk.
			hashTable->loadfactor = (float)(hashTable->indexCounter) / SIZE;		//Loadfactor güncellenir.		
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
	return (1 + (key % (SIZE - 1)));										//Ýkinci hash fonksiyonu.
}
