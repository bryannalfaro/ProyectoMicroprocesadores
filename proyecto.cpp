/*
Universidad del valle de Guatemala
Proyecto 2 - Programación de Microprocesadores
Integrantes:
Bryann Alfaro
Diego de Jesus Arredondo
Julio Roberto Herrera
Diego Alberto Alvarez
*/

#include <iostream>
#include <fstream>

using namespace std;

/*
Metodo para elaborar archivo de texto
*/
void makeText(string opcion){
	
	ofstream documento;
	string delimitar= opcion+".txt";
	
	documento.open(delimitar.c_str(),ios::out);
}

/*
Metodo para insertar datos
*/



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


int main(){
	string op;
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
		
		//lectura del archivo
		openText(op);
	}else{
		return 0;
	}	
	
	
	
	
}
