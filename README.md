# Extension Canton Controller (EXCC) - Chemin de Fer du Chablais (CFC)

[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32-blue)](https://platformio.org/)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

**Système de gestion de canton ferroviaire pour modélisme numérique DCC**

## 📋 Table des Matières

- [🚂 Présentation](#-présentation)
- [🎯 Fonctionnalités](#-fonctionnalités)
- [🏗️ Architecture](#-architecture)
- [📦 Structure du Projet](#-structure-du-projet)
- [🔌 Protocole CAN](#-protocole-can)
- [📡 Communication](#-communication)
- [⚙️ Configuration](#-configuration)
- [🔧 Matériel](#-matériel)
- [💡 Utilisation](#-utilisation)
- [📊 Signaux SNCF](#-signaux-sncf)
- [🔄 Adaptation au DCC_CAN_RMT-Booster](#-adaptation-au-dcc_can_rmt-booster)
- [📝 Changelog](#-changelog)
- [🤝 Contribution](#-contribution)
- [📄 Licence](#-licence)

---

## 🚂 Présentation

**EXCC (Extension Canton Controller)** est un firmware embarqué pour ESP32 conçu pour la gestion avancée de cantons ferroviaires en modélisme numérique DCC. Il fait partie de l'écosystème **Chemin de Fer du Chablais (CFC)** et fonctionne en synergie avec d'autres modules comme le **CC (Canton Controller)** et le **DCC_CAN_RMT-Booster**.

Ce système permet de :
- Gérer l'occupation des cantons via détection de courant
- Contrôler les signaux SNCF (BAL, Carré, Ralentissement, Rappel, Manœuvre)
- Piloter des aiguillages avec servos
- Détecter les trains via comptage d'essieux
- Communiquer via bus CAN avec d'autres modules
- Gérer le RailCom pour l'identification des locomotives

---

## 🎯 Fonctionnalités

### ✅ Gestion des Cantons
- Détection d'occupation par mesure de courant (via DRV8874)
- Gestion des seuils configurables (libre/occupé)
- Animation LED pour visualisation du statut
- Comptage d'essieux pour validation des trains

### ✅ Signaux Ferroviaires
- **5 types de signaux SNCF** :
  - Type A (BAL) : 3 feux
  - Type C (Carré + BAL) : 5 feux
  - Type E (Ralentissement) : 7 feux
  - Type G (Rappel) : 9 feux
  - Type M (Manœuvre) : 2 feux
- **11 aspects SNCF** : Carré, Carré Violet, Sémaphore, Avertissement, Ralentissement 30/60, Rappel 30/60, Voie Libre, Manœuvre, Masqué
- Affichage via LED WS2812/WS2815
- Clignotement configurables

### ✅ Aiguillages
- Contrôle de 6 servos (configurable via `AIG_COUNT`)
- Gestion des positions (Droit/Dévie/Indéterminé/Incohérent)
- Feedback via micro-switches
- Vitesse et positions configurables

### ✅ Booster DCC
- Génération du signal DCC
- Mesure de courant et tension en temps réel
- Protection contre les courts-circuits
- Détection thermique
- Gestion du cutout RailCom
- Décodage RailCom HF

### ✅ Communication CAN
- Protocole CAN 2.0B (29 bits)
- Communication bidirectionnelle avec CC, ERM, CLF
- Gestion des priorités
- Filtres de messages

### ✅ Télémétrie
- Mesure de courant via IPROPI (DRV8874)
- Mesure de tension via pont diviseur
- Échantillonnage RailCom
- LED de statut (CAN, RailCom, Booster)

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                         EXCC (ESP32)                                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌───────────────┐    ┌──────────────┐    ┌──────────────────────┐  │
│  │   main.cpp    │    │ EXCC_Main.h  │    │  EXCC_Main.cpp       │  │
│  │  (Entrée)     │    │  (Orchestre) │    │  (Logique principale)│  │
│  └──────┬────────┘    └──────┬───────┘    └──────────┬───────────┘  │
│         │                    │                         │            │
│         └────────────────────┼─────────────────────────┘            │
│                          │                                          │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │                    Tâche FreeRTOS (1 kHz)                     │  │
│  │  - esp_task_wdt_reset()                                       │  │
│  │  - EXCC_Main::loop()                                          │  │
│  │  - Watchdog matériel                                          │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐      │
│  │  EXCC_Signaux   │  │  EXCC_Occupation│  │ EXCC_Servo      │      │
│  │  (WS2812)       │  │  (Comptage)     │  │  (Aiguillages)  │      │
│  └────────┬────────┘  └────────┬────────┘  └────────┬────────┘      │
│           │                   │                     │               │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐      │
│  │  EXCC_Canton    │  │ EXCC_Compteur   │  │ EXCC_Switches   │      │
│  │  (LED)          │  │ (Essieux)       │  │ (Feedback)      │      │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘      │
│                                                                     │
│  ┌─────────────────┐  ┌──────────────────────────────────────────┐  │
│  │ EXCC_Booster    │  │         EXCC_CanBooster                  │  │
│  │ (WS2812)        │  │  - process() canMessages                 │  │
│  │ - DCC Generation│  │  - handleFrame()                         │  │
│  │ - RailCom       │  │  - Cutout management                     │  │
│  │ - Telemetry     │  └──────────────────────────────────────────┘  │
│  └────────┬────────┘                                                │
│           │                                                         │
│  ┌─────────────────┐    ┌─────────────────┐                         │
│  │EXCC_BoosterCore │    │ EXCC_Callbacks  │                         │
│  │ (Tâche FreeRTOS)│    │ (CC → EXCC)     │                         │
│  └─────────────────┘    └─────────────────┘                         │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │                    Bus CAN (ACAN_ESP32 / ACAN2515)            │  │
│  │  - CanBus::bus(1).send(msg)                                   │  │
│  │  - EXCC_CAN_CC::sendPong(), envoyerOccupation(), etc.         │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘

                 ┌─────────────────┐
                 │   DIP Switch    │
                 │   Configuration │
                 └────────┬────────┘
                          │
        ┌─────────────────────┼─────────────────────┐
        │                     │                     │
┌───────▼───────┐    ┌───────▼───────┐    ┌───────▼───────┐
│  PCA9685      │    │  DRV8874      │    │  MCP23017     │
│  (Servos)     │    │  (Booster)    │    │  (I/O)        │
└───────────────┘    └───────────────┘    └───────────────┘
        │                     │                     │
┌───────▼───────┐    ┌───────▼───────┐    ┌───────▼───────┐
│  WS2812/15    │    │  Capteurs     │    │  Switches     │
│  (Signaux)    │    │  (Quadrature) │    │  (Aiguillages)│
└───────────────┘    └───────────────┘    └───────────────┘
```

### Diagramme de Flux de Données

```
┌─────────────┐        ┌─────────────┐        ┌─────────────┐
│   CC        │        │   ERM       │        │   CLF       │
│ (Controller)│        │ (Maître)    │        │ (Logique)   │
└──────┬──────┘        └──────┬──────┘        └──────┬──────┘
       │                      │                      │
       │ CAN (Cmd_CC_to_EXCC) │                      │
       ▼                      ▼                      ▼
┌───────────────────────────────────────────────────────────────┐
│                        EXCC                                   │
│  ┌─────────────┐  ┌─────────────┐  ┌──────────────────────┐   │
│  │  Callbacks  │  │   CAN RX    │  │   Traitement CAN     │   │
│  │  (Réception)│  │ (Messages)  │  │   (EXCC_CAN_CC)      │   │
│  └──────┬──────┘  └──────┬──────┘  └──────────┬───────────┘   │
│         │                │                      │             │
│         ▼                ▼                      ▼             │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │                    EXCC_Main::loop()                    │  │
│  │  - Mise à jour des signaux                              │  │
│  │  - Gestion occupation                                   │  │
│  │  - Contrôle servos                                      │  │
│  │  - Télémétrie booster                                   │  │
│  └─────────────────────────────────────────────────────────┘  │
│                                                               │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │                    EXCC_CAN_CC::send*()                 │  │
│  │  - PONG, BOOSTER_INFO, OCCUPATION, etc.                 │  │
│  └─────────────────────────────────────────────────────────┘  │
└───────────────────────────────────────────────────────────────┘
       │                    │                   │
       ▼                    ▼                   ▼
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   CC        │     │   ERM       │     │   CLF       │
│ (Controller)│     │ (Maître)    │     │ (Logique)   │
└─────────────┘     └─────────────┘     └─────────────┘
```

---

## 📦 Structure du Projet

```
Extension_CantonController/
├── .gitignore                    # Fichiers à ignorer
├── .vscode/                      # Configuration VS Code
├── include/                      # En-têtes partagés (vide actuellement)
├── lib/                          # Bibliothèques locales (vide actuellement)
├── platformio.ini                # Configuration PlatformIO
├── src/                          # Code source principal
│   ├── Protocol.h                # ✅ Protocole CAN (opcodes, enums)
│   ├── main.cpp                  # Point d'entrée Arduino/FreeRTOS
│   ├── EXCC_Main.h/cpp           # Orchestration principale
│   ├── EXCC_Config.h             # Configuration globale
│   ├── EXCC_Pins.h               # Définition des broches
│   ├── 
│   ├── EXCC_BoosterCore.h/cpp    # Tâche FreeRTOS du booster
│   ├── EXCC_BoosterHardware.h/cpp # Interface matériel booster
│   ├── EXCC_Booster_WS2812.h/cpp  # Booster + télémétrie + RailCom
│   ├── EXCC_CanBooster.h/cpp      # Communication CAN du booster
│   ├── EXCC_CAN_CC.h/cpp          # Envoi de messages CAN (EXCC → CC)
│   ├── 
│   ├── EXCC_Callbacks.h/cpp       # Gestion des commandes CC → EXCC
│   ├── EXCC_CanCommands.h/cpp      # Traitement des commandes CAN
│   ├── EXCC_CanConfig.h           # Configuration CAN
│   ├── 
│   ├── EXCC_Signaux_WS2812.h/cpp  # Gestion des signaux SNCF (WS2812)
│   ├── EXCC_Canton_WS2812.h/cpp   # Gestion LED canton
│   ├── EXCC_LedDirection_WS2812.h/cpp # LED directionnelles
│   ├── EXCC_Booster_WS2812.h/cpp  # LED booster
│   ├── 
│   ├── EXCC_Servo.h/cpp           # Contrôle des servos (aiguillages)
│   ├── EXCC_Switches.h/cpp        # Gestion des micro-switches
│   ├── EXCC_Quadrature.h/cpp      # Capteurs quadrature
│   ├── 
│   ├── EXCC_Occupation.h/cpp      # Gestion occupation canton
│   ├── EXCC_Compteur.h/cpp        # Compteur général
│   ├── EXCC_CompteurEssieux.h/cpp # Compteur d'essieux
│   ├── EXCC_Railcom.h/cpp         # Décodage RailCom
│   ├── 
│   ├── EXCC_Calibration.h/cpp     # Calibration des capteurs
│   ├── EXCC_Runtime.h/cpp         # Runtime et utilitaires
│   ├── EXCC_System.h/cpp          # Gestion système
│   └── EXCC_Tension_Booster.h/cpp # Mesure tension
└── test/                         # Tests unitaires
```

---

## 🔌 Protocole CAN

### 📡 Identifiants CAN (29 bits)

Le protocole utilise des identifiants CAN sur **29 bits** avec la structure suivante :

```
Bit 31-29: Priorité (0-7)
Bit 28-16: Commande (opcode)
Bit 15-8:  Réponse/Source
Bit 7-0:   ID Canton (0-255)
```

Format : `CanID::make29(priority, command, response, cantonId)`

### 📋 Enums de Commandes

#### Commandes EXCC → CC (0xD0-0xDF)

| Opcode | Nom | Description | Données |
|--------|-----|-------------|---------|
| 0xD1 | PONG | Réponse PING | - |
| 0xD2 | BOOSTER_INFO | Info booster | etat, courant, tension (3 bytes) |
| 0xD6 | POSITION_AIGUILLE | Position servo | servoIndex, position, etat (3 bytes) |
| 0xD7 | OCCUPATION | État occupation | actif (1 byte) |
| 0xD8 | PONCTUEL_H | Ponctuel horaire | actif (1 byte) |
| 0xD9 | PONCTUEL_AH | Ponctuel anti-horaire | actif (1 byte) |
| 0xDA | RAILCOM_ADRESSE | Adresse RailCom | type, adresse LSB, adresse MSB (3 bytes) |
| 0xDB | CALIB_BOOSTER_INFO | Seuils calibration | libre LSB, libre MSB, occupe LSB, occupe MSB (4 bytes) |
| 0xDC | ESSIEUX | Compte essieux | essieux (1 byte) |

#### Commandes CC → EXCC (0xF0-0xFF)

| Opcode | Nom | Description | Données |
|--------|-----|-------------|---------|
| 0xF0 | SERVO_MOVE | Déplacer servo | servoIndex, direction |
| 0xF1 | SERVO_CONFIG | Configurer servo | servoIndex, posDroit, posDevie, speed |
| 0xF2 | SERVO_TEST | Tester servo | servoIndex |
| 0xF3 | RECALIBRER_BOOSTER | Recalibrer booster | - |
| 0xF4 | SET_SEUILS | Définir seuils | libre, occupe |
| 0xF5 | BOOSTER_POWER | Alimentation booster | power (0-255) |
| 0xF6 | CONFIG_SIGNAUX | Configurer signaux | données |
| 0xF7 | ASPECT_HORAIRE | Aspect horaire | aspect (ExccAspect) |
| 0xF8 | ASPECT_ANTIHORAIRE | Aspect anti-horaire | aspect (ExccAspect) |
| 0xF9 | DIRECTION_HORAIRE | Direction horaire | code |
| 0xFA | DIRECTION_ANTIHORAIRE | Direction anti-horaire | code |
| 0xFB | OCCUPATION_VOISINS | Occupation voisins | value |
| 0xFC | PING | PING | - |
| 0xFD | CANTON_ID | ID canton | données |
| 0xFE | PROFILE_VOIE | Profil voie | profile (12V/15V) |

#### Commandes CC ↔ CC (Exploitation)

| Opcode | Nom | Description |
|--------|-----|-------------|
| 0xE0 | UPDATE_VOISINS | Mise à jour voisins |
| 0xE3 | RESERVATION_LOCO | Réservation loco |
| 0xE5 | RAILCOM_VOISIN | RailCom voisin |
| 0xE7 | ASPECT_VOISIN | Aspect voisin |
| 0xE9 | AIGUILLAGE | Commande aiguillage |
| 0xEB | MESURE_PREPARE | Préparer mesure |
| 0xEC | RECHERCHE_WAGON | Recherche wagon |

#### Commandes CC ↔ CC (Exploration)

| Opcode | Nom | Description |
|--------|-----|-------------|
| 0xC0 | DEMANDE_ID | Demande ID |
| 0xC1 | UPDATE_MASQUE_AIG | Mise à jour masque aiguillage |
| 0xC2 | ID_VOISIN | ID voisin |

#### Commandes ERM → CC

| Opcode | Nom | Description |
|--------|-----|-------------|
| 0xB3 | TEST_BUS_REPLY | Réponse test bus |
| 0xB5 | REQUEST_ID_REPLY | Réponse demande ID |
| 0xBD | WIFI_ON_OFF | Activer/désactiver WiFi |
| 0xBE | EXPLORATION_ON_OFF | Activer/désactiver exploration |
| 0xBF | SAVE_ALL | Sauvegarder tout |
| 0xBC | RESTART_ALL | Redémarrer tout |
| 0x20 | SET_PROFILE | Définir profil |
| 0xC3 | OFFLINE | Mode hors ligne |

#### Commandes CC → ERM

| Opcode | Nom | Description |
|--------|-----|-------------|
| 0xB2 | TEST_BUS | Tester bus |
| 0xB4 | REQUEST_ID | Demander ID |

#### Commandes CC → CLF

| Opcode | Nom | Description |
|--------|-----|-------------|
| 0xA0 | MESURE_VITESSE | Mesure vitesse |
| 0xA1 | ESSIEUX_TRAIN | Essieux train |

#### Commandes CLF → CC

| Opcode | Nom | Description |
|--------|-----|-------------|
| 0x90 | TRAIN_VALIDE | Train validé |
| 0x91 | TRAIN_REARMER | Réarmer train |
| 0x92 | DEMANDE_MESURE | Demande mesure |
| 0x93 | CLF_TIME | Synchronisation heure |
| 0x94 | TRAIN_COMPOSITION | Composition train |

#### Commandes Globales (11 bits)

| Opcode | Nom | Description |
|--------|-----|-------------|
| 0x200 | HEARTBEAT | Battement de cœur |
| 0x201 | EMERGENCY_STOP | Arrêt d'urgence |
| 0x202 | CLEAR_STOP | Effacer arrêt |

#### Commandes CAN LaBox (DCC NMRA)

| Opcode | Nom | Description |
|--------|-----|-------------|
| 0x00 | SYSTEM_CONTROL | Contrôle système |
| 0x04 | LOCO_SPEED | Vitesse loco |
| 0x05 | LOCO_DIRECTION | Direction loco |
| 0x06 | LOCO_FUNCTION | Fonctions F0-F28 |
| 0x08 | LOCO_WRITECV_MAIN | Écrire CV |

### 🎨 Aspects SNCF

```cpp
enum class ExccAspect : uint8_t
{
    ASPECT_CARRE = 0,           // Arrêt absolu
    ASPECT_CARRE_VIOLET,        // Arrêt + avertissement
    ASPECT_SEMAPHORE,           // Sémaphore
    ASPECT_AVERTISSEMENT,       // Avertissement
    ASPECT_RALENTISSEMENT_30,   // Ralentir à 30 km/h
    ASPECT_RALENTISSEMENT_60,   // Ralentir à 60 km/h
    ASPECT_RAPPEL_30,           // Rappel 30
    ASPECT_RAPPEL_60,           // Rappel 60
    ASPECT_VOIE_LIBRE,          // Voie libre
    ASPECT_MANOEUVRE,           // Manœuvre
    ASPECT_MASQUE,              // Masqué
};
```

### 🏷️ Types de Signaux

```cpp
enum SignalType : uint8_t
{
    SIG_ABSENT = 0,  // Aucun signal
    TYPE_A = 1,      // BAL (3 feux)
    TYPE_C = 2,      // Carré + BAL (5 feux)
    TYPE_E = 3,      // Ralentissement (7 feux)
    TYPE_G = 4,      // Rappel (9 feux)
    TYPE_M = 5,      // Manœuvre (2 feux)
};
```

---

## 📡 Communication

### Bus CAN

- **Driver** : ACAN_ESP32 (ESP32 natif) ou ACAN2515 (via MCP2515)
- **Vitesse** : Configurable (généralement 500 kbps ou 1 Mbps)
- **Filtres** : Filtrage matérielle des messages CAN
- **Bibliothèques** : 
  - `pierremolinaro/ACAN_ESP32`
  - `pierremolinaro/ACAN2515`

### Classes de Communication

#### CanBus
Interface unifiée pour le bus CAN :
- `CanBus::bus(1).send(msg)` - Envoyer un message
- Configuration des filtres
- Gestion des interruptions

#### CanMsg
Structure de message CAN :
```cpp
struct CanMsg {
    uint32_t id;      // Identifiant CAN
    uint8_t data[8];  // Données (0-8 bytes)
    uint8_t dlc;      // Longueur des données
};
```

#### CanID
Utilitaires pour la construction des identifiants CAN :
- `CanID::make29(priority, cmd, resp, cantonId)`
- Extraction des champs

---

## ⚙️ Configuration

### Fichier `EXCC_Config.h`

#### Options Générales
```cpp
#define EXCC_DEBUG false           // Debug série
#define EXCC_DEBOUNCE_MS 5         // Anti-rebond capteur
#define EXCC_MAX_ESSIEUX 200       // Limite comptage essieux
```

#### Paramètres WS2812
```cpp
#define EXCC_WS2812_GLOBAL_BRIGHTNESS 128  // Luminosité (0-255)
#define EXCC_WS2812_BLINK_PERIOD_MS 500     // Période clignotement
```

#### Sécurité Booster
```cpp
#define EXCC_BOOSTER_MAX_COURANT_mA 1400    // Courant max
#define EXCC_BOOSTER_MIN_TENSION_mV 12000   // Tension min
#define EXCC_BOOSTER_PHASE_TOLERANCE 0       // Tolérance phase
#define EXCC_BOOSTER_ENABLE_GLOBAL_PROTECTION true
#define EXCC_BOOSTER_ENABLE_GLOBAL_CUTOUT true
```

#### Mesure Courant (DRV8874)
```cpp
#define EXCC_IPROPI_R_OHMS 3600.0f      // Résistance IPROPI
#define EXCC_IPROPI_GAIN_A_PER_A 0.000455f // Gain
```

#### Mesure Tension
```cpp
#define EXCC_ADC_VOLTAGE_R1_OHMS 68000.0f   // Pont diviseur haut
#define EXCC_ADC_VOLTAGE_R2_OHMS 10000.0f   // Pont diviseur bas
#define EXCC_ADC_VREF 3.3f                 // Tension référence ADC
```

#### Seuils d'Occupation
```cpp
#define EXCC_SEUIL_OCCUPE_mA 15    // Courant > 15mA → OCCUPÉ
#define EXCC_SEUIL_LIBRE_mA 8      // Courant < 8mA → LIBRE
```

#### Nombre d'Aiguillages
```cpp
constexpr uint8_t AIG_COUNT = 6;  // 6 aiguillages max
```

### Fichier `platformio.ini`

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

monitor_speed = 115200
upload_speed = 921600

build_flags =
    -DCORE_DEBUG_LEVEL=0
    -Wno-deprecated-declarations

board_build.flash_mode = dio
board_build.flash_size = 4MB
board_build.partitions = default.csv

lib_deps =
    https://github.com/prune74/CAN_DCC-Booster.git
    prune74/CanUniversal
    adafruit/Adafruit PWM Servo Driver Library
    adafruit/Adafruit MCP23017 Arduino Library
    adafruit/Adafruit BusIO
    pierremolinaro/ACAN_ESP32
    pierremolinaro/ACAN2515
    fastled/FastLED @ ^3.6.0
```

---

## 🔧 Matériel

### Composants Principaux

| Composant | Rôle | Brochage |
|-----------|------|----------|
| ESP32 | Microcontrôleur | - |
| DRV8874 | Driver booster DCC | IN1, IN2, EN, IPROPI |
| PCA9685 | PWM pour servos | SDA, SCL |
| MCP23017 | Extension I/O | SDA, SCL |
| WS2812/WS2815 | LED signaux | GPIO configurable |
| MCP2515 | Contrôleur CAN (optionnel) | SPI |

### Broches (EXCC_Pins.h)

Les broches sont configurables via `EXCC_Pins.h`. Voici un exemple de configuration :

```cpp
// CAN Bus
#define CAN_CS_PIN 5
#define CAN_INT_PIN 4

// Servos (PCA9685)
#define PCA9685_SDA 21
#define PCA9685_SCL 22

// Booster (DRV8874)
#define BOOSTER_EN_PIN 12
#define BOOSTER_IN1_PIN 13
#define BOOSTER_IN2_PIN 14
#define BOOSTER_IPROPI_PIN 34  // Analog

// Capteurs
#define QUAD_H_A_PIN 15
#define QUAD_H_B_PIN 16
#define QUAD_AH_A_PIN 17
#define QUAD_AH_B_PIN 18

// LED
#define WS2812_DATA_PIN 23

// Switches
#define SWITCH_1_PIN 19
#define SWITCH_2_PIN 25
// ...
```

---

## 💡 Utilisation

### Compilation

```bash
# Installer PlatformIO
pio install

# Compiler pour ESP32
pio run -e esp32dev

# Flasher
pio run -e esp32dev -t upload
```

### Configuration des DIP Switch

Les DIP switches permettent de configurer le comportement du module :

| DIP | Position ON | Position OFF |
|-----|-------------|--------------|
| 1 | Booster activé | Booster désactivé |
| 2 | Debug série | Debug désactivé |
| 3 | Mode test | Mode normal |
| 4 | Réservé | Réservé |

### Monitor Série

```bash
pio device monitor -b 115200
```

En mode debug (`EXCC_DEBUG true`), les messages suivants apparaissent :
- `[BOOT]` : Démarrage du système
- `[RTOS]` : Information tâche FreeRTOS
- `[CAN]` : Messages CAN
- `[BOOSTER]` : État booster
- `[RAILCOM]` : Détection RailCom

---

## 📊 Signaux SNCF

### Correspondance Aspects ↔ LED

| Aspect | Type A (BAL) | Type C (Carré+BAL) | Type E (Ralentissement) | Type G (Rappel) | Type M (Manœuvre) |
|--------|---------------|---------------------|--------------------------|-----------------|-------------------|
| CARRE | 🔴 | 🔴🔴 | - | - | - |
| CARRE_VIOLET | 🔴🟣 | 🔴🔴🟣 | - | - | - |
| SEMAPHORE | 🟡 | 🟡🔴 | - | - | - |
| AVERTISSEMENT | 🟡🟡 | 🟡🟡🔴 | - | - | - |
| RALENTISSEMENT_30 | 🟡 | 🟡🔴 | 🟡 | - | - |
| RALENTISSEMENT_60 | 🟡🟡 | 🟡🟡🔴 | 🟡🟡 | - | - |
| RAPPEL_30 | - | - | - | 🟡 | - |
| RAPPEL_60 | - | - | - | 🟡🟡 | - |
| VOIE_LIBRE | 🟢 | 🟢🔴 | 🟢 | 🟢 | - |
| MANOEUVRE | - | - | - | - | ⚪⚪ |
| MASQUE | - | - | - | - | - |

### Clignotement

- **Ralentissement 60** : Clignotement à 500ms
- **Rappel 60** : Clignotement à 500ms
- **Autres** : Statique

---

## 🔄 Adaptation au DCC_CAN_RMT-Booster

### État d'avancement — 13 novembre 2026

Version PDF : [télécharger le bilan d'avancement](docs/EXCC_Booster_Avancement.pdf).

L'intégration conserve les autres fonctions EXCC (signaux, cantons, quadrature,
compteur d'essieux, servos, switches, LEDs et communication avec le
CantonController). La compilation PlatformIO complète a réussi. Le matériel
booster et le convertisseur n'étant pas disponibles pour les essais, les
résultats électriques et RailCom restent à confirmer sur la carte assemblée.

| Phase | État | Avancement et limites |
|---|---|---|
| 0 — Crash au boot | ✅ Validée | Ordre d'initialisation corrigé; le démarrage propre a été confirmé par les logs série. |
| 1 — Brochage et bus CAN | ✅ Validée logiciellement | Brochage cible et bus TWAI unique configurés; compilation et démarrage confirmés. |
| 2 — Modules booster | ✅ Code intégré; matériel à valider | RMT TX canal 7, interface DRV8874, mesure ADC, digipot et UART2 RailCom sont présents. Les valeurs de tension/digipot et les signaux sur la voie ne sont pas calibrés physiquement. |
| 3 — Front-end RailCom | ⏳ En attente du matériel | UART2 est configuré sur GPIO36 à 250 kbit/s, 8E1. BAT54, TLV9151, TLP2362 et la réception RailCom réelle doivent être vérifiés sur la carte. |
| 4 — Flux CAN vers DCC | 🟡 Intégration logicielle partielle | `0x100` est validé puis dépaqueté en FIFO avant émission RMT. `0x101` déclenche une fenêtre locale temporisée de 454 µs, car START/END sont transmis presque consécutivement. RailCom valide est remonté au CantonController. Le failsafe à 500 ms et la protection distante décrits dans le cahier des charges restent à compléter/vérifier. |
| 5 — Validation système | 🟡 Partielle | Build propre réussi et boot déjà observé. Oscilloscope, cutout réel, RailCom avec locomotive, profils HO/N et perte du convertisseur restent à tester avec le matériel. |

#### Format CAN du convertisseur

- `0x100` : `data[0]` est le nombre de bits `N`; les deux nibbles par octet
  encodent `(bit << 1) | phase`, premier nibble dans le poids faible. EXCC
  utilise le bit DCC et conserve les blocs dans une FIFO avant le RMT.
- Le convertisseur a été corrigé pour limiter les blocs à **14 bits** :
  compteur + sept octets de données = DLC 8, limite d'une trame CAN standard.
  La correction est fusionnée dans `main` du dépôt
  [DCC_CAN_RMT-Booster](https://github.com/prune74/DCC_CAN_RMT-Booster).
- `0x101` transporte des marqueurs de cutout, pas sa durée. EXCC termine donc
  localement sa fenêtre RailCom après 454 µs; le timing réel doit être mesuré
  sur la carte.

#### Travaux restants

1. Assembler le front-end RailCom et confirmer le signal UART sur GPIO36.
2. À l'oscilloscope, vérifier DCC 58/100 µs, inversion IN1/IN2 et cutout BRAKE
   de 454 µs.
3. Tester le décodage RailCom avec une locomotive compatible et vérifier le
   message reçu par le CantonController.
4. Implémenter et tester le failsafe convertisseur (perte de `0x100` > 500 ms)
   ainsi que la protection distante si ces exigences restent requises.
5. Calibrer courant, tension et profils HO/N avec le DRV8874 et le digipot.

---

## 📝 Changelog

### Version 2026 (Chemin de Fer du Chablais (CFC))

- **Migration vers WS2812** : Tous les signaux et LED utilisent maintenant WS2812/WS2815
- **Refactorisation du protocole** : Organisation moderne avec enum class par liaison CAN
- **Séparation des responsabilités** : Occupation centralisée dans EXCC_Occupation
- **Amélioration RailCom** : Gestion du cutout et décodage HF
- **Nouveau système de configuration** : Centralisation dans EXCC_Config.h
- **Support multi-aiguillages** : Jusqu'à 6 aiguillages configurables
- **Télémétrie avancée** : Mesure précise courant/tension via IPROPI

### Historique

- **2025** : Première version avec support CAN et booster DCC
- **2024** : Ajout de la gestion des signaux SNCF
- **2023** : Développement initial

---

## 🤝 Contribution

Les contributions sont les bienvenues ! Veuillez :

1. Forker le dépôt
2. Créer une branche pour votre fonctionnalité (`git checkout -b feature/nouvelle-fonctionnalité`)
3. Committer vos changements (`git commit -m 'Ajout de la nouvelle fonctionnalité'`)
4. Pousser vers la branche (`git push origin feature/nouvelle-fonctionnalité`)
5. Ouvrir une Pull Request

### Normes de Codage

- **Style** : C++ moderne (C++17)
- **Noms** : camelCase pour les variables, PascalCase pour les classes
- **Commentaires** : Documentation complète des interfaces publiques
- **Tests** : Ajouter des tests pour les nouvelles fonctionnalités

---

## 📄 Licence

Ce projet est sous licence **MIT**. Voir le fichier [LICENSE](LICENSE) pour plus de détails.

---

## 🙏 Remerciements

- [prune74](https://github.com/prune74) pour les bibliothèques CAN_DCC-Booster et CanUniversal
- [pierremolinaro](https://github.com/pierremolinaro) pour ACAN_ESP32 et ACAN2515
- [FastLED](https://github.com/FastLED) pour la gestion des LED WS2812
- [Adafruit](https://github.com/adafruit) pour les bibliothèques PCA9685 et MCP23017

---

## 📞 Support

Pour toute question ou problème, ouvrez une **Issue** sur GitHub.

---

*Documentation générée le 11 novembre 2025*
*Version : Chemin de Fer du Chablais (CFC)*
