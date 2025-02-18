/* 
 * Universidad del valle de Guatemala
 * Proyecto 2 - Programacion de Microprocesadores
 * Integrantes:
 * Bryann Alfaro
 * Diego de Jesus Arredondo
 * Julio Roberto Herrera
 * Diego Alberto Alvarez
 * -----------------------------------------------
 * Encriptación de datos usando hilos
 * Se encriptan la cantidad de datos definida por
 * DATASIZE, debe ser la cantidad de lineas
 * que hay en Datos.txt
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
#include "structures.h"

#define DATASIZE 239
using namespace std;

//Creacion de variables
string str;
pthread_mutex_t mutex;
pthread_cond_t order;
int cont = 0;

/* Ronda inicial en la encripción
 * AddRoundKey es una operación XOR al bloque de 128-bit con
 * el bloque de 128-bit de la llave
 */
void AddRoundKey(unsigned char * state, unsigned char * roundKey) {
	for (int i = 0; i < 16; i++) {
		state[i] ^= roundKey[i];
	}
}

/* Sustitución de los 16 bits por s en structures.h
 * s en structures.h es la S-box Table de AES
 */
void SubBytes(unsigned char * state) {
	for (int i = 0; i < 16; i++) {
		state[i] = s[state[i]];
	}
}

// Shift left
void ShiftRows(unsigned char * state) {
	unsigned char tmp[16];

	/* Columna 1 */
	tmp[0] = state[0];
	tmp[1] = state[5];
	tmp[2] = state[10];
	tmp[3] = state[15];
	
	/* Columna 2*/
	tmp[4] = state[4];
	tmp[5] = state[9];
	tmp[6] = state[14];
	tmp[7] = state[3];

	/* Columna 3 */
	tmp[8] = state[8];
	tmp[9] = state[13];
	tmp[10] = state[2];
	tmp[11] = state[7];
	
	/* Columna 4 */
	tmp[12] = state[12];
	tmp[13] = state[1];
	tmp[14] = state[6];
	tmp[15] = state[11];

	for (int i = 0; i < 16; i++) {
		state[i] = tmp[i];
	}
}

 /* MixColumns usa mul2, mul3 de structures.h
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

/* Rondas que operan sobre la matriz de 128 bits
 * La cantidad de rondas está definida en AESEncrypt() 
 */
void Round(unsigned char * state, unsigned char * key) {
	SubBytes(state);
	ShiftRows(state);
	MixColumns(state);
	AddRoundKey(state, key);
}

// La FinalRound() es lo mismo que Round() pero sin MixColumns
void FinalRound(unsigned char * state, unsigned char * key) {
	SubBytes(state);
	ShiftRows(state);
	AddRoundKey(state, key);
}

/* Define el orden para cumplir con el algoritmo AES
 * Organiza los pasos de Confusión y Difusión del algoritmo
 */
void AESEncrypt(unsigned char * message, unsigned char * expandedKey, unsigned char * encryptedMessage) {
	unsigned char state[16]; // Guarda los primeros 16 bits del mensaje original

	for (int i = 0; i < 16; i++) {
		state[i] = message[i]; // Copia el mensaje
	}

	int numberOfRounds = 9; // En aes 128 se aplican 9 vueltas

	AddRoundKey(state, expandedKey); // Ronda para conjugar con la llave

	for (int i = 0; i < numberOfRounds; i++) {
		Round(state, expandedKey + (16 * (i+1))); // Las 9 rondas
	}

	FinalRound(state, expandedKey + 160); // Ronda final (sin MixColumns)

	// Copia el mensaje encriptado en el buffer
	for (int i = 0; i < 16; i++) {
		encryptedMessage[i] = state[i];
	}
}

// Estructura para pasar datos a *prepareEncrypt
struct argData {
	string data;
	int i;
};

// Estructura para pasar datos a Encrypt
struct returnPreparedData {
	int paddedMessageLen;
	unsigned char * paddedMessage;
	unsigned char * encryptedMessage;
	unsigned char expandedKey[176];
	int i;
	// Constructores
	returnPreparedData(void*&){};
	returnPreparedData(){};
};

// Convierte a hexadecimal el mensaje original
void *prepareEncrypt(void *arg) {
	struct argData *ad;
	struct returnPreparedData *rpd = (returnPreparedData*)malloc(sizeof(*rpd));
	ad = (struct argData*)arg;

	// Acomoda el mensaje a 16 bits según su longitud

	rpd->paddedMessageLen = ad->data.length();
	rpd->i = ad->i;

	if ((rpd->paddedMessageLen % 16) != 0) {
		rpd->paddedMessageLen = (rpd->paddedMessageLen / 16 + 1) * 16;
	}

	rpd->paddedMessage = new unsigned char[rpd->paddedMessageLen];
	for (int i = 0; i < rpd->paddedMessageLen; i++) {
		if (i >= (int)(ad->data.length())) {
			// Completa para los que no cumplen para su división de 16
			rpd->paddedMessage[i] = 0;
		}
		else {
			rpd->paddedMessage[i] = ad->data[i];
		}
	}
	
	pthread_exit(rpd); //Salida del hilo
}

// Llama a la función AESEncrypt y define el orden de la encripción de los
// datos mediante un mutex y variable de condición
void *encrypt(void *arg) {
	pthread_mutex_lock(&mutex); // Se bloquea la variable
	struct returnPreparedData *rpd;
	rpd = (struct returnPreparedData*)arg;
	
	rpd->encryptedMessage = new unsigned char[rpd->paddedMessageLen];

	while (cont < rpd->i) // Se salen los que van cumpliendo con el orden
	    pthread_cond_wait(&order, &mutex); //En esta parte se aplica sincronizacion
	
	for (int i = 0; i < rpd->paddedMessageLen; i += 16) {
		AESEncrypt(rpd->paddedMessage+i, rpd->expandedKey, rpd->encryptedMessage+i);
	}
	
	cont++;
	pthread_cond_broadcast(&order);
	pthread_mutex_unlock(&mutex);//Se desbloquea la variable
	pthread_exit(rpd);//Se sale del hilo
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
		getline(infile, str); // La primera linea de este archivo es la llave
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
	
	// Aplicar encripción
	pthread_t pid2[DATASIZE];
	void *vrpd;
	struct returnPreparedData rpd[DATASIZE];
	pthread_cond_init(&order, NULL);
	for (int i = 0; i < DATASIZE; i++) {
		pthread_join(pid1[i], &vrpd); // Espera que se haya hecho la conversion hex
		rpd[i] = *(returnPreparedData*)vrpd;
		for (int j = 0; j < 176; j++) {
			rpd[i].expandedKey[j] = expandedKey[j]; // Copia el bloque de llave
		}
		pthread_create(&pid2[i], NULL, encrypt, (void*)&rpd[i]);
	}
	
	// Imprime el mensaje en pantalla y escribe en archivo
	ofstream outfile;
	outfile.open("message.aes", ios::out | ios::binary);
	for (int i = 0; i < DATASIZE; i++) {
		pthread_join(pid2[i], &vrpd);
		rpd[i] = *(returnPreparedData*)vrpd;
		unsigned char toFile[rpd[i].paddedMessageLen * 2];
		cout << "\nEncrypted message in hex:" << endl;
		int contChar = 0;
		// Imprime en pantalla
		for (int j = 0; j < rpd[i].paddedMessageLen; j++) {
			char par[3];
			sprintf(par, "%02X", (int)((rpd[i].encryptedMessage)[j]));
			toFile[contChar] = par[0];
			toFile[contChar+1] = par[1];
			contChar += 2;
			cout << par;
			cout << " ";
		}
		cout << "\n";
		// Escribiendo dato encriptado en message.aes
		if (outfile.is_open())
		{
			outfile << toFile;
			outfile << "\n";
		}
		else cout << "Unable to open file";
	}
	outfile.close();
	return 0;
}
