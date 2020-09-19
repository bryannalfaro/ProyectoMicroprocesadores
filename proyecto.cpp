/*
 * Universidad del valle de Guatemala
 * Proyecto 2 - Programación de Microprocesadores
 * Integrantes:
 * Bryann Alfaro
 * Diego de Jesus Arredondo
 * Julio Roberto Herrera
 * Diego Alberto Alvarez
*/

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <pigpio.h>
#include <pthread.h>

#define SONAR_TRIGGER 23
#define SONAR_ECHO    24

using namespace std;

string op = "";
ofstream documento;

/*
Metodo para elaborar archivo de texto
*/
void makeText(string opcion){
	
	string delimitar= opcion+".txt";
	documento.open(delimitar.c_str(),ios::out);

}

/*
Metodo para insertar datos
*/
void insertText(string opcion, int ingreso){

	string delimitar= opcion+".txt";
	
	documento<<ingreso<<endl;
	
}


/*
Metodo para lectura de datos
*/

void sonarTrigger(void);

void sonarEcho(int gpio, int level, uint32_t tick);

void sonarTrigger(void)
{
   /* trigger a sonar reading */
   
   gpioWrite(SONAR_TRIGGER, PI_ON);

   gpioDelay(10); /* 10us trigger pulse */

   gpioWrite(SONAR_TRIGGER, PI_OFF);
}

void sonarEcho(int gpio, int level, uint32_t tick)
{
   static uint32_t startTick, firstTick=0;

   int diffTick;

   if (!firstTick) firstTick = tick;

   if (level == PI_ON)
   {
      startTick = tick;
   }
   else if (level == PI_OFF)
   {
      diffTick = (tick - startTick)/100;
      insertText(op, diffTick);
      printf("Distancia: ");
      printf("%u", diffTick);
      printf("cm\n");
   }
}

/*
Metodo para encriptar
*/



/*
Metodo para desencriptar
*/




/*
Metodo para abrir archivo
*/
void openText(string opcion){
	ifstream documento;
	string text;
	
	string delimitar= opcion+".txt";
	
	documento.open(delimitar.c_str(),ios::in);
	
	while(!documento.eof()){
		getline(documento,text);
		cout<<text<<endl;
	}
}


int main(int argc, char *argv[]){
	string menu;
	
	printf("--------------------------------------\n");
	cout<<"|  _                   _              |"<<endl;
	cout<<"| /  ._   ._ _|_  _.  /   _   _|  _   |"<<endl;
	cout<<"| |_ | |/ |_) |_ (_|  |_ (_) (_| (/_  |"<<endl;
	cout<<"|      /  |                           |"<<endl;
	printf("--------------------------------------\n");

	printf("Programa de encriptacion automatizada\n");
	printf("Escoge una de las siguientes opciones de trabajo: \n");
	printf("1. Empezar trabajo de encriptacion\n");
	printf("2. Salir\n");
	cin>>menu;
	
	if(menu=="1"){
		printf("Ingresa el nombre del archivo para guardar tus datos: ");
		cin>>op;
		//creacion del archivo
		makeText(op);
		
		if (gpioInitialise()<0) return 1;
		
		gpioSetMode(SONAR_TRIGGER, PI_OUTPUT);
		gpioWrite(SONAR_TRIGGER, PI_OFF);
		gpioSetMode(SONAR_ECHO,    PI_INPUT);

		/* update sonar 20 times a second, timer #0 */

		gpioSetTimerFunc(0, 500, sonarTrigger); /* every 50ms */

		/* monitor sonar echos */

		gpioSetAlertFunc(SONAR_ECHO, sonarEcho);

		while (1){
		sleep(1);
		}

		gpioTerminate();

		return 0;
		
	}else{
		return 0;
	}		
}
