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
- Détection d'occupation par courant mesuré par le Booster externe et transmis par UART
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

### ✅ Supervision du Booster DCC
- Commandes de puissance et profil de voie relayées par UART
- Mesures courant/tension reçues à 1 kHz
- État et événements RailCom relayés vers le CC
- La génération DCC, le DRV8874 et le cutout RailCom sont sur l’ESP32 Booster séparé

### ✅ Communication CAN
- Protocole CAN 2.0B (29 bits)
- Communication bidirectionnelle avec le CantonController (CC)
- Gestion des priorités
- Filtres de messages

### ✅ Télémétrie
- Courant et tension du Booster reçus par UART à 1 kHz
- Événements RailCom relayés au CC
- LED de statut CAN Command, RailCom et Booster

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
│  │EXCC_BoosterUart │  │         EXCC_CanCommands                  │  │
│  │ UART2 460800    │  │ CAN Command, bus 0, 250 kbit/s         │  │
│  │ mesures/RailCom │  │ EXCC_Callbacks (CC → EXCC)              │  │
│  └─────────────────┘  └──────────────────────────────────────────┘  │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │                    CAN Command et UART                       │  │
│  │  - CAN Command : EXCC_CAN_CC / EXCC_CanCommands, bus 0        │  │
│  │  - UART : commandes, mesures 1 kHz, état et RailCom          │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘

UART2 TX/RX GPIO16/17
          │
          ▼
┌─────────────────────────────────────────┐
│ ESP32 Booster externe                   │
│ CAN DCC · génération DCC · DRV8874      │
│ mesures courant/tension · RailCom       │
└─────────────────────────────────────────┘
```

### Diagramme de Flux de Données

```
LaBox/Hub ── CAN DCC ── DCC_CAN_RMT-Booster ── CAN DCC ── ESP32 Booster
LaBox ── CAN général ── CantonController (CC)
CC ── CAN Command (250 kbit/s) ── EXCC
EXCC ── UART (460800 bauds, commandes/télémétrie) ── ESP32 Booster
ESP32 Booster ── RMT ── DRV8874 ── voie

La trame 0x102 reste sur le CAN DCC pour la protection entre boosters.
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
│   ├── EXCC_BoosterUart.h/cpp     # Passerelle UART vers le booster externe
│   ├── UartProtocol.h             # Trames versionnées et CRC-16
│   ├── EXCC_CAN_CC.h/cpp          # Envoi de messages CAN (EXCC → CC)
│   ├── 
│   ├── EXCC_Callbacks.h/cpp       # Gestion des commandes CC → EXCC
│   ├── EXCC_CanCommands.h/cpp      # Traitement des commandes CAN
│   ├── EXCC_CanConfig.h           # Configuration CAN
│   ├── 
│   ├── EXCC_Signaux_WS2812.h/cpp  # Gestion des signaux SNCF (WS2812)
│   ├── EXCC_Canton_WS2812.h/cpp   # Gestion LED canton
│   ├── EXCC_LedDirection_WS2812.h/cpp # LED directionnelles
│   ├── 
│   ├── EXCC_Servo.h/cpp           # Contrôle des servos (aiguillages)
│   ├── EXCC_Switches.h/cpp        # Gestion des micro-switches
│   ├── EXCC_Quadrature.h/cpp      # Capteurs quadrature
│   ├── 
│   ├── EXCC_Occupation.h/cpp      # Gestion occupation canton
│   ├── EXCC_Compteur.h/cpp        # Compteur général
│   ├── EXCC_CompteurEssieux.h/cpp # Compteur d'essieux
│   ├── 
│   ├── EXCC_Calibration.h/cpp     # Calibration des capteurs
│   ├── EXCC_Runtime.h/cpp         # Runtime et utilitaires
│   ├── EXCC_System.h/cpp          # Gestion système
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

- **CAN Command EXCC↔CC** : TWAI natif, bus 0, 250 kbit/s
- **CAN DCC** : reste sur le TWAI du Booster externe; il n’est pas raccordé à l’EXCC
- **CAN général** : réseau physique distinct
- **Bibliothèque EXCC** : `prune74/CanUniversal @ 3.1.1`

### Liaison UART EXCC↔Booster

- **Débit** : 460800 bauds, 8N1
- **EXCC** : UART2, TX GPIO16 / RX GPIO17
- **Booster** : UART1, TX GPIO17 / RX GPIO16
- Trames versionnées avec CRC-16; échantillons de courant/tension à 1 kHz
- Heartbeat EXCC toutes les 100 ms; la sortie Booster est coupée après 250 ms sans heartbeat valide

---

## ⚙️ Configuration

### Fichier `EXCC_Config.h`

#### Options Générales
```cpp
#define EXCC_DEBUG true            // Debug série (à désactiver après validation)
#define EXCC_UART_BAUDRATE 115200  // Port série de diagnostic
#define EXCC_DEBOUNCE_MS 5         // Anti-rebond capteur
#define EXCC_MAX_ESSIEUX 200       // Limite comptage essieux
```

#### Paramètres WS2812
```cpp
#define EXCC_WS2812_GLOBAL_BRIGHTNESS 128  // Luminosité (0-255)
#define EXCC_WS2812_BLINK_PERIOD_MS 500     // Période clignotement
```

#### Supervision Booster
```cpp
#define EXCC_BOOSTER_MAX_COURANT_mA 1400    // Courant max
#define EXCC_BOOSTER_MIN_TENSION_mV 12000   // Tension min
```
La mesure est effectuée par le Booster externe puis transmise à l’EXCC par UART.

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
platform = espressif32@6.9.0
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
    prune74/Ws2812Universal @ 1.1.0
    prune74/CanUniversal @ 3.1.1
    adafruit/Adafruit PWM Servo Driver Library
    adafruit/Adafruit MCP23017 Arduino Library
    adafruit/Adafruit BusIO
```

---

## 🔧 Matériel

### Composants Principaux

| Composant | Rôle | Brochage |
|-----------|------|----------|
| ESP32 EXCC | Cantons, signaux, servos, switches et passerelle UART/CAN | - |
| ESP32 Booster externe | Génération DCC, DRV8874, mesures et RailCom | UART avec EXCC |
| PCA9685 | PWM pour servos | SDA, SCL |
| MCP23017 | Extension I/O | SDA, SCL |
| WS2812/WS2815 | LED signaux | GPIO configurable |
| Transceiver CAN | CAN Command EXCC↔CC | TWAI TX/RX |

### Broches (EXCC_Pins.h)

Les broches sont configurables via `EXCC_Pins.h`. Voici un exemple de configuration :

```cpp
// CAN Command (TWAI natif)
#define PIN_CAN_TX 4
#define PIN_CAN_RX 5
#define CAN_BITRATE 250000UL

// Servos (PCA9685)
#define PCA9685_SDA 21
#define PCA9685_SCL 22

// UART vers le Booster externe
#define PIN_BOOSTER_UART_TX 16
#define PIN_BOOSTER_UART_RX 17

// Capteurs quadrature
#define PIN_QUAD_H_A 12
#define PIN_QUAD_H_B 14
#define PIN_QUAD_AH_A 34
#define PIN_QUAD_AH_B 35

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

## 🔄 Migration vers le Booster externe

L'EXCC ne génère plus le DCC et ne pilote plus directement le DRV8874. Ces
fonctions sont exécutées par l'ESP32 Booster séparé. L'EXCC conserve la
calibration du courant, la décision d'occupation et le relais des commandes et
de la télémétrie entre le CantonController et le Booster.

### Interfaces et sûreté

- **CAN Command** : TWAI EXCC, bus 0, 250 kbit/s, vers le CantonController.
- **CAN DCC** : bus distinct entre le convertisseur DCC et le TWAI Booster.
  La trame historique `0x102` est conservée pour la protection entre boosters.
- **UART** : EXCC UART2 (GPIO16/17) vers Booster UART1 (GPIO17/16), 460800
  bauds, 8N1. Les commandes de puissance/profil vont à l'ESP32 Booster; les
  mesures, l'état et les événements RailCom reviennent à l'EXCC.
- Les trames UART sont versionnées, séquencées et protégées par CRC-16; les
  définitions des deux projets (`src/UartProtocol.h`) doivent rester identiques.
- Heartbeat EXCC toutes les 100 ms; le Booster coupe sa sortie après 250 ms sans
  heartbeat valide. Une mesure invalide ou vieille de plus de 10 ms est
  signalée en défaut et force le canton occupé.
- Le courant est transmis à 1 kHz. Calibration et seuils d'occupation restent
  appliqués côté EXCC.

### Validation

Les builds PlatformIO EXCC et Booster ont réussi après l'intégration UART.
Les essais matériels ne sont pas encore réalisés : vérifier l'échange UART,
les timeouts et coupures, les commandes ON/OFF et profil, les mesures et la
détection d'occupation, ainsi que le relais des événements RailCom. La
conception PCB est reportée; voir le [plan de migration](docs/Plan_Migration_Booster_Externe_EXCC.md).

---

## 📝 Changelog

### Version 2026 (Chemin de Fer du Chablais (CFC))

- **Migration vers WS2812** : Tous les signaux et LED utilisent maintenant WS2812/WS2815
- **WS2812** : Remplacement de FastLED par `prune74/Ws2812Universal @ 1.1.0`
- **Refactorisation du protocole** : Organisation moderne avec enum class par liaison CAN
- **Séparation des responsabilités** : Occupation centralisée dans EXCC_Occupation
- **Relais RailCom** : Événements du Booster transmis à l'EXCC par UART
- **Nouveau système de configuration** : Centralisation dans EXCC_Config.h
- **Support multi-aiguillages** : Jusqu'à 6 aiguillages configurables
- **Télémétrie Booster** : Courant et tension reçus par UART; occupation calculée côté EXCC

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

- [prune74](https://github.com/prune74) pour CanUniversal et Ws2812Universal
- [Ws2812Universal](https://github.com/prune74/Ws2812Universal) pour les bus WS2812 RMT
- [Adafruit](https://github.com/adafruit) pour les bibliothèques PCA9685 et MCP23017

---

## 📞 Support

Pour toute question ou problème, ouvrez une **Issue** sur GitHub.

---

*Documentation générée le 13 novembre 2026*
*Version : Chemin de Fer du Chablais (CFC)*
