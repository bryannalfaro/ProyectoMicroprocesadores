'''
Universidad del valle de Guatemala
Programacion de Microprocesadores
Integrantes:
Bryann Alfaro
Diego de Jesus
Julio Herrera
Diego Alvarez

Programa para comprobar la fidelidad de encriptacion
Referencias:
https://netroxtech.com/2019/04/28/learn-to-hash-files-in-python/
'''
import hashlib


texto  = open("Datos.txt", "r")
texto2 = open("DatosDesencriptados.txt", "r")
string = texto.read()
string2= texto2.read()

#Se realiza la creacion del hash
final = hashlib.sha256(str(string).encode('utf-8')).hexdigest()
final2 = hashlib.sha256(str(string2).encode('utf-8')).hexdigest()
print("Verificacion de la fidelidad de los datos> ")
print()
print("Datos iniciales SHA256: ")
print(final)
print("Datos desencriptados SHA256: ")
print(final2)
