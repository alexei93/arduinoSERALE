
/*INFO :
 08:C5:E1:FF:3F:40

 DNS and DHCP-based Web client 

Circuit:
* Ethernet shield attached to pins 10, 11, 12, 13 
*/

#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>
#include <SimpleDHT.h>

// MAC.
byte mac[] = {0x4C, 0x74, 0xBF, 0xA8, 0x4D, 0x60};
//IP del servidor.
char serverName[] = "88.15.231.148";

//byte mac[] = {0x08,0xC5,0xE1,0xFF,0x3F,0x40};
//PROFESOR
//byte mac[] = {0x18,0x03,0x73,0xD3,0x69,0x53}; 



// Inicializamos las librerias:
// Ethernet
EthernetClient client;
// Sensor temperatura humedad (dht11)
int pinDHT11 = 9;
SimpleDHT11 dht11;

// ************************************ PINOUT ***********************************

//##- Relés -##
const int rele1 =  18;
const int rele2 =  15;
const int rele4 =  26;

//##- Pulsadores y finales de carrera -##
const int FC1 = 12;
const int FC2 = 11;
const int FC3 = 8;
const int Pulsador_PersianaArriba = 6;
const int Pulsador_PersianaAbajo = 5;
const int Pulsador_Bombilla = 2;
const int Pulsador_Ventilador = 25;

//##- Speaker -##
const int Speaker = 37;

//##- Motores -##
//MOTOR A
const int IN1 = 53;
const int IN2 = 44;
const int ENA = 48;

//##- LCD -##
const int rs = 33, en = 34, d4 = 35, d5 = 36, d6 = 39, d7 = 40;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


//##- Temporizador -##
unsigned long startMillis = millis();
unsigned long currentMillis;
const unsigned long period = 20000;
// *******************************************************************************

// ************************************ SETUP ************************************

void setup() {

  
  // ***************************** ENTRADAS Y SALIDAS ****************************
    pinMode(rele1, OUTPUT);
    pinMode(rele2, OUTPUT);
    pinMode(rele4, OUTPUT);
    
    pinMode(FC1, INPUT);
    pinMode(FC2, INPUT);
    pinMode(FC3, INPUT);
    pinMode(Pulsador_PersianaArriba, INPUT);
    pinMode(Pulsador_PersianaAbajo, INPUT);
    pinMode(Pulsador_Bombilla, INPUT);
    pinMode(Pulsador_Ventilador, INPUT);

    pinMode(Speaker, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENA, OUTPUT);
    analogWrite(ENA, 255);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);



 // Inicializamos el LCD: (16x2):
 lcd.begin(16, 2);
 // Inicializamos el serial:
 Serial.begin(9600);
 // Inicializamos la conexion ethernet:
 if (Ethernet.begin(mac) == 0) {
   Serial.println("Failed to configure Ethernet using DHCP");
   // no point in carrying on, so do nothing forevermore:
   while(true);
 }
 // Le damos tiempo al escudo ethernet para que se inicialice:
 delay(2000);
 Serial.println("connecting...");
 
 // Si tenemos conexion, devovlemos un mensaje por serial: 
 if (client.connect(serverName, 80)) {
   Serial.println("Existe conexion con el servidor");
 } 
 else {
   // Si no tenemos conexion:
   Serial.println("No es posible conectar con el servidor. Reinicia el sistema");
   
 }

startMillis = millis();  //Tiempo de inicio de Loop
Serial.println(startMillis);

  
}

// *******************************************************************************

// ********************************** LOOP ***************************************


   
void loop(){

  Serial.println(startMillis);
  currentMillis = millis();
  Serial.println(currentMillis);
  
  String informacion = "";
  String informacion_salon = "";
  String informacion_habitacion = "";
  String informacion_garaje = "";
  String informacion_climatizacion = "";
  String webString = "";
  String posicion_persiana = "";
  String estado_sensor ="";

  // SENSOR DE TEMPERATURA
  // El sensor de temperatura no queremos que se este ejecutando constantemente ya que no es necesario y puede
  //  provocar lentitud y mas errores. Haremos que se ejecuten cada minuto.
   if (currentMillis - startMillis >= period){
        byte temperature = 0;
        byte humidity = 0;
        int err = SimpleDHTErrSuccess;
        
        if ((err = dht11.read(pinDHT11, &temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
          //Si falla no hacemos nada
        }
        else {
          //En caso de que este bien lo metemos en la pantalla.
              Serial.println("");
              Serial.print((int)temperature);
              Serial.print(" Celcius, ");        
              Serial.print((int)humidity);
              Serial.println(" %");
              Serial.println("");
                 
              //Limpiamos pantalla
              lcd.clear(); 
              //Nos situamos en el primer cuadro
              lcd.setCursor(0,0);
              //Escribimos la temperatura en la fila 1
              lcd.print("Temp(C): ");
              lcd.print((int)temperature);
              //Nos situamos en la segunda fila y escribimos humedad
              lcd.setCursor(0,1);
              lcd.print("Humedad(%): ");
              lcd.print((int)humidity);
          // Mandamos la informacion al servidor
              String PostData = "pRoom=Climatizacion&pObjeto=Temperatura (C)&pRele=0&pValor=";
              PostData = PostData + (int)temperature;
              PostData = PostData + "&pID=5&pReposo=1&pType=integer" ;
              Post(PostData);
              PostData = "pRoom=Climatizacion&pObjeto=Humedad relativa&pRele=0&pValor=";
              PostData = PostData + (int)humidity;
              PostData = PostData + "&pID=6&pReposo=1&pType=integer" ;
              Post(PostData);
          startMillis = millis();
        }
   }
    
   // Ahora que tenemos toda la informacion en el servidor, la leemos.
    informacion = peticion();   
    //Serial.println("El resultado es: ");
    //Serial.println(informacion);

  // Procesamos la informacion para cada habitación
    // SALON
        informacion_salon = informacion.substring(0,77);
          Serial.println("Info salon: ");
          Serial.println(informacion_salon);
          Serial.println();
          Serial.println("Estado Bombilla salon: " + informacion_salon.substring(61,62) );
    // HABITACION
        informacion_habitacion = informacion.substring(81,167);
        Serial.println();
        Serial.println("Info habitacion: "+informacion_habitacion);
        //la persiana puede estar en:
        //  1 -> Cerrada
        //  2 -> Entreabierta
        //  3 -> Abierta
        Serial.println();
        // String informacion_persiana = informacion_habitacion.substring(70,71);
        Serial.println("Estado persiana habitacion: "+informacion_habitacion.substring(70,71));
    // GARAJE (Regleta)
        informacion_garaje = informacion.substring(555,646);
         Serial.println();
         Serial.println("Info garaje: ");
         Serial.println(informacion_garaje);
         Serial.println(informacion_garaje.substring(65,66));
    // CLIMATIZACION (Ventilador)
        informacion_climatizacion = informacion.substring(257,348);
        Serial.println();
        Serial.println("Info climatizacion: ");
        Serial.println(informacion_climatizacion);
        Serial.print("Ventilador:");
        Serial.print(informacion_climatizacion.substring(75,76));
  // CONTROLAMOS LOS ELEMENTOS INSTALADOS.
        //CONTROL DE VENTILADOR
          if (informacion_climatizacion.substring(75,76) == "1"){
            digitalWrite(rele4, HIGH);
         //   Serial.println("Lo enciendo");
          }
          else if (informacion_climatizacion.substring(75,76) == "0"){
            digitalWrite(rele4, LOW);
          //  Serial.println("Lo apago");
          }
          else{
            //No deberia darse este caso, a no ser que falle la comunicacion con el servidor
            //asi que no haremos nada, pero lo programamos porque nos puede ser de utilidad
            //controlar por ejemplo la cantidad de veces que falla.
          }
        //CONTROL REGLETA GARAJE
          if (informacion_garaje.substring(65,66) == "1"){
            digitalWrite(rele2, HIGH);
         //   Serial.println("Lo enciendo");
          }
          else if (informacion_garaje.substring(65,66) == "0"){
            digitalWrite(rele2, LOW);
          //  Serial.println("Lo apago");
          }
          else{
            //No deberia darse este caso, a no ser que falle la comunicacion con el servidor
            //asi que no haremos nada, pero lo programamos porque nos puede ser de utilidad
            //controlar por ejemplo la cantidad de veces que falla.
          }
        //CONTROL DE BOMBILLA
          if (informacion_salon.substring(61,62) == "1"){
            digitalWrite(rele1, HIGH);
         //   Serial.println("Lo enciendo");
          }
          else if (informacion_salon.substring(61,62) == "0"){
            digitalWrite(rele1, LOW);
          //  Serial.println("Lo apago");
          }
          else{
            //No deberia darse este caso, a no ser que falle la comunicacion con el servidor
            //asi que no haremos nada, pero lo programamos porque nos puede ser de utilidad
            //controlar por ejemplo la cantidad de veces que falla.
          }
        // CONTROL DE PERSIANA.
            // Primero debemos saber en que situacion nos encontramos (ARRIBA|MEDIO|ABAJO|ERROR)
              if (digitalRead(FC1)==HIGH && digitalRead(FC2)==LOW && digitalRead(FC3)==LOW){
                //estamos abajo y sin errores
                posicion_persiana = "abajo";
              }
              else if (digitalRead(FC1)==LOW && digitalRead(FC2)==HIGH && digitalRead(FC3)==LOW ){
                //estamos en el medio y sin errores
                posicion_persiana = "medio";
              }
              else if (digitalRead(FC1)==LOW && digitalRead(FC2)==LOW && digitalRead(FC3)==HIGH ){
                //estamos arriba y sin errores
                posicion_persiana = "arriba";      
              }
              else{
                // Existe algun error, no estamos en ningun posicion
                posicion_persiana = "error";
              }
          
           // Ahora comparamos donde estamos (ARRIBA|MEDIO|ABAJO|ERROR) con donde tenemos que estar (3(arriba)|2|1)
          
             if(informacion_habitacion.substring(70,71)=="3"){
                while (digitalRead(FC3)==LOW) {
                  Serial.println("Subiendo hasta arriba");
                    digitalWrite(IN1, LOW);
                    digitalWrite(IN2, HIGH);
                }
                  Serial.println("Llegamos arriba");
                  digitalWrite(IN1, LOW);
                  digitalWrite(IN2, LOW);
              }
              if(informacion_habitacion.substring(70,71)=="1"){
                while (digitalRead(FC1)==LOW){
                  Serial.println("Bajando hasta abajo");
                  digitalWrite(IN1, HIGH);
                  digitalWrite(IN2, LOW);
                }
                Serial.println("Llegamos abajo");
                digitalWrite(IN1, LOW);
                digitalWrite(IN2, LOW);
              }
              if(informacion_habitacion.substring(70,71)=="2" && posicion_persiana=="arriba"){
                while(digitalRead(FC2)==LOW){
                  Serial.println("Bajando hasta medio");
                  digitalWrite(IN1, HIGH);
                  digitalWrite(IN2, LOW);
                }
                Serial.println("Llegamos al medio");
                digitalWrite(IN1, LOW);
                digitalWrite(IN2, LOW);
              }
              if(informacion_habitacion.substring(70,71)=="2" && posicion_persiana=="abajo"){
                while(digitalRead(FC2)==LOW){
                  Serial.println("Subiendo hasta medio");
                  digitalWrite(IN1, LOW);
                  digitalWrite(IN2, HIGH);
                }
                Serial.println("Llegamos al medio");
                digitalWrite(IN1, LOW);
                digitalWrite(IN2, LOW);
              }
              else{
                //Serial.println("Existe algun error con la persiana, no moveremos nada");
                digitalWrite(IN1, LOW);
                digitalWrite(IN2, LOW);
              }
            //while(true);
            //delay(5000);



  // ############### PULSADORES ###############
    // ########################################### BOMBILLA ########################################
        Serial.println();
        Serial.println("Pulsador Bombilla: " + digitalRead(Pulsador_Bombilla) );
        
        if (digitalRead(Pulsador_Bombilla)==HIGH){
          while(digitalRead(Pulsador_Bombilla)==HIGH){
            Serial.println("pulsador de bombilla pulsado, esperando a que se suelte");
            delay(5);
          }
          //Vemos si la bombilla esta encendida o apagada para mandar un 1 o un 0 en funcion de ello.
          if(digitalRead(rele1)==HIGH){
            Serial.println("Bombilla encendida, la apagamos. (pulsador)");
            String PostData = "pRoom=Salon&pObjeto=Luz1&pRele=1&pValor=1&pID=1&pReposo=1&pType=boolean";
            Post(PostData);
          }
          else{
            Serial.println("Bombilla apagado, la encendemos. (pulsador)");
            String PostData = "pRoom=Salon&pObjeto=Luz1&pRele=1&pValor=0&pID=1&pReposo=1&pType=boolean";
            Post(PostData);
            Serial.println(webString);
          }  
        }
    // ################################################################################################

    // ########################################### VENTILADOR ########################################
        Serial.println();
        Serial.println("Pulsador Ventilador: " + digitalRead(Pulsador_Ventilador) );
        
        if (digitalRead(Pulsador_Ventilador)==HIGH){
          while(digitalRead(Pulsador_Ventilador)==HIGH){
            Serial.println("pulsador de ventilador pulsado, esperando a que se suelte");
            delay(5);
          }
          //Vemos si la bombilla esta encendida o apagada para mandar un 1 o un 0 en funcion de ello.
          if(digitalRead(rele4)==HIGH){
            Serial.println("Ventilador encendido, lo apagamos. (pulsador)");
            String PostData = "pRoom=Climatizacion&pObjeto=Ventilador&pRele=4&pValor=1&pID=4&pReposo=1&pType=boolean";
            Post(PostData);
          }
          else{
            digitalWrite(rele4,HIGH);
            Serial.println("Ventilador apagado, lo encendemos. (pulsador)");
            String PostData = "pRoom=Climatizacion&pObjeto=Ventilador&pRele=4&pValor=0&pID=4&pReposo=1&pType=boolean";
            Post(PostData);
            Serial.println(webString);
          }  
        }
    // ################################################################################################

    
    // ##################################### PERSIANA ARRIBA ##########################################
        // OJO LOS FINALES DE CARRERA MAGNETICOS VAN AL CONTRARIO.
        // DAN SIEMPRE 5 VOLTIOS Y CUANDO ESTAN PULSADOS DAN 0  
        Serial.println();
        Serial.print("Pulsador PersianaArriba: ");
        Serial.println(digitalRead(Pulsador_PersianaArriba));
        Serial.print(" FC1(abajo): ");
        Serial.println(digitalRead(FC1));
        Serial.print(" FC2(medio): ");
        Serial.println(digitalRead(FC2));
        Serial.print(" FC3(arriba): ");
        Serial.println(digitalRead(FC3));
        
        if (digitalRead(Pulsador_PersianaArriba)==HIGH){
          int contador = 0;
          while(digitalRead(Pulsador_PersianaArriba)==HIGH){   
            //EN EL CASO DE LA PERSIANA, CONTROLAMOS CUANTO TIEMPO ESTA PULSADO,
            // ASI SI LO DEJAMOS MAS DE 5 SEGUNDOS SUBIRA O BAJARA TOTALMENTE
            // EN CASO CONTRARIO AVANZARA HASTA LA SIGUIENTE POSICION (DE LAS 3 QUE HAY)
            // CUANDO SE SUPERE EL TIEMPO DE ESPERA, SONARA UN PITIDO PARA INDICAR QUE YA PODEMOS SOLTAR
            Serial.println("Pulsador_PersianaArriba pulsado, esperando a que se suelte");
            delay(50);
            contador = contador + 50;
            if (contador > 2000){
              //*pendiente hacer que pite el speaker.
              while (digitalRead(Pulsador_PersianaArriba)==HIGH){
                digitalWrite(Speaker, HIGH);
                delay(300);
                digitalWrite(Speaker, LOW);
                Serial.println("Han pasado 4 segundos ya puedes soltar");
                delay(500);
              }
            }
          }
          Serial.println();
          Serial.print("El valor del contador es:");
          Serial.println(contador);
          
          //Aqui es cuando ya dejamos de soltar.
          //Ahora tenemos que ver ademas en que posicion estamos.
        
          //Si estamos abajo de todo (FC1) y pulsamos menos de 4 segundos -> IREMOS hasta FC2
          if( contador < 2000 && digitalRead(FC1)==HIGH ){
            Serial.println("Subimos desde FC1 a FC2");
            String PostData = "pRoom=Habitacion&pObjeto=Persiana&pRele=2&pValor=2&pID=2&pReposo=1&pType=persiana";
            Post(PostData);
            //*pendiente movernos hasta FC2
          }
          //Si estamos en el medio (FC2) y pulsamos menos de 4 segundos -> IREMOS hasta FC3
          if (contador < 2000 && digitalRead(FC2)==HIGH){
            Serial.println("Subimos desde FC2 a FC3");
            String PostData = "pRoom=Habitacion&pObjeto=Persiana&pRele=2&pValor=3&pID=2&pReposo=1&pType=persiana";
            Post(PostData);
            //*pendiente movernos hasta FC3
          }
          //Si estamos en FC3 y pulsamos arriba no hacemos nada da igual si son 4 o menos segundos
          if (digitalRead(FC3)==HIGH){
            Serial.println("Ya estamos arriba, no se puede subir mas");
          }
          //En cualquier otro caso (es decir, que no estemos en FC3 y si pulsamos mas de 4 segundos,subimos totalmente)
          if (contador > 2000){
            Serial.println("Subiendo arriba de todo");
            String PostData = "pRoom=Habitacion&pObjeto=Persiana&pRele=2&pValor=3&pID=2&pReposo=1&pType=persiana";
            Post(PostData);
          }  
        }
    // ################################################################################################
    
    // ####################################### PERSIANA ABAJO #########################################
          
          if (digitalRead(Pulsador_PersianaAbajo)==HIGH){
            int contador = 0;
            while(digitalRead(Pulsador_PersianaAbajo)==HIGH){   
              //EN EL CASO DE LA PERSIANA, CONTROLAMOS CUANTO TIEMPO ESTA PULSADO,
              // ASI SI LO DEJAMOS MAS DE 5 SEGUNDOS SUBIRA O BAJARA TOTALMENTE
              // EN CASO CONTRARIO AVANZARA HASTA LA SIGUIENTE POSICION (DE LAS 3 QUE HAY)
              // CUANDO SE SUPERE EL TIEMPO DE ESPERA, SONARA UN PITIDO PARA INDICAR QUE YA PODEMOS SOLTAR
              Serial.println("Pulsador_PersianaAbajo pulsado, esperando a que se suelte");
              delay(50);
              contador = contador + 50;
              if (contador > 2000){
                //*pendiente hacer que pite el speaker.
                while (digitalRead(Pulsador_PersianaAbajo)==HIGH){
                  digitalWrite(Speaker, HIGH);
                  delay(300);
                  digitalWrite(Speaker, LOW);
                  Serial.println("Han pasado 4 segundos ya puedes soltar");
                  delay(500);
                }
              }
            }
            Serial.println();
            Serial.print("El valor del contador es:");
            Serial.println(contador);
            
            //Aqui es cuando ya dejamos de soltar.
            //Ahora tenemos que ver ademas en que posicion estamos.
          
            //Si estamos arriba de todo (FC3) y pulsamos menos de 4 segundos -> IREMOS hasta FC2
            if( contador < 2000 && digitalRead(FC3)==HIGH ){
              Serial.println("Bajamos desde FC3 a FC2");
              String PostData = "pRoom=Habitacion&pObjeto=Persiana&pRele=2&pValor=2&pID=2&pReposo=1&pType=persiana";
              Post(PostData);
              //*pendiente movernos hasta FC2
            }
            //Si estamos en el medio (FC2) y pulsamos menos de 4 segundos -> IREMOS hasta FC1
            if (contador < 2000 && digitalRead(FC2)==HIGH){
              Serial.println("Bajamos desde FC2 a FC1");
              String PostData = "pRoom=Habitacion&pObjeto=Persiana&pRele=2&pValor=1&pID=2&pReposo=1&pType=persiana";
              Post(PostData);
              //*pendiente movernos hasta FC3
            }
            //Si estamos en FC1 y pulsamos abajo no hacemos nada da igual si son 4 o menos segundos
            if (digitalRead(FC1)==HIGH){
              Serial.println("Ya estamos arriba, no se puede subir mas");
            }
            //En cualquier otro caso (es decir, que no estemos en FC1 y si pulsamos mas de 4 segundos,bajamos totalmente)
            if (contador > 2000){
              Serial.println("Bajando de todo");
              String PostData = "pRoom=Habitacion&pObjeto=Persiana&pRele=2&pValor=1&pID=2&pReposo=1&pType=persiana";
              Post(PostData);
            }
          }
      // ################################################################################################

//Se cierra loop
}


// DECLARACION DE FUNCIONES -----------------------------------------------------------------------------

String peticion(){ //En esta funcion hacemos la peticion y guardamos el resultado en 'request'
  
  String requestString;

  // Nos conectamos al servidor y hacemos la peticion
  if (client.connect(serverName, 80)){
    //Serial.println("Realizando peticion");
    client.println("GET /arduino.php HTTP/1.0");
    client.println();
    delay(300);
    // Si se recibe informacion la voy guardando en la variable 'request'
    // Guardamos la info hasta que deja de llegar, que volvemos al loop devolviendo el resultado
     while (true){
      if (client.available()){
      char temp = client.read();
      requestString += temp;
      }
      //Serial.print(requestString);
       if (!client.connected()) {
        // Serial.println("Se cumple la condicion de salida");
         //Serial.print(requestString);
         //Serial.println();
         //Serial.println("disconnecting.");
         //Serial.println("Saliendo");
         //Serial.println(requestString);
         String info = requestString.substring(requestString.indexOf("_{")+1,requestString.indexOf("}<br>_")+1);
         //Serial.println(info);
         return info;
       }
     }
  }
  else {
    client.connect(serverName, 80);
    Serial.println("Rearmando...");
    delay(1000);
    String info = "aa";
    return info;
  }
}

//------------------------------------------------------------------------------------------------------------

String Post(String PostData){
  // Esta es la funcion que nos permitira hacer un post cuando pulsemos alguno de los botones y por lo tanto,
  // que se actualice el valor en la BBDD. Posteriormente arduino leera los cambios de la BD y actualizara
  // el estado de las salidas.

   String webString = "";
  // Nos conectamos al servidor y hacemos la peticion
    if (client.connect(serverName, 80)){
      client.println("POST /update.php HTTP/1.0");
      client.println("Host: 88.15.231.148");
      client.println("User-Agent: Arduino/1.0");
      client.println("Accept: */*");
      client.println("Accept-Language: es-ES,es;q=0.8,en-US;q=0.5,en;q=0.3");
      client.println("Accept-Encoding: gzip, deflate");
      client.println("Referer: http://88.15.231.148/");
      client.println("Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
      client.println("X-Requested-With: XMLHttpRequest");
      client.print("Content-Length:");
      client.println(PostData.length());
      client.println("Connection: keep-alive");
      client.println();
      client.println(PostData);
      Serial.println(PostData);
      Serial.println();
    }
    else {
      client.connect(serverName, 80);
      Serial.println("Post fallido -> Rearmando...");
      delay(1000);
    }

    while (client.available() == 0) {
      Serial.println("Estoy esperando");
      //Espero respuesta del servidor
    }

     if (client.available()) {
    Serial.println("Respuesta del Servidor---->");
    while (client.available()) {
      char c = client.read();
      webString += c;
    }
    return webString;
   }   
}

//------------------------------------------------------------------------------------------------------------

String SensorTemp(){
  //Esta funcion lo que haces es poner el valor de la temperatura y la humedad
  //  en la pantalla LCD y mandar un POST de los valores a la pagina Web.
  String INFO="";
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  
  if ((err = dht11.read(pinDHT11, &temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("No reading , err="); Serial.println(err);delay(1000);
    return INFO;
  }
  
  Serial.print("Readings: ");
  Serial.print((int)temperature); Serial.print(" Celcius, ");
  Serial.print((int)humidity); Serial.println(" %");
 
  //Limpiamos pantalla
  lcd.clear(); 
  //Nos situamos en el primer cuadro
  lcd.setCursor(0,0);
  //Escribimos la temperatura en la fila 1
  lcd.print("Temp: ");
  lcd.print((int)temperature);
  //Nos situamos en la segunda fila y escribimos humedad
  lcd.setCursor(0,1);
  lcd.print("Humidity(%): ");
  lcd.print((int)humidity); 
  
  // Enviamos los valores al servidor para que aparezcan en la web.
  
  // Volvemos al bucle principal
  INFO = "Actualizado.";
  return INFO;
}

//------------------------------------------------------------------------------------------------------------
