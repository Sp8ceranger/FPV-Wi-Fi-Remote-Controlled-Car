// ============================================
// Contrôle de voiture télécommandée via WiFi + Flux Vidéo
// Matériel : XIAO ESP32 S3 Sense / Adafruit Feather ESP32 2MB PSRAM+ TB6612FNG + 4 moteurs DC 5V
// Auteur : Vibe Code (pour Sp8ceranger)
// Documentation moteurs : https://passionelectronique.fr/tutoriel-tb6612fng/
// Code : 
// ============================================

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "esp_camera.h"

// --- Configuration WiFi ---
const char* ssid = "ESP32_Car";
const char* password = "12345678";  // Min. 8 caractères

// --- Configuration WebSocket ---
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// --- Broches TB6612FNG (à adapter selon ton câblage) ---
// Moteur 1 (Gauche)
#define IN1 1
#define IN2 2
#define PWM1 5

// Moteur 2 (Droit)
#define IN3 3
#define IN4 4
#define PWM2 10

// Marche - Arret
#define STBY 9

// --- Variables globales ---
int speed = 100;  // Vitesse par défaut (0-100%)
int joyX = 0;     // Position X du joystick (-100 à 100)
int joyY = 0;     // Position Y du joystick (-100 à 100)
unsigned long lastActivityTime = 0;
const unsigned long timeout = 5000;  // 5 secondes

// --- Configuration Caméra (Adafruit Feather ESP32 / XIAO ESP32 S3 Sense) ---
// Broches caméra pour Adafruit Feather ESP32
#define CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// Variable globale pour la config caméra
camera_config_t camera_config;

// ============================================
// Fonction pour envoyer une image JPEG via WiFiClient
// ============================================
void sendJPEG(WiFiClient &client, camera_fb_t *fb) {
  if (fb->format != PIXFORMAT_JPEG) {
    return;
  }
  
  // Envoi des headers
  client.print("--frame\r\n");
  client.print("Content-Type: image/jpeg\r\n");
  client.print("Content-Length: ");
  client.print(fb->len);
  client.print("\r\n\r\n");
  
  // Envoi des données binaires
  client.write(fb->buf, fb->len);
  client.print("\r\n");
}

// ============================================
// FONCTIONS POUR LES MOTEURS (À PERSONNALISER)
// ============================================

// Avancer (les 2 moteurs en avant)
void forward(int speedPercent) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGN);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, HIGH);
  analogWrite(PWM1, map(speedPercent, 0, 100, 0, 255));
  analogWrite(PWM2, map(speedPercent, 0, 100, 0, 255));
}

// Reculer (les 2 moteurs en arrière)
void backward(int speedPercent) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGN);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(PWM1, map(speedPercent, 0, 100, 0, 255));
  analogWrite(PWM2, map(speedPercent, 0, 100, 0, 255));
}

// Tourner à gauche (moteur gauche en arrière, moteur droit en avant)
void left(int speedPercent) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGN);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, HIGH);
  analogWrite(PWM1, map(speedPercent, 0, 100, 0, 255));
  analogWrite(PWM2, map(speedPercent, 0, 100, 0, 255));
}

// Tourner à droite (moteur gauche en avant, moteur droit en arrière)
void right(int speedPercent) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGN);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(PWM1, map(speedPercent, 0, 100, 0, 255));
  analogWrite(PWM2, map(speedPercent, 0, 100, 0, 255));
}

// Arrêter tous les moteurs
void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(PWM1, 0);
  analogWrite(PWM2, 0);
}

// ============================================
// Configuration initiale
// ============================================
void setup() {
  Serial.begin(115200);
  
  // Configuration des broches TB6612FNG
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(PWM1, OUTPUT);
  pinMode(PWM2, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, LOW);  // Désactive les moteurs au démarrage
  
  // Initialisation de la caméra
  camera_config.ledc_channel = LEDC_CHANNEL_0;
  camera_config.ledc_timer = LEDC_TIMER_0;
  camera_config.pin_d0 = Y2_GPIO_NUM;
  camera_config.pin_d1 = Y3_GPIO_NUM;
  camera_config.pin_d2 = Y4_GPIO_NUM;
  camera_config.pin_d3 = Y5_GPIO_NUM;
  camera_config.pin_d4 = Y6_GPIO_NUM;
  camera_config.pin_d5 = Y7_GPIO_NUM;
  camera_config.pin_d6 = Y8_GPIO_NUM;
  camera_config.pin_d7 = Y9_GPIO_NUM;
  camera_config.pin_xclk = XCLK_GPIO_NUM;
  camera_config.pin_pclk = PCLK_GPIO_NUM;
  camera_config.pin_vsync = VSYNC_GPIO_NUM;
  camera_config.pin_href = HREF_GPIO_NUM;
  camera_config.pin_sscb_sda = SIOD_GPIO_NUM;
  camera_config.pin_sscb_scl = SIOC_GPIO_NUM;
  camera_config.pin_pwdn = PWDN_GPIO_NUM;
  camera_config.pin_reset = RESET_GPIO_NUM;
  camera_config.xclk_freq_hz = 20000000;
  camera_config.pixel_format = PIXFORMAT_JPEG;
  camera_config.frame_size = FRAMESIZE_QVGA;  // 320x240
  camera_config.jpeg_quality = 10;  // Qualité JPEG (0-63)
  camera_config.fb_count = 2;  // 2 framebuffers pour éviter les blocages
  camera_config.grab_mode = CAMERA_GRAB_LATEST;

  // Initialisation de la caméra
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK) {
    Serial.printf("Erreur initialisation caméra: 0x%x", err);
    // On continue sans caméra si erreur
  } else {
    Serial.println("Caméra initialisée avec succès");
    
    // Configuration de la caméra
    sensor_t* s = esp_camera_sensor_get();
    s->set_vflip(s, 1);        // Retourner verticalement si nécessaire
    s->set_hmirror(s, 1);      // Retourner horizontalement si nécessaire
    s->set_brightness(s, 0);   // Luminosité (-2 à 2)
    s->set_contrast(s, 0);     // Contraste (-2 à 2)
  }
  
  // Démarrage du WiFi
  WiFi.softAP(ssid, password);
  Serial.println("Point d'accès démarré");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
  
  // Configuration du serveur web
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "max-age=604800");
    server.send(200, "text/html", getHTML());
  });
  server.on("/style.css", HTTP_GET, []() {
    server.send(200, "text/css", getCSS());
  });
  server.on("/script.js", HTTP_GET, []() {
    server.send(200, "application/javascript", getJS());
  });
  
  // Route pour le flux vidéo MJPEG
  server.on("/stream", HTTP_GET, []() {
    if (esp_camera_init(&camera_config) != ESP_OK) {
      server.send(500, "text/plain", "Caméra non disponible");
      return;
    }
    
    // Récupération du client WiFi
    WiFiClient client = server.client();
    
    // Envoi des headers MJPEG
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
    client.println("Connection: close");
    client.println();
    
    // Boucle d'envoi des images
    while (client.connected()) {
      camera_fb_t* fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Erreur capture image");
        break;
      }
      
      // Envoi de l'image
      sendJPEG(client, fb);
      
      // Libération du framebuffer
      esp_camera_fb_return(fb);
      
      // Petit délai pour limiter le FPS (environ 20 FPS)
      delay(50);
    }
  });
  
  server.begin();
  
  // Démarrage du WebSocket
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  Serial.println("Serveur prêt !");
  Serial.println("Connectez-vous au WiFi: " + String(ssid));
  Serial.println("Puis ouvrez: http://" + WiFi.softAPIP().toString());
}

// ============================================ ========================================== loop ============================================
// Boucle principale
// ============================================
void loop() {
  server.handleClient();
  webSocket.loop();
  // Mise à jour du temps d'activité si un client est connecté
  if (webSocket.connectedClients() > 0) {
    lastActivityTime = millis();
  }

  if (webSocket.connectedClients() > 0 || (millis() - lastActivityTime > timeout)) {
    digitalWrite(STBY, HIGH);
  }
  else {
    digitalWrite(STBY, LOW);
  }
  
  // Contrôle des moteurs en fonction du joystick
  controlMotors();
}

// ============================================ =============================================================================================
// Gestion des événements WebSocket
// ============================================
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_TEXT) {
    String data = (char*)payload;
    
    // Format attendu: "X:valeur,Y:valeur,S:speed"
    int x = 0, y = 0, s = 100;
    
    // Extraction des valeurs
    int xPos = data.indexOf("X:");
    int yPos = data.indexOf(",Y:");
    int sPos = data.indexOf(",S:");
    
    if (xPos != -1 && yPos != -1) {
      x = data.substring(xPos + 2, yPos).toInt();
      y = data.substring(yPos + 3, sPos != -1 ? sPos : data.length()).toInt();
    }
    if (sPos != -1) {
      s = data.substring(sPos + 3).toInt();
    }
    
    joyX = x;
    joyY = y;
    speed = constrain(s, 0, 100);  // Limite à 0-100%
    
    Serial.printf("X: %d, Y: %d, Speed: %d%%\n", joyX, joyY, speed);
  }
}

// ============================================
// Contrôle des moteurs (appelle les fonctions forward, backward, left, right)
// ============================================
void controlMotors() {
  // Seuil pour éviter les micro-mouvements (joystick au centre)
  const int deadZone = 20;
  
  if (joyY > deadZone) {
    // Avant
    if (joyX < -deadZone) {
      // Tourner à gauche en avançant (ralentir le moteur gauche)
      int leftSpeed = map(joyX, -100, -deadZone, 0, speed);
      int rightSpeed = speed;
      forward(rightSpeed);  // Le moteur droit avance à pleine vitesse
      analogWrite(PWM1, map(leftSpeed, 0, 100, 0, 255));  // Le moteur gauche ralentit
    } else if (joyX > deadZone) {
      // Tourner à droite en avançant (ralentir le moteur droit)
      int leftSpeed = speed;
      int rightSpeed = map(joyX, deadZone, 100, speed, 0);
      forward(leftSpeed);  // Le moteur gauche avance à pleine vitesse
      analogWrite(PWM2, map(rightSpeed, 0, 100, 0, 255));  // Le moteur droit ralentit
    } else {
      // Avancer tout droit
      forward(speed);
    }
  } else if (joyY < -deadZone) {
    // Arrière
    if (joyX < -deadZone) {
      // Tourner à gauche en reculant (ralentir le moteur gauche)
      int leftSpeed = map(joyX, -100, -deadZone, 0, speed);
      int rightSpeed = speed;
      backward(rightSpeed);  // Le moteur droit recule à pleine vitesse
      analogWrite(PWM1, map(leftSpeed, 0, 100, 0, 255));  // Le moteur gauche ralentit
    } else if (joyX > deadZone) {
      // Tourner à droite en reculant (ralentir le moteur droit)
      int leftSpeed = speed;
      int rightSpeed = map(joyX, deadZone, 100, speed, 0);
      backward(leftSpeed);  // Le moteur gauche recule à pleine vitesse
      analogWrite(PWM2, map(rightSpeed, 0, 100, 0, 255));  // Le moteur droit ralentit
    } else {
      // Reculer tout droit
      backward(speed);
    }
  } else {
    // Joystick au centre verticalement
    if (joyX > deadZone) {
      // Tourner à droite sur place
      right(speed);
    } else if (joyX < -deadZone) {
      // Tourner à gauche sur place
      left(speed);
    } else {
      // Arrêt complet
      stopMotors();
    }
  }
}

// ============================================
// HTML de la page web
// ============================================
String getHTML() {
  return R"=====(
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Contrôle Voiture - ESP32</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <h1>Contrôle Voiture + Caméra</h1>
        <p>Connecté à: <span id="ip"></span></p>
        
        <div class="layout">
            <div class="video-container">
                <img id="video-stream" src="/stream" alt="Flux vidéo">
            </div>
            
            <div class="controls-container">
                <div class="joystick-container">
                    <div id="joystick" class="joystick">
                        <div id="stick" class="stick"></div>
                    </div>
                </div>
                
                <div class="slider-container">
                    <label for="speed">Vitesse: <span id="speed-value">100%</span></label>
                    <input type="range" id="speed" min="0" max="100" value="100" class="slider">
                </div>
                
                <div class="status">
                    <p>X: <span id="x-value">0</span> | Y: <span id="y-value">0</span></p>
                </div>
            </div>
        </div>
    </div>
    
    <script src="/script.js"></script>
</body>
</html>
)=====";
}

// ============================================
// CSS de la page web
// ============================================
String getCSS() {
  return R"=====(
body {
    font-family: Arial, sans-serif;
    margin: 0;
    padding: 20px;
    background-color: #f0f0f0;
    text-align: center;
    user-select: none;
}

.container {
    max-width: 800px;
    margin: 0 auto;
    background: white;
    padding: 20px;
    border-radius: 10px;
    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
}

h1 {
    color: #333;
    margin-bottom: 10px;
}

.layout {
    display: flex;
    flex-direction: row;
    gap: 20px;
    margin-top: 20px;
    justify-content: center;
    align-items: flex-start;
}

.video-container {
    flex: 1;
    max-width: 320px;
}

.video-container img {
    width: 100%;
    border-radius: 8px;
    border: 2px solid #333;
    background: #000;
}

.controls-container {
    flex: 1;
    display: flex;
    flex-direction: column;
    align-items: center;
    max-width: 300px;
}

.joystick-container {
    margin: 10px 0;
    position: relative;
    width: 200px;
    height: 200px;
}

.joystick {
    width: 100%;
    height: 100%;
    background: #ddd;
    border-radius: 50%;
    position: relative;
    touch-action: none;
}

.stick {
    width: 50px;
    height: 50px;
    background: #ff4444;
    border-radius: 50%;
    position: absolute;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    cursor: pointer;
}

.slider-container {
    margin: 20px 0;
    width: 100%;
}

.slider {
    width: 100%;
    height: 20px;
    border-radius: 10px;
    background: #ddd;
    outline: none;
    -webkit-appearance: none;
}

.slider::-webkit-slider-thumb {
    -webkit-appearance: none;
    appearance: none;
    width: 30px;
    height: 30px;
    border-radius: 50%;
    background: #ff4444;
    cursor: pointer;
}

.slider::-moz-range-thumb {
    width: 30px;
    height: 30px;
    border-radius: 50%;
    background: #ff4444;
    cursor: pointer;
    border: none;
}

.status {
    margin-top: 10px;
    font-size: 14px;
    color: #666;
}

label {
    display: block;
    margin-bottom: 10px;
    font-weight: bold;
}

@media (max-width: 700px) {
    .layout {
        flex-direction: column;
        align-items: center;
    }
    
    .video-container {
        max-width: 100%;
    }
}
)=====";
}

// ============================================
// JavaScript de la page web
// ============================================
String getJS() {
  return R"=====(
// Adresse IP du serveur
const ip = window.location.hostname;
document.getElementById('ip').textContent = ip;

// Variables pour le joystick
let joystick = document.getElementById('joystick');
let stick = document.getElementById('stick');
let isDragging = false;
let joystickCenterX = joystick.offsetWidth / 2;
let joystickCenterY = joystick.offsetHeight / 2;
let stickRadius = stick.offsetWidth / 2;
let joystickRadius = joystick.offsetWidth / 2 - stickRadius;

// Variables pour les valeurs
let currentX = 0;
let currentY = 0;
let currentSpeed = 100;

// Connexion WebSocket
let socket;

// Initialisation
function init() {
    // Connexion WebSocket
    socket = new WebSocket('ws://' + ip + ':81/');
    socket.onopen = function() {
        console.log('Connecté au WebSocket');
    };
    socket.onclose = function() {
        console.log('Déconnecté du WebSocket');
        setTimeout(init, 1000); // Reconnexion après 1 seconde
    };
    socket.onerror = function(error) {
        console.error('Erreur WebSocket:', error);
    };
    
    // Événements du joystick
    stick.addEventListener('mousedown', startDrag);
    stick.addEventListener('touchstart', startDrag, { passive: false });
    
    // Événements pour le slider
    let speedSlider = document.getElementById('speed');
    speedSlider.addEventListener('input', function() {
        currentSpeed = this.value;
        document.getElementById('speed-value').textContent = currentSpeed + '%';
        sendData();
    });
    
    // Mise à jour initiale
    sendData();
}

// Début du glisser-déposer
function startDrag(e) {
    isDragging = true;
    e.preventDefault();
    
    // Position initiale
    let rect = joystick.getBoundingClientRect();
    let clientX = e.clientX || e.touches[0].clientX;
    let clientY = e.clientY || e.touches[0].clientY;
    let centerX = rect.left + joystickCenterX;
    let centerY = rect.top + joystickCenterY;
    
    // Calcul des coordonnées relatives
    let x = clientX - centerX;
    let y = clientY - centerY;
    
    // Limite au cercle du joystick
    let distance = Math.sqrt(x * x + y * y);
    if (distance > joystickRadius) {
        x = (x / distance) * joystickRadius;
        y = (y / distance) * joystickRadius;
    }
    
    // Mise à jour de la position
    currentX = Math.round((x / joystickRadius) * 100);
    currentY = Math.round((y / joystickRadius) * 100);
    
    // Positionnement du stick
    stick.style.left = (centerX + x - stickRadius) + 'px';
    stick.style.top = (centerY + y - stickRadius) + 'px';
    
    // Affichage des valeurs
    document.getElementById('x-value').textContent = currentX;
    document.getElementById('y-value').textContent = currentY;
    
    // Envoi des données
    sendData();
    
    // Événements de déplacement
    document.addEventListener('mousemove', drag);
    document.addEventListener('touchmove', drag, { passive: false });
    document.addEventListener('mouseup', stopDrag);
    document.addEventListener('touchend', stopDrag);
}

// Glisser-déposer
function drag(e) {
    if (!isDragging) return;
    e.preventDefault();
    
    let rect = joystick.getBoundingClientRect();
    let clientX = e.clientX || e.touches[0].clientX;
    let clientY = e.clientY || e.touches[0].clientY;
    let centerX = rect.left + joystickCenterX;
    let centerY = rect.top + joystickCenterY;
    
    // Calcul des coordonnées relatives
    let x = clientX - centerX;
    let y = clientY - centerY;
    
    // Limite au cercle du joystick
    let distance = Math.sqrt(x * x + y * y);
    if (distance > joystickRadius) {
        x = (x / distance) * joystickRadius;
        y = (y / distance) * joystickRadius;
    }
    
    // Mise à jour de la position
    currentX = Math.round((x / joystickRadius) * 100);
    currentY = Math.round((y / joystickRadius) * 100);
    
    // Positionnement du stick
    stick.style.left = (centerX + x - stickRadius) + 'px';
    stick.style.top = (centerY + y - stickRadius) + 'px';
    
    // Affichage des valeurs
    document.getElementById('x-value').textContent = currentX;
    document.getElementById('y-value').textContent = currentY;
    
    // Envoi des données
    sendData();
}

// Fin du glisser-déposer
function stopDrag() {
    isDragging = false;
    
    // Retour au centre
    currentX = 0;
    currentY = 0;
    
    stick.style.left = '50%';
    stick.style.top = '50%';
    stick.style.transform = 'translate(-50%, -50%)';
    
    document.getElementById('x-value').textContent = '0';
    document.getElementById('y-value').textContent = '0';
    
    // Envoi des données
    sendData();
    
    // Suppression des écouteurs
    document.removeEventListener('mousemove', drag);
    document.removeEventListener('touchmove', drag);
    document.removeEventListener('mouseup', stopDrag);
    document.removeEventListener('touchend', stopDrag);
}

// Envoi des données au serveur
function sendData() {
    if (socket && socket.readyState === WebSocket.OPEN) {
        socket.send('X:' + currentX + ',Y:' + currentY + ',S:' + currentSpeed);
    }
}

// Initialisation au chargement
window.onload = init;
)=====";
}
