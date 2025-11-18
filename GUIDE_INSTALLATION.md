# Guide d'Installation Complet - Système de Reconnaissance Faciale ESP32-CAM

## Table des Matières
1. [Prérequis](#prérequis)
2. [Installation du Logiciel](#installation-du-logiciel)
3. [Connexion du Matériel](#connexion-du-matériel)
4. [Téléversement du Code](#téléversement-du-code)
5. [Configuration Initiale](#configuration-initiale)
6. [Test du Système](#test-du-système)
7. [Dépannage](#dépannage)

---

## Prérequis

### Matériel Nécessaire

| Composant | Quantité | Notes |
|-----------|----------|-------|
| ESP32-CAM (AI-Thinker) | 1 | Avec caméra OV2640 |
| Programmateur FTDI ou ESP32-CAM-MB | 1 | Pour téléverser le code |
| Module relais 5V | 1 | 1 canal suffisant |
| Alimentation 5V/2A | 1 | Minimum 2A requis |
| Câbles Dupont | Plusieurs | Mâle-Femelle et Mâle-Mâle |
| Breadboard | 1 | Optionnel mais pratique |

### Logiciel Nécessaire

- **Arduino IDE** 1.8.19 ou supérieur
- **Pilote USB** (CP2102 ou CH340 selon votre programmateur)
- **Câble USB** type A vers Mini/Micro USB

---

## Installation du Logiciel

### Étape 1: Installer Arduino IDE

1. Télécharger depuis: https://www.arduino.cc/en/software
2. Installer l'application
3. Lancer Arduino IDE

### Étape 2: Ajouter le Support ESP32

1. Dans Arduino IDE: **Fichier → Préférences**
2. Dans "URL de gestionnaire de cartes additionnelles", ajouter:
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
3. Cliquer **OK**
4. Aller dans **Outils → Type de carte → Gestionnaire de carte**
5. Rechercher **"ESP32"**
6. Installer **"esp32 by Espressif Systems"** (version 2.0.x ou supérieure)
7. Attendre la fin de l'installation (peut prendre plusieurs minutes)

### Étape 3: Installer les Pilotes USB

#### Pour Windows:
- **CP2102**: Télécharger depuis Silicon Labs
- **CH340**: Télécharger depuis le site du fabricant

#### Pour Linux:
```bash
# Les pilotes sont généralement préinstallés
# Si nécessaire, installer avec:
sudo apt-get install python3-serial
```

#### Pour macOS:
- Télécharger les pilotes depuis le site du fabricant
- Autoriser l'installation dans "Préférences Système → Sécurité"

---

## Connexion du Matériel

### Schéma 1: ESP32-CAM vers Relais

```
ESP32-CAM          Module Relais
─────────          ─────────────
5V        ──→      VCC
GND       ──→      GND
GPIO12    ──→      IN
```

### Schéma 2: ESP32-CAM vers Programmateur FTDI

```
FTDI              ESP32-CAM
────              ─────────
5V     ──→        5V
GND    ──→        GND
TX     ──→        U0R (RX)
RX     ──→        U0T (TX)
```

**Important pour la programmation:**
- Connecter **GPIO0 à GND** avant de téléverser
- Appuyer sur le bouton **RESET**
- Déconnecter GPIO0 après téléversement

### Schéma 3: Connexion de la Charge au Relais

**Exemple avec serrure électrique 12V:**

```
Alimentation 12V (+) ──→ Relais COM
Relais NO ──→ Serrure électrique (+)
Serrure électrique (-) ──→ GND Alimentation 12V
```

---

## Téléversement du Code

### Étape 1: Ouvrir le Projet

1. Télécharger le fichier `esp32_face_recognition.ino`
2. Ouvrir avec Arduino IDE
3. Le code devrait s'afficher

### Étape 2: Configurer la Carte

1. **Outils → Type de carte → ESP32 Arduino → AI Thinker ESP32-CAM**
2. Configurer les paramètres suivants:

| Paramètre | Valeur |
|-----------|--------|
| Upload Speed | 115200 |
| Flash Frequency | 80MHz |
| Flash Mode | QIO |
| Partition Scheme | **Huge APP (3MB No OTA/1MB SPIFFS)** |
| Core Debug Level | None |
| Port | *Sélectionner votre port COM* |

### Étape 3: Téléverser

1. Connecter GPIO0 à GND sur l'ESP32-CAM
2. Connecter le programmateur FTDI à l'ordinateur
3. Appuyer sur le bouton **RESET** de l'ESP32-CAM
4. Dans Arduino IDE, cliquer sur **Téléverser** (→)
5. Attendre le message: "Hard resetting via RTS pin..."
6. **Déconnecter GPIO0 de GND**
7. Appuyer à nouveau sur **RESET**

### Vérification du Téléversement

Si tout fonctionne:
```
[✓] Écriture à 0x00001000...
[✓] Écriture à 0x00008000...
[✓] Hash de données vérifié.
[✓] Téléversement terminé
```

En cas d'erreur, voir section [Dépannage](#dépannage).

---

## Configuration Initiale

### Étape 1: Ouvrir le Moniteur Série

1. Dans Arduino IDE: **Outils → Moniteur série**
2. Régler la vitesse: **115200 baud**
3. Appuyer sur **RESET** sur l'ESP32-CAM

Vous devriez voir:
```
=================================
ESP32-CAM Face Recognition System
=================================

Caméra initialisée avec succès!

=== MODE D'ENRÔLEMENT ===
Envoyez 'e' pour enrôler un nouveau visage
Envoyez 'd' pour supprimer tous les visages
Envoyez 'r' pour démarrer la reconnaissance
========================
```

### Étape 2: Enrôler des Visages

1. Positionner votre visage devant la caméra (distance: 50-70cm)
2. Envoyer **'e'** dans le moniteur série
3. Attendre les messages:
   ```
   Échantillon 1/5 capturé
   Échantillon 2/5 capturé
   ...
   ✓ Visage enrôlé avec succès!
   ```
4. Répéter pour chaque personne autorisée (max 7 visages)

**Conseils pour un bon enrôlement:**
- Éclairage uniforme et suffisant
- Visage de face (ne pas tourner la tête)
- Rester immobile pendant la capture
- Éviter les ombres sur le visage

### Étape 3: Activer la Reconnaissance

1. Envoyer **'r'** dans le moniteur série
2. Le système démarre:
   ```
   >>> Mode reconnaissance activé <<<
   ```
3. Le système fonctionne maintenant en continu

---

## Test du Système

### Test 1: Visage Reconnu

1. Se placer devant la caméra
2. Observer le moniteur série:
   ```
   Visage détecté!
   ✓ VISAGE RECONNU! ID: 0, Confiance: 0.85
   >>> ACTIVATION DU RELAIS <<<
   ```
3. Le relais devrait s'activer pendant 5 secondes
4. La LED flash s'allume (si activée)

### Test 2: Visage Non Reconnu

1. Demander à une personne non enrôlée de se présenter
2. Observer le moniteur série:
   ```
   Visage détecté!
   ✗ VISAGE NON RECONNU - Relais inactif
   ```
3. Le relais reste inactif

### Test 3: Aucun Visage

1. Retirer toute personne du champ de vision
2. Le système ne devrait rien afficher
3. Le relais reste inactif

---

## Dépannage

### Problème: La caméra ne s'initialise pas

**Erreur:**
```
Erreur d'initialisation de la caméra: 0x105
```

**Solutions:**
1. Vérifier l'alimentation (minimum 2A)
2. Vérifier les connexions de la caméra au module
3. Essayer une autre alimentation
4. Vérifier que la nappe de la caméra est bien enfoncée

### Problème: Impossible de téléverser le code

**Erreur:**
```
A fatal error occurred: Failed to connect to ESP32
```

**Solutions:**
1. Vérifier que GPIO0 est bien connecté à GND
2. Appuyer sur RESET avant de téléverser
3. Vérifier les connexions TX/RX (inverser si nécessaire)
4. Essayer un autre câble USB
5. Vérifier que le bon port COM est sélectionné
6. Réduire la vitesse de téléversement à 921600 ou 460800

### Problème: Aucun visage détecté

**Symptôme:**
```
Aucun visage détecté. Réessayez...
```

**Solutions:**
1. Améliorer l'éclairage (lumière naturelle ou LED blanc)
2. Se rapprocher de la caméra (50-70cm optimal)
3. Positionner le visage bien de face
4. Nettoyer la lentille de la caméra
5. Éviter le contre-jour

### Problème: Reconnaissance imprécise

**Symptôme:**
- Faux positifs (visage non autorisé reconnu)
- Faux négatifs (visage autorisé non reconnu)

**Solutions:**

**Pour réduire les faux positifs:**
```cpp
// Augmenter le seuil dans le code
#define FACE_RECOGNITION_THRESHOLD 0.8  // Au lieu de 0.7
```

**Pour réduire les faux négatifs:**
```cpp
// Diminuer le seuil
#define FACE_RECOGNITION_THRESHOLD 0.6  // Au lieu de 0.7
```

**Améliorer la qualité d'enrôlement:**
1. Supprimer tous les visages: envoyer **'d'**
2. Réenrôler avec un meilleur éclairage
3. S'assurer d'être immobile pendant l'enrôlement

### Problème: Le relais ne s'active pas

**Symptôme:**
- Message "ACTIVATION DU RELAIS" affiché
- Mais le relais ne clique pas

**Solutions:**
1. Vérifier la connexion GPIO12 → IN du relais
2. Mesurer la tension sur GPIO12 quand activé (devrait être 3.3V)
3. Vérifier l'alimentation du module relais (5V)
4. Tester le relais avec une source 3.3V externe
5. Vérifier que le module relais supporte un signal 3.3V (certains nécessitent 5V)

**Si le module nécessite 5V:**
```cpp
// Utiliser un transistor ou un level shifter
// Ou changer pour un module relais compatible 3.3V
```

### Problème: Redémarrages intempestifs

**Symptôme:**
```
Brownout detector was triggered
ets Jun  8 2016 00:22:57
```

**Solutions:**
1. Utiliser une alimentation plus puissante (2.5A ou 3A)
2. Ajouter un condensateur 1000µF sur l'alimentation 5V
3. Utiliser des câbles courts et de bonne qualité
4. Vérifier que le relais n'est pas sur la même alimentation

### Problème: Erreur de mémoire

**Erreur:**
```
Guru Meditation Error: Core 1 panic'ed (LoadProhibited)
```

**Solutions:**
1. Vérifier que "Huge APP" est sélectionné dans Partition Scheme
2. Réduire le nombre de visages enregistrés
3. Redémarrer l'ESP32-CAM
4. Retéléverser le code

---

## Optimisations Avancées

### Améliorer la Vitesse de Reconnaissance

```cpp
// Modifier la fréquence de capture
void loop() {
  recognizeFace();
  delay(50);  // Au lieu de 100
}
```

### Ajouter un Timeout de Sécurité

```cpp
// Désactiver automatiquement après X secondes sans détection
unsigned long lastDetection = 0;
#define SECURITY_TIMEOUT 30000  // 30 secondes

void loop() {
  if (millis() - lastDetection > SECURITY_TIMEOUT) {
    digitalWrite(RELAY_PIN, LOW);  // Sécurité
  }
  recognizeFace();
}
```

### Sauvegarder les Visages dans la Flash

```cpp
// Ajouter cette fonction pour sauvegarder dans SPIFFS
#include "SPIFFS.h"

void saveFacesToFlash() {
  File file = SPIFFS.open("/faces.dat", FILE_WRITE);
  file.write((uint8_t*)&id_list, sizeof(id_list));
  file.close();
}

void loadFacesFromFlash() {
  if (SPIFFS.exists("/faces.dat")) {
    File file = SPIFFS.open("/faces.dat", FILE_READ);
    file.read((uint8_t*)&id_list, sizeof(id_list));
    file.close();
  }
}
```

---

## Conseils de Sécurité

⚠️ **Important:**

1. **Ne jamais dépasser la capacité du relais** (vérifier A/V max)
2. **Isoler les circuits haute tension** des circuits logiques
3. **Ajouter une protection fusible** sur les charges importantes
4. **Tester d'abord avec une LED** avant de connecter une charge réelle
5. **Ne pas utiliser pour des applications critiques de sécurité** sans backup

---

## Support et Ressources

### Documentation Officielle
- ESP32: https://docs.espressif.com/projects/esp-idf/
- Arduino ESP32: https://github.com/espressif/arduino-esp32

### Forums Utiles
- ESP32 Forum: https://www.esp32.com/
- Arduino Forum: https://forum.arduino.cc/

### Vidéos Tutoriels
- Rechercher "ESP32-CAM face recognition" sur YouTube
- Tutoriels de programmation ESP32-CAM

---

**Version du Guide:** 1.0
**Dernière mise à jour:** 2025
**Compatibilité:** ESP32-CAM AI-Thinker avec Arduino IDE
