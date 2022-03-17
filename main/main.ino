#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
#include <SD.h>

// Se establece pins de sensores y LCD 
LiquidCrystal lcd(5, 6, 7, 8, 9, 10);   // Se establecen los pines 5,6,7,8,9,10 para LCD
OneWire ourWire1(3);				    // Se establece el pin 3  como bus OneWire
OneWire ourWire2(2);				    // Se establece el pin 2  como bus OneWire

// Función
File myFile;

// Variables Globales
int FileCounter = 0;    // Contador para archivos.
char FileName[14];      // Array para buscar archivos disponibles. Rango de trabajo 0-99.
bool isWorking = true;  // Guarda el status del funcionamiento de la tarjeta SD en la sesión actuál.
char timecounter[16];   //Array para display del tiempo. Rango máximo de display 99 minutos.
signed short minutes, secondes; // Variables de tiempo.

DallasTemperature sensors1(&ourWire1); // Se declara una variable u objeto para nuestro sensor1
DallasTemperature sensors2(&ourWire2); // Se declara una variable u objeto para nuestro sensor2

void setup()
{
	Serial.begin(9600); // Se inicia el puerto serial de monitoreo
	sensors1.begin(); // Se inicia el sensor 1
	sensors2.begin(); // Se inicia el sensor 2
	lcd.begin(20, 4); // Se inicia LCD 20,4

	// Se inicia lectura del SD
	// Se compueba la correcta iniciación de la tarjeta SD mediante el pin 4.
	Serial.print("Iniciando SD ...");
	if (!SD.begin(4)) // Si la tarjeta no inicializa, se guadra el status en isWorking como false.
	{
		isWorking = false;
		Serial.println("No se pudo inicializar");
		return;
	}
	else // Se guarda el status en isWorking como true.
	{
		Serial.println("inicializacion exitosa");
	}

    // Si la tarjeta se inicio, se inicia el protocolo de creación de archivo, evitando sobre-escribir archivos exsistenes.
	if (isWorking)
	{
		snprintf(FileName, 14, "datalog%d.csv", FileCounter); // concatena el nombre del archivo con un digito (0-99)
		while (SD.exists(FileName)) // verifica si el archivo existe, en ese caso, se aumento factor de uno el numero en el nombre.
		{
			Serial.println(FileName);
      FileCounter++; // Al existir un archivo con ese nombre, se aumenta el digito por un factor de 1.
			snprintf(FileName, 14, "datalog%d.csv", FileCounter); // concatena el nombre del archivo con nuevo digito.
		}

        // Al encontrar un nombre disponible:
		Serial.println("Se encotro un nombre desocupado");
		Serial.println(FileName);
		myFile = SD.open(FileName, FILE_WRITE);
    
		if (myFile)
		{
			Serial.println("Archivo nuevo, Escribiendo encabezado(fila 1)");
			myFile.println("Tiempo(ms),TempInt,TempExt");
			myFile.close();
			Serial.println("Archivo Creado");
		}
		else
		{
			Serial.println("Error creando el archivo datalog.csv");
		}
	}
}

void loop()
{
	sensors1.requestTemperatures();			   // Se envía el comando para leer la temperatura
	float temp1 = sensors1.getTempCByIndex(0); // Se obtiene la temperatura en ºC del sensor 1

	sensors2.requestTemperatures();			   // Se envía el comando para leer la temperatura
	float temp2 = sensors2.getTempCByIndex(0); // Se obtiene la temperatura en ºC del sensor 2

    // Se envia la infromación correspondiente a la LCD.
	lcd.setCursor(0, 0);
	lcd.print("Temp Int: ");
	lcd.print(temp1);
	lcd.print(" C");
	lcd.setCursor(0, 2);
	lcd.print("Temp Ext: ");
	lcd.print(temp2);
	lcd.print(" C");
	lcd.setCursor(0, 4);
  	sprintf(timecounter,"%0.2d mins %0.2d secs", minutes, secondes);
  	lcd.print(timecounter);

    // Se convierten los milisegundos de millis() a segundos.
	secondes = (millis()/1000);
    secondes = secondes-(60*minutes);

    // Se determina la cantidad de minutos transcurridos de acuerdo a los millis().
  	if (secondes >= 60)
  	{
    	minutes ++;
 	}
  
    // Si se inicializó correctamente la tarjeta SD en void setup(), se guardan los datos en cada iteración del void loop().
    // En caso de que no se inicializara correctamente, no se intenta guardar los datos.
	if(isWorking)
	{
		myFile = SD.open(FileName, FILE_WRITE); // abrimos  el archivo
		if (myFile)
		{
			Serial.println("Escribiendo SD: ");
			myFile.print((millis()/1000));
			myFile.print(",");
			myFile.print(temp1);
			myFile.print(",");
			myFile.print(temp2);
			myFile.print("\n");
			myFile.close(); // cerramos el archivo
		}
		else
		{
			// Si el archivo no abre, sale un error.
			Serial.println("Error al abrir el archivo");
		}
	}
	delay(500);
}