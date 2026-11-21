#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

// ======== CONFIGURE AQUI ========
const char* WIFI_SSID  = "Convidado iot";
const char* WIFI_SENHA = "mescla@1417";
const char* PROXIMO_IP = "192.168.1.105";
// ================================

const unsigned long ATRASO_MS = 2000;       // espera antes de repassar o token

ESP8266WebServer server(80);

// Estado do token (compartilhado entre o handler e o loop)
bool          envioPendente    = false;
long          valorParaEnviar  = 0;
unsigned long instanteRecebido = 0;

// ---------- Rota /relay ----------
void handleRelay() {
  if (!server.hasArg("valor")) {
    server.send(400, "text/plain", "Erro: use /relay?valor=N");
    return;
  }

  long valor = server.arg("valor").toInt();
  Serial.printf("[RELAY] Token recebido: %ld\n", valor);

  if (envioPendente) {
    Serial.println("[RELAY] Aviso: havia um token pendente, sera substituido.");
  }

  // Apenas agenda o envio -- NÃO espera nem faz requisição aqui dentro
  valorParaEnviar  = valor + 1;
  instanteRecebido = millis();
  envioPendente    = true;

  // Responde imediatamente para liberar quem nos chamou
  server.send(200, "text/plain",
              "OK! Recebi " + String(valor) +
              ", repassarei " + String(valorParaEnviar) + " em 2 s.");
}

// ---------- Cliente: envia para o próximo ----------
void enviarParaProximo() {
  WiFiClient client;
  HTTPClient http;

  String url = String("http://") + PROXIMO_IP + "/relay?valor=" + String(valorParaEnviar);
  Serial.printf("[RELAY] Enviando %ld -> %s\n", valorParaEnviar, url.c_str());

  http.setTimeout(3000);  // limita o tempo que podemos ficar presos se o vizinho cair

  if (!http.begin(client, url)) {
    Serial.println("[RELAY] Falha ao iniciar a conexao.");
    return;
  }

  int codigo = http.GET();
  if (codigo > 0) {
    Serial.printf("[RELAY] Resposta do proximo: HTTP %d\n", codigo);
  } else {
    Serial.printf("[RELAY] Erro no envio: %s\n", http.errorToString(codigo).c_str());
  }
  http.end();
}

void handleRaiz() {
  server.send(200, "text/plain", "Token Ring ativo. Use /relay?valor=N");
}

void setup() {
  Serial.begin(115200);
  delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  Serial.print("\nConectando");
  while (WiFi.status() != WL_CONNECTED) {   // só no setup, antes do servidor existir
    delay(300);
    Serial.print(".");
  }
  Serial.printf("\nConectado! Meu IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Proximo da cadeia: %s\n", PROXIMO_IP);

  server.on("/", handleRaiz);
  server.on("/relay", handleRelay);
  server.begin();
  Serial.println("Servidor HTTP iniciado.");
}

void loop() {
  server.handleClient();   // servidor sempre atendendo

  // Temporização não-bloqueante com millis()
  if (envioPendente && (millis() - instanteRecebido >= ATRASO_MS)) {
    envioPendente = false;
    enviarParaProximo();
  }
}
