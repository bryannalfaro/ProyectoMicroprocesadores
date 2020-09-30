/* decrypt.cpp
 * Performs decryption using AES 128-bit
 * @author Cecelia Wisniewska
 */
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <pthread.h>
#include "structures.h"

using namespace std;

/* Used in Round() and serves as the final round during decryption
 * SubRoundKey is simply an XOR of a 128-bit block with the 128-bit key.
 * So basically does the same as AddRoundKey in the encryption
 */
void SubRoundKey(unsigned char * state, unsigned char * roundKey) {
	for (int i = 0; i < 16; i++) {
		state[i] ^= roundKey[i];
	}
}

/* InverseMixColumns uses mul9, mul11, mul13, mul14 look-up tables
 * Unmixes the columns by reversing the effect of MixColumns in encryption
 */
void InverseMixColumns(unsigned char * state) {
	unsigned char tmp[16];

	tmp[0] = (unsigned char)mul14[state[0]] ^ mul11[state[1]] ^ mul13[state[2]] ^ mul9[state[3]];
	tmp[1] = (unsigned char)mul9[state[0]] ^ mul14[state[1]] ^ mul11[state[2]] ^ mul13[state[3]];
	tmp[2] = (unsigned char)mul13[state[0]] ^ mul9[state[1]] ^ mul14[state[2]] ^ mul11[state[3]];
	tmp[3] = (unsigned char)mul11[state[0]] ^ mul13[state[1]] ^ mul9[state[2]] ^ mul14[state[3]];

	tmp[4] = (unsigned char)mul14[state[4]] ^ mul11[state[5]] ^ mul13[state[6]] ^ mul9[state[7]];
	tmp[5] = (unsigned char)mul9[state[4]] ^ mul14[state[5]] ^ mul11[state[6]] ^ mul13[state[7]];
	tmp[6] = (unsigned char)mul13[state[4]] ^ mul9[state[5]] ^ mul14[state[6]] ^ mul11[state[7]];
	tmp[7] = (unsigned char)mul11[state[4]] ^ mul13[state[5]] ^ mul9[state[6]] ^ mul14[state[7]];

	tmp[8] = (unsigned char)mul14[state[8]] ^ mul11[state[9]] ^ mul13[state[10]] ^ mul9[state[11]];
	tmp[9] = (unsigned char)mul9[state[8]] ^ mul14[state[9]] ^ mul11[state[10]] ^ mul13[state[11]];
	tmp[10] = (unsigned char)mul13[state[8]] ^ mul9[state[9]] ^ mul14[state[10]] ^ mul11[state[11]];
	tmp[11] = (unsigned char)mul11[state[8]] ^ mul13[state[9]] ^ mul9[state[10]] ^ mul14[state[11]];

	tmp[12] = (unsigned char)mul14[state[12]] ^ mul11[state[13]] ^ mul13[state[14]] ^ mul9[state[15]];
	tmp[13] = (unsigned char)mul9[state[12]] ^ mul14[state[13]] ^ mul11[state[14]] ^ mul13[state[15]];
	tmp[14] = (unsigned char)mul13[state[12]] ^ mul9[state[13]] ^ mul14[state[14]] ^ mul11[state[15]];
	tmp[15] = (unsigned char)mul11[state[12]] ^ mul13[state[13]] ^ mul9[state[14]] ^ mul14[state[15]];

	for (int i = 0; i < 16; i++) {
		state[i] = tmp[i];
	}
}

// Shifts rows right (rather than left) for decryption
void ShiftRows(unsigned char * state) {
	unsigned char tmp[16];

	/* Column 1 */
	tmp[0] = state[0];
	tmp[1] = state[13];
	tmp[2] = state[10];
	tmp[3] = state[7];

	/* Column 2 */
	tmp[4] = state[4];
	tmp[5] = state[1];
	tmp[6] = state[14];
	tmp[7] = state[11];

	/* Column 3 */
	tmp[8] = state[8];
	tmp[9] = state[5];
	tmp[10] = state[2];
	tmp[11] = state[15];

	/* Column 4 */
	tmp[12] = state[12];
	tmp[13] = state[9];
	tmp[14] = state[6];
	tmp[15] = state[3];

	for (int i = 0; i < 16; i++) {
		state[i] = tmp[i];
	}
}

/* Perform substitution to each of the 16 bytes
 * Uses inverse S-box as lookup table
 */
void SubBytes(unsigned char * state) {
	for (int i = 0; i < 16; i++) { // Perform substitution to each of the 16 bytes
		state[i] = inv_s[state[i]];
	}
}

/* Each round operates on 128 bits at a time
 * The number of rounds is defined in AESDecrypt()
 * Not surprisingly, the steps are the encryption steps but reversed
 */
void Round(unsigned char * state, unsigned char * key) {
	SubRoundKey(state, key);
	InverseMixColumns(state);
	ShiftRows(state);
	SubBytes(state);
}

// Same as Round() but no InverseMixColumns
void InitialRound(unsigned char * state, unsigned char * key) {
	SubRoundKey(state, key);
	ShiftRows(state);
	SubBytes(state);
}

/* The AES decryption function
 * Organizes all the decryption steps into one function
 */
void AESDecrypt(unsigned char * encryptedMessage, unsigned char * expandedKey, unsigned char * decryptedMessage)
{
	unsigned char state[16]; // Stores the first 16 bytes of encrypted message

	for (int i = 0; i < 16; i++) {
		state[i] = encryptedMessage[i];
	}

	InitialRound(state, expandedKey+160);

	for (int i = 8; i >= 0; i--) {
		Round(state, expandedKey + (16 * (i + 1)));
	}

	SubRoundKey(state, expandedKey); // Final round

	// Copy decrypted state to buffer
	for (int i = 0; i < 16; i++) {
		decryptedMessage[i] = state[i];
	}
}

struct argData {
	string data;
	int i;
};

struct returnPreparedData {
	unsigned char * msg;
	unsigned char * encryptedMessage;
	unsigned char * decryptedMessage;
	unsigned char expandedKey[176];
	int i;
	returnPreparedData(void*&){};
	returnPreparedData(){};
};

void *prepareDecrypt(void *arg) {
	struct argData *ad;
	struct returnPreparedData *rpd = (returnPreparedData*)malloc(sizeof(*rpd));
	ad = (struct argData*)arg;
	
	// Convert to hex
	rpd->msg = new unsigned char[ad->data.size()+1];

	strcpy((char*)rpd->msg, ad->data.c_str());

	rpd->encryptedMessage = new unsigned char[16];
	int contHex = 0;
	for (int i = 0; i < 16; i++) {
		char hex[2];
		hex[0] = rpd->msg[contHex];
		hex[1] = rpd->msg[contHex + 1];
		contHex += 2;
		int d;
		sscanf(hex,"%X", &d);
		rpd->encryptedMessage[i] = (unsigned char)d;
	}
	
	pthread_exit(rpd);
}

void *decrypt(void *arg) {
	struct returnPreparedData *rpd;
	rpd = (struct returnPreparedData*)arg;
	
	int messageLen = 16;

	rpd->decryptedMessage = new unsigned char[messageLen];

	for (int i = 0; i < messageLen; i += 16) {
		AESDecrypt(rpd->encryptedMessage + i, rpd->expandedKey, rpd->decryptedMessage + i);
	}
	
	pthread_exit(rpd);
}

int executeD() {

	cout << "=============================" << endl;
	cout << " 128-bit AES Decryption Tool " << endl;
	cout << "=============================" << endl;

	// Read in the message from message.aes
	int DATASIZE = 0;
	ifstream infile;
	infile.open("message.aes", ios::in | ios::binary);

	if (infile.is_open())
	{
		string dataUnique;
		while (getline(infile, dataUnique)) {
			DATASIZE++;
		}
		infile.close();
	}
	
	printf("la cantidad de lineas es %d", DATASIZE);
	
	string msgstr[DATASIZE];
	infile.open("message.aes", ios::in | ios::binary);

	if (infile.is_open())
	{
		int line = 0;
		string dataUnique;
		while (getline(infile, dataUnique)) {
			if (line <= DATASIZE) {
				msgstr[line] = dataUnique;
				msgstr[line].pop_back();
				msgstr[line].pop_back();
			}
			line++;
		}
		infile.close();
	}

	else cout << "Unable to open file";
	
	// Read in the key
	string keystr;
	ifstream keyfile;
	keyfile.open("keyfile", ios::in | ios::binary);

	if (keyfile.is_open())
	{
		getline(keyfile, keystr); // The first line of file should be the key
		cout << "Read in the 128-bit key from keyfile" << endl;
		keyfile.close();
	}

	else cout << "Unable to open file";

	istringstream hex_chars_stream(keystr);
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
	
	pthread_t pid1[DATASIZE];
	struct argData ad[DATASIZE];
	for (int i = 0; i < DATASIZE; i++) {
		ad[i].data = msgstr[i];
		ad[i].i = i;
		pthread_create(&pid1[i], NULL, prepareDecrypt, (void*)&ad[i]);
	}

	// Free memory
	//delete[] msg;
	
	pthread_t pid2[DATASIZE];
	void *vrpd;
	struct returnPreparedData rpd[DATASIZE];
	for (int i = 0; i < DATASIZE; i++) {
		pthread_join(pid1[i], &vrpd); // Espera que se haya hecho la conversion hex
		rpd[i] = *(returnPreparedData*)vrpd;
		for (int j = 0; j < 176; j++) { // TODO: investigar la funcion para copiar array
			rpd[i].expandedKey[j] = expandedKey[j];
		}
		pthread_create(&pid2[i], NULL, decrypt, (void*)&rpd[i]);
	}


	ofstream outfile;
	outfile.open("DatosDesencriptados.txt", ios::out | ios::binary);
	for (int i = 0; i < DATASIZE; i++) {
		pthread_join(pid2[i], &vrpd);
		rpd[i] = *(returnPreparedData*)vrpd;
		cout << "Decrypted message in hex:" << endl;
		for (int j = 0; j < 16; j++) {
			cout << hex << (int)rpd[i].decryptedMessage[j];
			cout << " ";
		}
		cout << endl;
		cout << "Decrypted message: ";
		unsigned char toFile[256];
		for (int j = 0; j < 16; j++) {
			cout << rpd[i].decryptedMessage[j];
			toFile[j] = rpd[i].decryptedMessage[j];
		}
		cout << endl;
		if (outfile.is_open())
		{
			outfile << toFile;
			outfile << "\n";
		}
		else cout << "Unable to open file";
	}

	

	return 0;
}
