#include <Wire.h> 
#include <DHT.h>
#include <Ethernet.h>

#define DHTPIN 2
#define DHTTYPE DHT11
#define HISTORY_SIZE 10

DHT dht(DHTPIN, DHTTYPE);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Cambiar por tu dirección MAC
IPAddress ip(192, 168, 0, 180); // Cambiar por la dirección IP deseada
EthernetServer server(80);

float temperaturaHistory[HISTORY_SIZE];
float humedadHistory[HISTORY_SIZE];
int historyIndex = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Conectando con el servidor...");

  // Inicialización de Ethernet y servidor web
  Ethernet.begin(mac, ip);
  server.begin();

  // Mensaje de conexión exitosa
  Serial.println("Conexión exitosa");

  // Inicialización del sensor DHT
  dht.begin();
}

void loop() {
  // Verificar si hay una conexión entrante
  EthernetClient client = server.available();
  if (client) {
    // Si hay una conexión, procesarla
    while (client.connected()) {
      if (client.available()) {
        // Respuesta HTTP
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");
        client.println("Refresh: 1");
        client.println();

        // Inicio del contenido HTML
        client.println("<!DOCTYPE HTML>");
        client.println("<html><head><title>Lectura del Sensor</title>");
        // Estilo CSS para el diseño de la página
        client.println("<style>");
        client.println("body { font-family: Arial, sans-serif; background-color: #f6f6f6; }");
        client.println("h1 { color: #333333; text-align: center; }");
        client.println("p { color: #666666; }");
        client.println(".container { max-width: 600px; margin: 20px auto; padding: 20px; background-color: #ffffff; border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }");
        client.println(".reading { text-align: center; margin-top: 30px; }");
        client.println(".temperature { color: #ff6347; font-size: 24px; }");
        client.println(".humidity { color: #4682b4; font-size: 24px; }");
        client.println("</style>");
        client.println("</head><body>");
        // Encabezado
        client.println("<header style='background-color: #4caf50; color: #ffffff; padding: 20px 0;'>");
        client.println("<h1>Lectura del Sensor</h1>");
        client.println("</header>");
        // Contenedor principal
        client.println("<div class='container'>");

        // Lectura de temperatura y humedad
        float humedad = dht.readHumidity();
        float temperatura = dht.readTemperature();

        // Guardar la lectura en el historial
        temperaturaHistory[historyIndex] = temperatura;
        humedadHistory[historyIndex] = humedad;
        historyIndex = (historyIndex + 1) % HISTORY_SIZE;

        // Verificar si las lecturas son válidas
        if (!isnan(humedad) && !isnan(temperatura)) {
          // Imprimir las últimas lecturas en la página web
          client.println("<div class='reading'>");
          client.println("<h2>Historial de Lecturas:</h2>");
          for (int i = 0; i < HISTORY_SIZE; i++) {
            int index = (historyIndex + i) % HISTORY_SIZE;
            client.print("<p><strong>Lectura ");
            client.print(i + 1);
            client.print(":</strong> Temperatura: ");
            client.print(temperaturaHistory[index]);
            client.print(" &#176;C, Humedad: ");
            client.print(humedadHistory[index]);
            client.println(" %</p>");
          }
          client.println("</div>");
        } else {
          // En caso de error en las lecturas
          Serial.println("Error al leer el sensor");
          client.println("<p class='error'>Error al leer el sensor</p>");
        }

        // Fin del contenido HTML
        client.println("</div>");
        client.println("</body></html>");
        break;
      }
    }
    // Esperar un breve tiempo antes de cerrar la conexión
    delay(1);
    client.stop();
  }

  // Esperar un tiempo antes de realizar la siguiente lectura
  delay(2000);
}
