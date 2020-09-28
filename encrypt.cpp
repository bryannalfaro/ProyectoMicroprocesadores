/* 
 * Universidad del valle de Guatemala
 * Proyecto 2 - Programación de Microprocesadores
 * Integrantes:
 * Bryann Alfaro
 * Diego de Jesus Arredondo
 * Julio Roberto Herrera
 * Diego Alberto Alvarez
 * -----------------------------------------------
 * Modificación de:
 * Performs encryption using AES 128-bit
 * @author Cecelia Wisniewska
 */

#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <pthread.h>
#include "structures.h"

#define DATASIZE 239
using namespace std;
string str;
pthread_mutex_t mutex;
pthread_cond_t order;
int cont = 0;

/* Serves as the initial round during encryption
 * AddRoundKey is simply an XOR of a 128-bit block with the 128-bit key.
 */
void AddRoundKey(unsigned char * state, unsigned char * roundKey) {
	for (int i = 0; i < 16; i++) {
		state[i] ^= roundKey[i];
	}
}

/* Perform substitution to each of the 16 bytes
 * Uses S-box as lookup table 
 */
void SubBytes(unsigned char * state) {
	for (int i = 0; i < 16; i++) {
		state[i] = s[state[i]];
	}
}

// Shift left, adds diffusion
void ShiftRows(unsigned char * state) {
	unsigned char tmp[16];

	/* Column 1 */
	tmp[0] = state[0];
	tmp[1] = state[5];
	tmp[2] = state[10];
	tmp[3] = state[15];
	
	/* Column 2 */
	tmp[4] = state[4];
	tmp[5] = state[9];
	tmp[6] = state[14];
	tmp[7] = state[3];

	/* Column 3 */
	tmp[8] = state[8];
	tmp[9] = state[13];
	tmp[10] = state[2];
	tmp[11] = state[7];
	
	/* Column 4 */
	tmp[12] = state[12];
	tmp[13] = state[1];
	tmp[14] = state[6];
	tmp[15] = state[11];

	for (int i = 0; i < 16; i++) {
		state[i] = tmp[i];
	}
}

 /* MixColumns uses mul2, mul3 look-up tables
  * Source of diffusion
  */
void MixColumns(unsigned char * state) {
	unsigned char tmp[16];

	tmp[0] = (unsigned char) mul2[state[0]] ^ mul3[state[1]] ^ state[2] ^ state[3];
	tmp[1] = (unsigned char) state[0] ^ mul2[state[1]] ^ mul3[state[2]] ^ state[3];
	tmp[2] = (unsigned char) state[0] ^ state[1] ^ mul2[state[2]] ^ mul3[state[3]];
	tmp[3] = (unsigned char) mul3[state[0]] ^ state[1] ^ state[2] ^ mul2[state[3]];

	tmp[4] = (unsigned char)mul2[state[4]] ^ mul3[state[5]] ^ state[6] ^ state[7];
	tmp[5] = (unsigned char)state[4] ^ mul2[state[5]] ^ mul3[state[6]] ^ state[7];
	tmp[6] = (unsigned char)state[4] ^ state[5] ^ mul2[state[6]] ^ mul3[state[7]];
	tmp[7] = (unsigned char)mul3[state[4]] ^ state[5] ^ state[6] ^ mul2[state[7]];

	tmp[8] = (unsigned char)mul2[state[8]] ^ mul3[state[9]] ^ state[10] ^ state[11];
	tmp[9] = (unsigned char)state[8] ^ mul2[state[9]] ^ mul3[state[10]] ^ state[11];
	tmp[10] = (unsigned char)state[8] ^ state[9] ^ mul2[state[10]] ^ mul3[state[11]];
	tmp[11] = (unsigned char)mul3[state[8]] ^ state[9] ^ state[10] ^ mul2[state[11]];

	tmp[12] = (unsigned char)mul2[state[12]] ^ mul3[state[13]] ^ state[14] ^ state[15];
	tmp[13] = (unsigned char)state[12] ^ mul2[state[13]] ^ mul3[state[14]] ^ state[15];
	tmp[14] = (unsigned char)state[12] ^ state[13] ^ mul2[state[14]] ^ mul3[state[15]];
	tmp[15] = (unsigned char)mul3[state[12]] ^ state[13] ^ state[14] ^ mul2[state[15]];

	for (int i = 0; i < 16; i++) {
		state[i] = tmp[i];
	}
}

/* Each round operates on 128 bits at a time
 * The number of rounds is defined in AESEncrypt()
 */
void Round(unsigned char * state, unsigned char * key) {
	SubBytes(state);
	ShiftRows(state);
	MixColumns(state);
	AddRoundKey(state, key);
}

 // Same as Round() except it doesn't mix columns
void FinalRound(unsigned char * state, unsigned char * key) {
	SubBytes(state);
	ShiftRows(state);
	AddRoundKey(state, key);
}

/* The AES encryption function
 * Organizes the confusion and diffusion steps into one function
 */
void AESEncrypt(unsigned char * message, unsigned char * expandedKey, unsigned char * encryptedMessage) {
	unsigned char state[16]; // Stores the first 16 bytes of original message

	for (int i = 0; i < 16; i++) {
		state[i] = message[i];
	}

	int numberOfRounds = 9;

	AddRoundKey(state, expandedKey); // Initial round

	for (int i = 0; i < numberOfRounds; i++) {
		Round(state, expandedKey + (16 * (i+1)));
	}

	FinalRound(state, expandedKey + 160);

	// Copy encrypted state to buffer
	for (int i = 0; i < 16; i++) {
		encryptedMessage[i] = state[i];
	}
}

struct argData {
	string data;
	int i;
};

struct returnPreparedData {
	int paddedMessageLen;
	unsigned char * paddedMessage;
	unsigned char * encryptedMessage;
	unsigned char expandedKey[176];
	int i;
	returnPreparedData(void*&){};
	returnPreparedData(){};
};

void *prepareEncrypt(void *arg) {
	struct argData *ad;
	struct returnPreparedData *rpd = (returnPreparedData*)malloc(sizeof(*rpd));
	ad = (struct argData*)arg;

	// Pad message to 16 bytes

	rpd->paddedMessageLen = ad->data.length();
	rpd->i = ad->i;

	if ((rpd->paddedMessageLen % 16) != 0) {
		rpd->paddedMessageLen = (rpd->paddedMessageLen / 16 + 1) * 16;
	}

	rpd->paddedMessage = new unsigned char[rpd->paddedMessageLen];
	for (int i = 0; i < rpd->paddedMessageLen; i++) {
		if (i >= (int)(ad->data.length())) {
			rpd->paddedMessage[i] = 0;
		}
		else {
			rpd->paddedMessage[i] = ad->data[i];
		}
	}
	
	pthread_exit(rpd);
}



void *encrypt(void *arg) {
	pthread_mutex_lock(&mutex);
	struct returnPreparedData *rpd;
	rpd = (struct returnPreparedData*)arg;
	
	rpd->encryptedMessage = new unsigned char[rpd->paddedMessageLen];

	while (cont < rpd->i)
	    pthread_cond_wait(&order, &mutex);
	
	for (int i = 0; i < rpd->paddedMessageLen; i += 16) {
		AESEncrypt(rpd->paddedMessage+i, rpd->expandedKey, rpd->encryptedMessage+i);
	}
	
	cont++;
	pthread_cond_broadcast(&order);
	pthread_mutex_unlock(&mutex);
	pthread_exit(rpd);
}

int executeE() {

	// Abriendo archivo
	string data[DATASIZE]; // Contiene todos los datos a encriptar
	ifstream infile;
	infile.open("Datos.txt", ios::in | ios::binary);
	if (infile.is_open())
	{
		int line = 0;
		string dataUnique;
		while (getline(infile, dataUnique)) {
			data[line] = dataUnique;
			line++;
		}
		infile.close();
	}
	else cout << "Unable to open file";
	
	// Abriendo archivo de llave
	infile.open("keyfile", ios::in | ios::binary);

	if (infile.is_open())
	{
		getline(infile, str); // The first line of file should be the key
		infile.close();
	}
	else cout << "Unable to open file";
	
	istringstream hex_chars_stream(str);
	unsigned char key[16];
	int i = 0;
	unsigned int c;
	while (hex_chars_stream >> hex >> c)
	{
		key[i] = c;
		i++;
	}

	unsigned char expandedKey[176];

	KeyExpansion(key, expandedKey);

	// Empiezan los hilos por cada dato
	
	// convertir a hexadecimal
	pthread_t pid1[DATASIZE];
	struct argData ad[DATASIZE];
	for (int i = 0; i < DATASIZE; i++) {
		ad[i].data = data[i];
		ad[i].i = i;
		pthread_create(&pid1[i], NULL, prepareEncrypt, (void*)&ad[i]);
	}
	
	pthread_t pid2[DATASIZE];
	void *vrpd;
	struct returnPreparedData rpd[DATASIZE];
	pthread_cond_init(&order, NULL);
	for (int i = 0; i < DATASIZE; i++) {
		pthread_join(pid1[i], &vrpd); // Espera que se haya hecho la conversion hex
		rpd[i] = *(returnPreparedData*)vrpd;
		for (int j = 0; j < 176; j++) { // TODO: investigar la funcion para copiar array
			rpd[i].expandedKey[j] = expandedKey[j];
		}
		pthread_create(&pid2[i], NULL, encrypt, (void*)&rpd[i]);
	}
	
	ofstream outfile;
	outfile.open("message.aes", ios::out | ios::binary);
	for (int i = 0; i < DATASIZE; i++) {
		pthread_join(pid2[i], &vrpd);
		rpd[i] = *(returnPreparedData*)vrpd;
		unsigned char toFile[rpd[i].paddedMessageLen * 2];
		cout << "\nEncrypted message in hex:" << endl;
		int contChar = 0;
		for (int j = 0; j < rpd[i].paddedMessageLen; j++) {
			char par[3];
			sprintf(par, "%02X", (int)((rpd[i].encryptedMessage)[j]));
			toFile[contChar] = par[0];
			toFile[contChar+1] = par[1];
			contChar += 2;
			cout << par;
			cout << " ";
		}
		cout << endl;
		// Escribiendo dato encriptado en message.aes
		if (outfile.is_open())
		{
			outfile << toFile;
			outfile << "\n";
		}
		else cout << "Unable to open file";
	}
	outfile.close();
/*
	// Free memory
	delete[] paddedMessage;
	delete[] encryptedMessage;
*/
	return 0;
}
