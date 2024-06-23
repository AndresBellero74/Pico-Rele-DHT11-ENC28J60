
#include <SPI.h>
#include <EthernetENC.h>
#include <DHT11.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

int lampara=8; //Pin de la lÃ¡mpara// seleccionamos el pin donde conectaremos el rele que activara la lampara
int estado;// creamos una variable para ver si la lampara actualmente estÃ¡ activada o no
DHT11 dht11(14);


void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  Ethernet.init(17);   // MKR ETH Shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit FeatherWing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit FeatherWing Ethernet

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Ethernet WebServer Example");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  pinMode(lampara,OUTPUT);//configuramos el pin 8 como salida para activar o desactivar la lampara
  digitalWrite(8,HIGH);//inicialmente a nivel alto ya que el relÃ© es activo a nivel bajo

  pinMode(15,OUTPUT);
  digitalWrite(15,HIGH);
}

void loop()
{
   // Attempt to read the temperature value from the DHT11 sensor.
    int temperature = dht11.readTemperature();

    // Check the result of the reading.
    // If there's no error, print the temperature value.
    // If there's an error, print the appropriate error message.
    if (temperature != DHT11::ERROR_CHECKSUM && temperature != DHT11::ERROR_TIMEOUT) {
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println(" °C");
    } else {
        // Print error message based on the error code.
        Serial.println(DHT11::getErrorString(temperature));
    }
  EthernetClient client = server.available(); //Creamos un cliente Web
  //Cuando detecte un cliente a travÃ©s de una peticiÃ³n HTTP
  if (client) {
    Serial.println("new client");
    boolean currentLineIsBlank = true; //Una peticiÃ³n HTTP acaba con una lÃ­nea en blanco
    String comando=""; //cadena de caracteres vacÃ­a para guardar el estado del la lampara
    while (client.connected()) {//mientras el cliente este conectado y disponible
      if (client.available()) {
        char c = client.read();//Leemos la peticiÃ³n HTTP cada caracter individualmente
        Serial.write(c);
        comando.concat(c);//vamos agregando cada caracter de cada peticion para obtener la orden completa
 
         
         int poscomando=comando.indexOf("LAM="); //buscamos en el texto donde empieza el comando a ejecutar palabra LAM
         if(comando.substring(poscomando)=="LAM=ON")//Si en la posicion poscomando hay "LAM=ON"
          {
            digitalWrite(lampara,LOW);//encendemos la bombilla rele activo a nivel bajo
            
          }
          if(comando.substring(poscomando)=="LAM=OFF")//Si en la posicion poscomando hay "LAM=OFF"
          {
            digitalWrite(lampara,HIGH);//apagamos bombilla rele desactivo a nivel alto
        
          }        
        if (c == '\n' && currentLineIsBlank) {//comprobamos que ha acabado la peticiÃ³n con una linea en blanco
                     
            client.println("HTTP/1.1 200 OK");//Enviamos la respuesta de la peticion al cliente
            client.println("Content-Type: text/html");
            client.println();
            estado=digitalRead(lampara);//comprobamos si la lampara esta encendida o no
            client.println("<html>");//servimos la web a mostrar en HTML
            client.println("<head>");
            client.println("</head>");
            client.println("<body>");
            client.println("<h1>fbellsan Informatica</h1>");//titulo de nuestra web 
            client.print("<h2>Temperatura: ");  
            client.print(temperature);
            client.println("</h1>");        
            client.println("<h2>Control de rel&eacute; a trav&eacute;s de internet</h2>");//las tildes usando &_acute
            //con estos dos if de acontinuacion hacemos el juego de que se muestre junto al elemento radio button 
            //la opcion contraria al estado actual de la lampara 
            if (estado==1){//si la lampara esta apagada
            client.println("<radio onClick=location.href='./?LAM=ON\'>");//cuando se pinche envia el comando lam=on
            client.print("<P><input type=\"radio\"name=\"status\"value=\"1\">ENCENDER");//muestra el texto encender
            
            }
            if(estado==0){//si esta encendida muestra texto apagar
            client.println("<radio onClick=location.href='./?LAM=OFF\'>");//cuando se pinche da orden de apagar
            client.print("<P><input type=\"radio\"name=\"status\"value=\"1\">APAGAR");//muestra texto apagar
            
            }
            client.println("</html>");
            break;
        }
        //final de respuesta
        if (c == '\n') {
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    
    delay(1);//Esperamos a que reciba la respuesta
    client.stop();//Termina la conexiÃ³n
  }
}

