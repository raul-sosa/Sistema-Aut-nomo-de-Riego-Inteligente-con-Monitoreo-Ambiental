#include <WiFi.h>                 // Biblioteca para conexión Wi-Fi
#include <FirebaseESP32.h>        // Biblioteca para conexión con Firebase
#include <DHT.h>                  // Biblioteca para el sensor de temperatura y humedad

// Configuración Wi-Fi
#define WIFI_SSID "TU_RED_WIFI"
#define WIFI_PASSWORD "TU_CONTRASEÑA_WIFI"

// Configuración Firebase
#define FIREBASE_HOST "tu-proyecto.firebaseio.com" // Cambia con tu URL de Firebase
#define FIREBASE_AUTH "tu-token-de-autenticacion" // Token de autenticación

// Pines del hardware
#define DHTPIN 4                  // Pin donde está conectado el sensor DHT
#define SOIL_SENSOR_PIN 34        // Pin analógico para sensor de humedad del suelo
#define RELAY_PIN 25              // Pin digital para controlar la válvula de riego

// Configuración del sensor DHT
#define DHTTYPE DHT22             // Cambia a DHT11 si usas este modelo
DHT dht(DHTPIN, DHTTYPE);

// Configuración de Firebase
FirebaseData firebaseData;

// Umbrales de riego
float soilMoistureThreshold = 40.0;  // Porcentaje mínimo de humedad en el suelo para activar riego

void setup() {
  // Configuración básica
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Apaga el riego inicialmente

  // Inicialización del sensor DHT
  dht.begin();

  // Conexión Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConexión Wi-Fi establecida");

  // Conexión a Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Serial.println("Conectado a Firebase");
}

void loop() {
  // Leer datos de sensores
  float soilMoisture = analogRead(SOIL_SENSOR_PIN); // Leer humedad del suelo
  soilMoisture = map(soilMoisture, 0, 4095, 0, 100); // Convertir a porcentaje
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Validar lecturas
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Error al leer datos del DHT");
    return;
  }

  // Mostrar datos por Serial
  Serial.printf("Humedad del Suelo: %.2f%%\n", soilMoisture);
  Serial.printf("Temperatura: %.2f°C\n", temperature);
  Serial.printf("Humedad Relativa: %.2f%%\n", humidity);

  // Subir datos a Firebase
  Firebase.setFloat(firebaseData, "/sensores/humedad_suelo", soilMoisture);
  Firebase.setFloat(firebaseData, "/sensores/temperatura", temperature);
  Firebase.setFloat(firebaseData, "/sensores/humedad", humidity);

  // Control del riego
  if (soilMoisture < soilMoistureThreshold) {
    digitalWrite(RELAY_PIN, LOW); // Activar riego
    Serial.println("Riego activado");
    Firebase.setBool(firebaseData, "/estado_riego", true);
  } else {
    digitalWrite(RELAY_PIN, HIGH); // Desactivar riego
    Serial.println("Riego desactivado");
    Firebase.setBool(firebaseData, "/estado_riego", false);
  }

  delay(10000); // Esperar 10 segundos antes de la siguiente lectura
}
