# FPV Wi-Fi Remote Controlled Car
## description
(fr) Voici les codes pour créé une voiture télécommandé "FPV" (accès a la caméra sur le serveur le la carte électronique) contrôlé par wifi (vous pouvez  modifier les identifiant wifi a tout moment dans le fichier "car_control_tb6612.ino" (ligne 15 - 16)(https://github.com/Sp8ceranger/FPV-Wi-Fi-Remote-Controlled-Car/blob/main/car_control_tb6612/car_control_tb6612.ino))

* matériel : 
  1. XIAO ESP32 S3 Sense (avec la caméra) x1,
  2. moteur x4,
  3. module moteur TB6612FNG x1
##
(en) Here are the codes for creating an “FPV” remote-controlled car (camera access via the server on the circuit board) controlled via Wi-Fi (you can change the Wi-Fi credentials at any time in the “car_control_tb6612.ino” file (lines 15–16)) (https://github.com/Sp8ceranger/FPV-Wi-Fi-Remote-Controlled-Car/blob/main/car_control_tb6612/car_control_tb6612.ino))

* Hardware: 
  1. XIAO ESP32 S3 Sense (with camera) x1, 
  2. motors x4
  3. TB6612FNG motor driver module x1

Translated with DeepL.com (free version)

## câblage / wiring
 | Broche TB6612FNG | Broche ESP32 | Description          |
 |------------------|--------------|----------------------|
 | IN1              | GPIO 1       | Contrôle moteur 1    |
 | IN2              | GPIO 2       | Contrôle moteur 1    |
 | IN3              | GPIO 3       | Contrôle moteur 2    |
 | IN4              | GPIO 4       | Contrôle moteur 2    |
 | PWM1             | GPIO 5       | Vitesse moteur 1     |
 | PWM2             | GPIO 6       | Vitesse moteur 2     |
 | STBY             | GPIO 7       | Mode veille          |
 | VM               | 5V           | Alimentation moteurs |
 | VCC              | 3.3V         | Alimentation logique |
 | GND              | GND          | Masse commune        |
