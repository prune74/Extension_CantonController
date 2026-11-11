# 🚀 Configuration de Migration vers DCC_CAN_RMT-Booster

**Fichier de Configuration pour la Migration**
*Projet : Extension Canton Controller (EXCC)*
*Date : 11 novembre 2025*

---

## 📋 Instructions

Ce fichier contient les options de configuration pour la migration vers le **DCC_CAN_RMT-Booster**.

### 🎯 Comment utiliser ce fichier

1. **Choisir le type de booster** en décommentant la ligne correspondante
2. **Adapter platformio.ini** selon vos besoins
3. **Compiler et tester** avec chaque configuration

---

## ⚙️ Options de Configuration

### 🔧 Type de Booster

Choisir **UN SEUL** des options suivantes :

#### Option 1 : Utiliser le NOUVEAU DCC_CAN_RMT-Booster (Recommandé)

```cpp
// Dans EXCC_Main.cpp ou EXCC_Config.h
#define EXCC_USE_RMT_BOOSTER 1
```

Ou dans platformio.ini :
```ini
[env:esp32dev]
build_flags =
    -D EXCC_USE_RMT_BOOSTER=1
```

#### Option 2 : Utiliser l'ANCIEN CAN_DCC-Booster (Fallback)

```cpp
// Dans EXCC_Main.cpp ou EXCC_Config.h
// #define EXCC_USE_RMT_BOOSTER  // Commenter ou supprimer
```

Ou dans platformio.ini :
```ini
[env:esp32dev]
build_flags =
    # -D EXCC_USE_RMT_BOOSTER=1  // Commenter
```

#### Option 3 : Détection Automatique (Non implémenté par défaut)

```cpp
// Dans EXCC_BoosterFactory.cpp
// #define EXCC_BOOSTER_AUTO_DETECT 1
```

---

## 📦 Configuration PlatformIO

### 🎯 Pour le DCC_CAN_RMT-Booster

```ini
; PlatformIO Configuration File
; Pour utiliser DCC_CAN_RMT-Booster

[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

monitor_speed = 115200
upload_speed = 921600

build_flags =
    -DCORE_DEBUG_LEVEL=0
    -Wno-deprecated-declarations
    -D EXCC_USE_RMT_BOOSTER=1
    -D EXCC_DEBUG=true  ; Optionnel pour le debug

board_build.flash_mode = dio
board_build.flash_size = 4MB
board_build.partitions = default.csv

lib_deps =
    ; DCC_CAN_RMT-Booster (à adapter avec le vrai chemin)
    https://github.com/prune74/DCC_CAN_RMT-Booster.git
    
    ; Bibliothèques CAN
    prune74/CanUniversal
    pierremolinaro/ACAN_ESP32
    pierremolinaro/ACAN2515
    
    ; Bibliothèques matérielles
    adafruit/Adafruit PWM Servo Driver Library
    adafruit/Adafruit MCP23017 Arduino Library
    adafruit/Adafruit BusIO
    
    ; LED
    fastled/FastLED @ ^3.6.0
```

### 🎯 Pour l'ancien CAN_DCC-Booster (Fallback)

```ini
; PlatformIO Configuration File
; Pour utiliser l'ancien CAN_DCC-Booster

[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

monitor_speed = 115200
upload_speed = 921600

build_flags =
    -DCORE_DEBUG_LEVEL=0
    -Wno-deprecated-declarations
    ; -D EXCC_USE_RMT_BOOSTER=1  ; NE PAS activer
    -D EXCC_DEBUG=true  ; Optionnel pour le debug

board_build.flash_mode = dio
board_build.flash_size = 4MB
board_build.partitions = default.csv

lib_deps =
    ; Ancien booster
    https://github.com/prune74/CAN_DCC-Booster.git
    
    ; Bibliothèques CAN
    prune74/CanUniversal
    pierremolinaro/ACAN_ESP32
    pierremolinaro/ACAN2515
    
    ; Bibliothèques matérielles
    adafruit/Adafruit PWM Servo Driver Library
    adafruit/Adafruit MCP23017 Arduino Library
    adafruit/Adafruit BusIO
    
    ; LED
    fastled/FastLED @ ^3.6.0
```

---

## 🔌 Configuration des Broches

### 📍 DCC_CAN_RMT-Booster

Le DCC_CAN_RMT-Booster utilise probablement des broches différentes. Adapter **EXCC_Pins.h** :

```cpp
// EXCC_Pins.h - Configuration pour RMT Booster

// Broche RMT pour la génération DCC
#define DCC_RMT_GPIO 25  // GPIO25 pour RMT (exemple)

// Broche pour la mesure de courant (IPROPI)
#define BOOSTER_IPROPI_PIN 34

// Broche pour la mesure de tension
#define BOOSTER_VOLTAGE_PIN 35

// Broche pour le enable du booster
#define BOOSTER_EN_PIN 12

// Broche pour le cutout RailCom (si nécessaire)
#define RAILCOM_CUTOUT_PIN 13
```

### 📍 Ancien CAN_DCC-Booster

```cpp
// EXCC_Pins.h - Configuration pour l'ancien booster

// Broches PWM pour DCC
#define BOOSTER_IN1_PIN 13
#define BOOSTER_IN2_PIN 14
#define BOOSTER_EN_PIN 12

// Broche pour la mesure de courant (IPROPI)
#define BOOSTER_IPROPI_PIN 34

// Broche pour la mesure de tension
#define BOOSTER_VOLTAGE_PIN 35
```

---

## 🎛️ Configuration des Paramètres

### 📊 Paramètres RMT

Dans **EXCC_Config.h**, ajouter :

```cpp
// ============================================================
// Configuration RMT (DCC_CAN_RMT-Booster)
// ============================================================

// Canal RMT à utiliser (0-7)
#define EXCC_RMT_CHANNEL RMT_CHANNEL_0

// Résolution RMT en nanosecondes
#define EXCC_RMT_RESOLUTION_NS 12.5f

// Taille du buffer RMT (en items)
#define EXCC_RMT_BUFFER_SIZE 1024

// Priorité d'interruption RMT
#define EXCC_RMT_INTERRUPT_PRIORITY 1
```

### 📊 Paramètres DCC

```cpp
// ============================================================
// Configuration DCC
// ============================================================

// Période du signal DCC en microsecondes
#define EXCC_DCC_PERIOD_US 20000  // 20ms pour DCC standard

// Largeur du préambule en microsecondes
#define EXCC_DCC_PREAMBLE_US 14000

// Largeur du bit 0 en microsecondes
#define EXCC_DCC_BIT0_US 116

// Largeur du bit 1 en microsecondes
#define EXCC_DCC_BIT1_US 200
```

---

## 🔧 Configuration du Debug

### 📝 Activer le Logging de Migration

```cpp
// Dans EXCC_Config.h
#define EXCC_MIGRATION_LOG true
```

Ou dans platformio.ini :
```ini
build_flags =
    -D EXCC_MIGRATION_LOG=true
```

### 📝 Niveaux de Logging

```cpp
// Dans EXCC_Config.h
typedef enum {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_VERBOSE = 5
} LogLevel;

#define EXCC_LOG_LEVEL LOG_LEVEL_DEBUG
```

---

## 🎯 Configuration des Tests

### 🧪 Tests Unitaires

```ini
; Dans platformio.ini
[env:test]
platform = native
framework = ardutest

lib_deps =
    ; Dépendances pour les tests
    
build_flags =
    -D UNIT_TEST=1
```

### 🧪 Tests d'Intégration

```cpp
// Dans EXCC_Config.h
#define EXCC_INTEGRATION_TEST false  // Activer pour les tests d'intégration
```

---

## 📝 Configuration des Seuils

### 🔌 Protection Booster

```cpp
// Dans EXCC_Config.h

// Seuils de protection (adaptés pour le nouveau booster)
#define EXCC_BOOSTER_MAX_COURANT_mA 1500    // Courant max avant coupure
#define EXCC_BOOSTER_MIN_TENSION_mV 11000   // Tension min avant coupure
#define EXCC_BOOSTER_THERMAL_THRESHOLD 80   // Température max en °C

// Seuils RailCom
#define EXCC_RAILCOM_MIN_SAMPLE 100   // Valeur minimale RailCom
#define EXCC_RAILCOM_MAX_SAMPLE 900   // Valeur maximale RailCom

// Seuils d'occupation
#define EXCC_SEUIL_OCCUPE_mA 20       // Courant > 20mA → OCCUPÉ
#define EXCC_SEUIL_LIBRE_mA 10        // Courant < 10mA → LIBRE
```

---

## 🌐 Configuration Réseau (Optionnel)

### 📡 WiFi (Si supporté)

```cpp
// Dans EXCC_Config.h
#define EXCC_WIFI_ENABLED false  // Activer si nécessaire
#define EXCC_WIFI_SSID ""
#define EXCC_WIFI_PASSWORD ""
```

### 📡 CAN Bus

```cpp
// Dans EXCC_Config.h
#define EXCC_CAN_BAUD_RATE 500000  // 500 kbps (ou 1000000 pour 1 Mbps)
#define EXCC_CAN_USE_ACAN_ESP32 1  // Utiliser ACAN_ESP32 (ESP32 natif)
;#define EXCC_CAN_USE_ACAN_2515 0   // Utiliser ACAN2515 (MCP2515)
```

---

## ⚡ Configuration des Performances

### ⚙️ FreeRTOS

```cpp
// Dans EXCC_Config.h
#define EXCC_TASK_PRIORITY 1          // Priorité de la tâche EXCC
#define EXCC_TASK_STACK_SIZE 4096     // Taille du stack en bytes
#define EXCC_TASK_CORE APP_CPU_NUM    // Cœur CPU (0 ou 1)

// Pour le booster (si tâche séparée)
#define EXCC_BOOSTER_TASK_PRIORITY 2
#define EXCC_BOOSTER_TASK_STACK_SIZE 2048
```

### ⚙️ Watchdog

```cpp
// Dans EXCC_Config.h
#define EXCC_WDT_TIMEOUT_SEC 2        // Timeout watchdog en secondes
#define EXCC_WDT_ENABLED true        // Activer le watchdog
```

---

## 📋 Checklist de Vérification

### ✅ Avant Compilation

- [ ] Type de booster sélectionné (RMT ou OLD)
- [ ] platformio.ini adapté
- [ ] EXCC_Pins.h configuré pour le bon booster
- [ ] Toutes les dépendances installées
- [ ] Pas de conflits de définition

### ✅ Après Compilation

- [ ] Aucune erreur de compilation
- [ ] Taille du binaire acceptable (< 1MB)
- [ ] Mémoire utilisée < 80%
- [ ] Pas d'avertissements critiques

### ✅ Après Flash

- [ ] LED de statut allumée
- [ ] Port série fonctionnel
- [ ] Messages de boot visibles
- [ ] Pas d'erreurs immédiates

### ✅ Tests Fonctionnels

- [ ] Génération DCC vérifiée
- [ ] Communication CAN fonctionnelle
- [ ] Télémétrie précise
- [ ] RailCom détecté
- [ ] Protections actives

---

## 💡 Résolution des Problèmes

### ❌ Problème : Erreur de compilation "DCC_CAN_RMT_Booster.h not found"

**Solution** :
1. Vérifier que le dépôt a été cloné :
   ```bash
   ls lib/DCC_CAN_RMT_Booster/src/DCC_CAN_RMT_Booster.h
   ```
2. Vérifier le chemin dans l'include :
   ```cpp
   #include "DCC_CAN_RMT_Booster.h"
   // ou
   #include <DCC_CAN_RMT_Booster.h>
   ```
3. Vérifier platformio.ini :
   ```ini
   lib_deps =
       https://github.com/prune74/DCC_CAN_RMT-Booster.git
   ```

### ❌ Problème : Conflit de définition entre les deux boosters

**Solution** :
1. Utiliser des namespaces :
   ```cpp
   namespace RMT_Booster {
       class Booster { ... };
   }
   
   namespace Old_Booster {
       class Booster { ... };
   }
   ```
2. Renommer les classes en conflit
3. Utiliser des #ifdef :
   ```cpp
   #ifdef EXCC_USE_RMT_BOOSTER
   #include "DCC_CAN_RMT_Booster.h"
   #else
   #include "CanDccBooster.h"
   #endif
   ```

### ❌ Problème : Signal DCC non généré

**Solution** :
1. Vérifier que le booster est initialisé :
   ```cpp
   g_booster->begin();
   ```
2. Vérifier que la sortie est activée :
   ```cpp
   g_booster->enableOutput();
   ```
3. Vérifier les broches :
   ```cpp
   #define DCC_RMT_GPIO 25  // Vérifier que c'est la bonne broche
   ```
4. Vérifier avec un oscilloscope
5. Tester avec un exemple simple du RMT

### ❌ Problème : Communication CAN ne fonctionne pas

**Solution** :
1. Vérifier la vitesse CAN :
   ```cpp
   #define EXCC_CAN_BAUD_RATE 500000
   ```
2. Vérifier les broches CAN :
   ```cpp
   #define CAN_CS_PIN 5
   #define CAN_INT_PIN 4
   ```
3. Vérifier l'initialisation :
   ```cpp
   CanBus::bus(1).begin();
   ```
4. Tester avec un analyseur CAN
5. Vérifier que les identifiants CAN sont corrects

---

## 🎯 Configuration Recommandée pour la Migration

### 📍 Phase 1 : Test avec l'ancien booster (Fallback)

```ini
[env:esp32dev]
build_flags =
    ; -D EXCC_USE_RMT_BOOSTER=1  ; DÉSACTIVÉ
    -D EXCC_DEBUG=true
    -D EXCC_LOG_LEVEL=LOG_LEVEL_DEBUG
```

### 📍 Phase 2 : Migration vers RMT

```ini
[env:esp32dev]
build_flags =
    -D EXCC_USE_RMT_BOOSTER=1  ; ACTIVÉ
    -D EXCC_DEBUG=true
    -D EXCC_LOG_LEVEL=LOG_LEVEL_DEBUG
    -D EXCC_MIGRATION_LOG=true
```

### 📍 Phase 3 : Production

```ini
[env:esp32dev]
build_flags =
    -D EXCC_USE_RMT_BOOSTER=1  ; ACTIVÉ
    ; -D EXCC_DEBUG=false  ; DÉSACTIVÉ
    ; -D EXCC_MIGRATION_LOG=false  ; DÉSACTIVÉ
```

---

## 📚 Annexes

### 🔗 Liens vers la Documentation

- [README.md](README.md) - Documentation principale
- [AUDIT_CANVAS.md](AUDIT_CANVAS.md) - Audit technique
- [ADAPTATION_DCC_CAN_RMT_Booster.md](ADAPTATION_DCC_CAN_RMT_Booster.md) - Guide de migration

### 📞 Support

Pour toute question sur la configuration :
1. Consulter ce fichier
2. Consulter le README.md
3. Vérifier les logs série
4. Ouvrir une issue sur GitHub

---

*Fichier de configuration généré par Mistral Vibe - 11 novembre 2025*
*Version : 1.0*
