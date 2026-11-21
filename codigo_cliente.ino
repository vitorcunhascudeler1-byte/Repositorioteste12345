// dupla: arthur pedruzi e vitor cunha com joao piazentin e vinicius luperini


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ==================================================
// WI-FI
// ==================================================

const char* ssid = "Convidado iot";
const char* password = "mescla@1417";

// ==================================================
// IP DO SEU ESP
// ==================================================

// Coloque aqui o IP que apareceu no SEU Monitor Serial
IPAddress ipColega(192, 168, 1, 120);

// ==================================================
// SERVIDOR HTTP
// ==================================================

ESP8266WebServer server(80);

// ==================================================
// CONTROLE DO TEMPO
// ==================================================

unsigned long ultimoEnvio = 0;
const unsigned long intervalo = 10000;


// ==================================================
// FUNÇÃO /toggle
// ==================================================

void toggle() {

  // Inverte o estado do LED interno (GPIO 2)
  digitalWrite(2, !digitalRead(2));

  Serial.println("GET /toggle recebido.");
  Serial.println("LED invertido.");

  // Responde "OK"
  server.send(200, "text/plain", "OK");
}


// ==================================================
// SETUP
// ==================================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("====================================");
  Serial.println("        ESP8266 - PING PONG");
  Serial.println("====================================");

  // LED interno
  pinMode(2, OUTPUT);

  // LED desligado
  digitalWrite(2, HIGH);


  // ==================================================
  // CONECTAR AO WI-FI
  // ==================================================

  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);

  Serial.print("Conectando ao Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();
  Serial.println();

  Serial.println("Wi-Fi conectado!");


  // ==================================================
  // MOSTRAR O IP DO ESP DO COLEGA
  // ==================================================

  Serial.print("IP DESTE ESP: ");
  Serial.println(WiFi.localIP());

  Serial.println();


  // ==================================================
  // CONFIGURAR SERVIDOR HTTP
  // ==================================================

  server.on("/toggle", HTTP_GET, toggle);

  server.begin();

  Serial.println("Servidor HTTP iniciado.");
  Serial.println("Porta: 80");
  Serial.println("Rota: /toggle");

  Serial.println();

  Serial.print("IP DO SEU ESP: ");
  Serial.println(ipColega);

  Serial.println();
  Serial.println("Aguardando...");
}


// ==================================================
// LOOP
// ==================================================

void loop() {

  // Mantém o servidor HTTP funcionando
  server.handleClient();


  // ==================================================
  // A CADA 10 SEGUNDOS, ENVIA GET /toggle
  // ==================================================

  if (millis() - ultimoEnvio >= intervalo) {

    ultimoEnvio = millis();

    Serial.println();
    Serial.println("Enviando GET /toggle para o seu ESP...");

    WiFiClient client;

    // Conecta ao seu ESP pela porta 80
    if (client.connect(ipColega, 80)) {

      Serial.println("Conectado ao seu ESP.");

      // Envia requisição HTTP GET
      client.println("GET /toggle HTTP/1.1");

      client.print("Host: ");
      client.println(ipColega);

      client.println("Connection: close");
      client.println();


      // ==================================================
      // AGUARDA A RESPOSTA
      // ==================================================

      unsigned long inicio = millis();

      while (!client.available() && millis() - inicio < 3000) {
        delay(10);
      }


      // ==================================================
      // MOSTRA A RESPOSTA
      // ==================================================

      if (client.available()) {

        String resposta = client.readString();

        Serial.println("Resposta recebida:");
        Serial.println(resposta);

      } else {

        Serial.println("Nenhuma resposta recebida.");

      }

      client.stop();

    } else {

      Serial.println("ERRO: nao foi possivel conectar ao seu ESP.");

    }
  }
}