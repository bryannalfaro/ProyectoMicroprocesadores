/* 
 * Universidad del valle de Guatemala
 * Proyecto 2 - Programacion de Microprocesadores
 * Integrantes:
 * Bryann Alfaro
 * Diego de Jesus Arredondo
 * Julio Roberto Herrera
 * Diego Alberto Alvarez
 * -----------------------------------------------
 * Desencriptación de datos usando hilos
 * La cantidad de datos a desencriptar está definida
 * por la cantidad de líneas en Message.aes
 * -----------------------------------------------
 * Modificacion del codigo de:
 * Performs encryption using AES 128-bit
 * author: Cecelia Wisniewska
 */
 
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <pthread.h>
#include "structuresD.h"

using namespace std;

/* En la desencripción sirve para cada Round() y en la ronda final
 * SubRoundKey es una operación XOR al bloque de 128-bits del mensaje
 * con el bloque de 128-bits de la llave.
 */
void SubRoundKey(unsigned char * state, unsigned char * roundKey) {
	for (int i = 0; i < 16; i++) {
		state[i] ^= roundKey[i];
	}
}

/* Usa mul9D, mul11D, mul13D, mul14D de structures.h
 * Las operaciones invierten las realizadas por MixColumns en la encriptación
 */
void InverseMixColumns(unsigned char * state) {
	unsigned char tmp[16];

	tmp[0] = (unsigned char)mul14D[state[0]] ^ mul11D[state[1]] ^ mul13D[state[2]] ^ mul9D[state[3]];
	tmp[1] = (unsigned char)mul9D[state[0]] ^ mul14D[state[1]] ^ mul11D[state[2]] ^ mul13D[state[3]];
	tmp[2] = (unsigned char)mul13D[state[0]] ^ mul9D[state[1]] ^ mul14D[state[2]] ^ mul11D[state[3]];
	tmp[3] = (unsigned char)mul11D[state[0]] ^ mul13D[state[1]] ^ mul9D[state[2]] ^ mul14D[state[3]];

	tmp[4] = (unsigned char)mul14D[state[4]] ^ mul11D[state[5]] ^ mul13D[state[6]] ^ mul9D[state[7]];
	tmp[5] = (unsigned char)mul9D[state[4]] ^ mul14D[state[5]] ^ mul11D[state[6]] ^ mul13D[state[7]];
	tmp[6] = (unsigned char)mul13D[state[4]] ^ mul9D[state[5]] ^ mul14D[state[6]] ^ mul11D[state[7]];
	tmp[7] = (unsigned char)mul11D[state[4]] ^ mul13D[state[5]] ^ mul9D[state[6]] ^ mul14D[state[7]];

	tmp[8] = (unsigned char)mul14D[state[8]] ^ mul11D[state[9]] ^ mul13D[state[10]] ^ mul9D[state[11]];
	tmp[9] = (unsigned char)mul9D[state[8]] ^ mul14D[state[9]] ^ mul11D[state[10]] ^ mul13D[state[11]];
	tmp[10] = (unsigned char)mul13D[state[8]] ^ mul9D[state[9]] ^ mul14D[state[10]] ^ mul11D[state[11]];
	tmp[11] = (unsigned char)mul11D[state[8]] ^ mul13D[state[9]] ^ mul9D[state[10]] ^ mul14D[state[11]];

	tmp[12] = (unsigned char)mul14D[state[12]] ^ mul11D[state[13]] ^ mul13D[state[14]] ^ mul9D[state[15]];
	tmp[13] = (unsigned char)mul9D[state[12]] ^ mul14D[state[13]] ^ mul11D[state[14]] ^ mul13D[state[15]];
	tmp[14] = (unsigned char)mul13D[state[12]] ^ mul9D[state[13]] ^ mul14D[state[14]] ^ mul11D[state[15]];
	tmp[15] = (unsigned char)mul11D[state[12]] ^ mul13D[state[13]] ^ mul9D[state[14]] ^ mul14D[state[15]];

	for (int i = 0; i < 16; i++) {
		state[i] = tmp[i];
	}
}

// Shifts rows right (para inventir Shift left)
void ShiftRowsD(unsigned char * state) {
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

/* Realiza la sustitución para el bloque de 16 bytes con la tabla
 * S-box de AES definida en structures.h
 */
void SubBytesD(unsigned char * state) {
	for (int i = 0; i < 16; i++) { // Sustitución para cada uno de los 16 bloques
		state[i] = inv_sD[state[i]];
	}
}

/* La ronda que se operará a la matriz de 128-bits
 * El número de rondas está definido por los la encriptación de 128
 * Los pasos son los invertidos que los de la encriptación
 */
void RoundD(unsigned char * state, unsigned char * key) {
	SubRoundKey(state, key);
	InverseMixColumns(state);
	ShiftRowsD(state);
	SubBytesD(state);
}

// La ronda inicial es igual a Round() pero sin InverseMixColumns
void InitialRound(unsigned char * state, unsigned char * key) {
	SubRoundKey(state, key);
	ShiftRowsD(state);
	SubBytesD(state);
}

/* La función que define el orden del algoritmo AES
 * Define los pasos para la desencripción
 */
void AESDecrypt(unsigned char * encryptedMessage, unsigned char * expandedKey, unsigned char * decryptedMessage)
{
	unsigned char state[16]; // Guarda los 16 bits del mensaje encriptado

	for (int i = 0; i < 16; i++) {
		state[i] = encryptedMessage[i];
	}

	InitialRound(state, expandedKey+160);

	for (int i = 8; i >= 0; i--) {
		RoundD(state, expandedKey + (16 * (i + 1)));
	}

	SubRoundKey(state, expandedKey); // Ronda final

	// Copia el mensaje desencriptado en el buffer
	for (int i = 0; i < 16; i++) {
		decryptedMessage[i] = state[i];
	}
}

// Estructura para pasar datos a prepareDecrypt
struct argData {
	string data;
	int i;
};

// Estructura para pasar datos a Decrypt
struct returnPreparedData {
	unsigned char * msg;
	unsigned char * encryptedMessage;
	unsigned char * decryptedMessage;
	unsigned char expandedKey[176];
	int i;
	// Constructores
	returnPreparedData(void*&){};
	returnPreparedData(){};
};

// Prepara el mensaje para 16 bits
void *prepareDecrypt(void *arg) {
	struct argData *ad;
	struct returnPreparedData *rpd = (returnPreparedData*)malloc(sizeof(*rpd));
	ad = (struct argData*)arg;
	
	// Convierte a hexadecimal
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

// Llama a la función AESDecrypt
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

	// Lee los datos de message.aes
	// Define la cantidad de lineas = datos a desencriptar
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
	
	// Obtiene los datos encriptados	
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
	
	// Lee la llave
	string keystr;
	ifstream keyfile;
	keyfile.open("keyfile", ios::in | ios::binary);

	if (keyfile.is_open())
	{
		getline(keyfile, keystr); // La primera linea en este archivo debe ser la llave
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

	KeyExpansionD(key, expandedKey);
	
	// Hilos para convertir a hex
	pthread_t pid1[DATASIZE];
	struct argData ad[DATASIZE];
	for (int i = 0; i < DATASIZE; i++) {
		ad[i].data = msgstr[i];
		ad[i].i = i;
		pthread_create(&pid1[i], NULL, prepareDecrypt, (void*)&ad[i]);
	}
	
	// Aplica la desencriptción
	pthread_t pid2[DATASIZE];
	void *vrpd;
	struct returnPreparedData rpd[DATASIZE];
	for (int i = 0; i < DATASIZE; i++) {
		pthread_join(pid1[i], &vrpd); // Espera que se haya hecho la conversion hex
		rpd[i] = *(returnPreparedData*)vrpd;
		for (int j = 0; j < 176; j++) {
			rpd[i].expandedKey[j] = expandedKey[j];
		}
		pthread_create(&pid2[i], NULL, decrypt, (void*)&rpd[i]);
	}

	// Imprime los mensajes desencriptados y escribe en archivo
	ofstream outfile;
	outfile.open("DatosDesencriptados.txt", ios::out | ios::binary);
	for (int i = 0; i < DATASIZE; i++) {
		pthread_join(pid2[i], &vrpd);
		rpd[i] = *(returnPreparedData*)vrpd;
		cout << "Decrypted message: ";
		unsigned char toFile[256];
		// Imprime en pantalla
		for (int j = 0; j < 16; j++) {
			cout << rpd[i].decryptedMessage[j];
			toFile[j] = rpd[i].decryptedMessage[j];
		}
		cout << endl;
		// Escribe en archivo
		if (outfile.is_open())
		{
			outfile << toFile;
			outfile << "\n";
		}
		else cout << "Unable to open file";
	}

	

	return 0;
}
