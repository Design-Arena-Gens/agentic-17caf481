/**
 * ESP32-CAM Face Recognition with Relay Control
 * Système de reconnaissance faciale hors ligne avec activation de relais
 */

#include "esp_camera.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "fr_flash.h"

// Configuration des pins pour AI-Thinker ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Pin du relais
#define RELAY_PIN         12

// LED Flash (optionnel)
#define FLASH_LED_PIN      4

// Variables globales pour la reconnaissance faciale
mtmn_config_t mtmn_config = {0};
face_id_list id_list = {0};

// Seuil de confiance pour la reconnaissance (0.0 - 1.0)
#define FACE_RECOGNITION_THRESHOLD 0.7

// Durée d'activation du relais (en millisecondes)
#define RELAY_ACTIVE_TIME 5000

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=================================");
  Serial.println("ESP32-CAM Face Recognition System");
  Serial.println("=================================\n");

  // Configuration du relais
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Relais désactivé par défaut

  // Configuration du flash LED (optionnel)
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  // Configuration de la caméra
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Configuration de la résolution
  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA; // 320x240 pour la reconnaissance faciale
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Initialisation de la caméra
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Erreur d'initialisation de la caméra: 0x%x\n", err);
    ESP.restart();
  }

  Serial.println("Caméra initialisée avec succès!");

  // Configuration de la détection de visage
  mtmn_config = mtmn_init_config();

  // Initialisation de la base de données de visages
  face_id_init(&id_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);

  Serial.println("\n=== MODE D'ENRÔLEMENT ===");
  Serial.println("Envoyez 'e' pour enrôler un nouveau visage");
  Serial.println("Envoyez 'd' pour supprimer tous les visages");
  Serial.println("Envoyez 'r' pour démarrer la reconnaissance");
  Serial.println("========================\n");
}

void loop() {
  // Vérifier les commandes série
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    if (cmd == 'e' || cmd == 'E') {
      enrollFace();
    } else if (cmd == 'd' || cmd == 'D') {
      deleteFaces();
    } else if (cmd == 'r' || cmd == 'R') {
      Serial.println("\n>>> Mode reconnaissance activé <<<\n");
    }
  }

  // Reconnaissance faciale en continu
  recognizeFace();

  delay(100); // Petit délai pour éviter la surcharge
}

void enrollFace() {
  Serial.println("\n=== ENRÔLEMENT D'UN NOUVEAU VISAGE ===");
  Serial.println("Positionnez le visage devant la caméra...");

  int enrollCount = 0;
  face_id_name_t face_id_name;

  while (enrollCount < ENROLL_CONFIRM_TIMES) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Erreur: Impossible de capturer l'image");
      continue;
    }

    // Détection de visage
    dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
    fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item);

    box_array_t *net_boxes = face_detect(image_matrix, &mtmn_config);

    if (net_boxes && net_boxes->len > 0) {
      // Visage détecté
      dl_matrix3du_t *aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);
      if (align_face(net_boxes, image_matrix, aligned_face) == ESP_OK) {

        if (enrollCount == 0) {
          // Premier enrôlement - créer un nouvel ID
          int8_t left_sample_face = enroll_face(&id_list, aligned_face);
          if (left_sample_face >= 0) {
            enrollCount = ENROLL_CONFIRM_TIMES - left_sample_face;
            Serial.printf("Échantillon %d/%d capturé\n", enrollCount, ENROLL_CONFIRM_TIMES);
          }
        } else {
          // Échantillons supplémentaires
          int8_t left_sample_face = enroll_face(&id_list, aligned_face);
          if (left_sample_face >= 0) {
            enrollCount = ENROLL_CONFIRM_TIMES - left_sample_face;
            Serial.printf("Échantillon %d/%d capturé\n", enrollCount, ENROLL_CONFIRM_TIMES);
          }
        }
      }
      dl_matrix3du_free(aligned_face);
    } else {
      Serial.println("Aucun visage détecté. Réessayez...");
    }

    dl_matrix3du_free(image_matrix);
    esp_camera_fb_return(fb);
    free(net_boxes->box);
    free(net_boxes->score);
    free(net_boxes);

    delay(500);
  }

  if (enrollCount >= ENROLL_CONFIRM_TIMES) {
    Serial.println("\n✓ Visage enrôlé avec succès!");
    Serial.printf("Total de visages enregistrés: %d\n\n", id_list.count);
  }
}

void deleteFaces() {
  face_id_init(&id_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
  Serial.println("\n✓ Tous les visages ont été supprimés\n");
}

void recognizeFace() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Erreur: Impossible de capturer l'image");
    return;
  }

  // Conversion de l'image
  dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
  fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item);

  // Détection de visage
  box_array_t *net_boxes = face_detect(image_matrix, &mtmn_config);

  if (net_boxes && net_boxes->len > 0) {
    Serial.println("Visage détecté!");

    // Alignement du visage
    dl_matrix3du_t *aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);
    if (align_face(net_boxes, image_matrix, aligned_face) == ESP_OK) {

      // Reconnaissance du visage
      if (id_list.count > 0) {
        face_id_name_t face_recognized = recognize_face(&id_list, aligned_face);

        if (face_recognized.id >= 0) {
          // Visage reconnu!
          Serial.printf("✓ VISAGE RECONNU! ID: %d, Confiance: %.2f\n",
                       face_recognized.id, face_recognized.similarity);

          if (face_recognized.similarity >= FACE_RECOGNITION_THRESHOLD) {
            activateRelay();
          } else {
            Serial.println("Confiance insuffisante. Relais non activé.");
          }
        } else {
          // Visage non reconnu
          Serial.println("✗ VISAGE NON RECONNU - Relais inactif");
          digitalWrite(RELAY_PIN, LOW);
        }
      } else {
        Serial.println("Aucun visage enregistré dans la base de données");
      }
    }
    dl_matrix3du_free(aligned_face);
  }

  dl_matrix3du_free(image_matrix);
  esp_camera_fb_return(fb);

  if (net_boxes) {
    free(net_boxes->box);
    free(net_boxes->score);
    free(net_boxes);
  }
}

void activateRelay() {
  Serial.println("\n>>> ACTIVATION DU RELAIS <<<");
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(FLASH_LED_PIN, HIGH); // LED indicateur

  delay(RELAY_ACTIVE_TIME);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(FLASH_LED_PIN, LOW);
  Serial.println(">>> RELAIS DÉSACTIVÉ <<<\n");
}
