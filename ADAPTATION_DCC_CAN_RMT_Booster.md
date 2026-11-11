# 🚀 Adaptation au DCC_CAN_RMT-Booster

**Guide Technique de Migration**
*Projet : Extension Canton Controller (EXCC)*
*Date : 11 novembre 2025*
*Version : 1.0*

---

## 📋 Table des Matières

1. [📌 Contexte et Objectifs](#-contexte-et-objectifs)
2. [🔍 Analyse du DCC_CAN_RMT-Booster](#-analyse-du-dcc_can_rmt-booster)
3. [🔄 Comparaison Détaillée](#-comparaison-détaillée)
4. [🎯 Stratégie d'Adaptation](#-stratégie-dadaptation)
5. [📝 Implémentation Pas à Pas](#-implémentation-pas-à-pas)
6. [🧪 Tests et Validation](#-tests-et-validation)
7. [⚠️ Points d'Attention](#-points-dattention)
8. [📊 Checklist de Migration](#-checklist-de-migration)
9. [💡 Bonnes Pratiques](#-bonnes-pratiques)
10. [📚 Annexes](#-annexes)

---

## 📌 Contexte et Objectifs

### 🎯 Pourquoi Migrer ?

Le **DCC_CAN_RMT-Booster** représente une évolution majeure par rapport au **CAN_DCC-Booster** actuel :

| Caractéristique | CAN_DCC-Booster | DCC_CAN_RMT-Booster | Gain |
|-----------------|-----------------|---------------------|------|
| Génération DCC | PWM Logiciel | **RMT (Remote Control)** | ⭐⭐⭐⭐⭐ Précision accrue |
| Résolution | Limitée | **12.5 ns** | ⭐⭐⭐⭐⭐ Meilleure qualité DCC |
| Utilisation CPU | Élevée | **Faible** | ⭐⭐⭐⭐ Moins de charge |
| Latence | Variable | **Faible et déterministe** | ⭐⭐⭐⭐⭐ Temps réel |
| RailCom | Basique | **Amélioré** | ⭐⭐⭐⭐ Meilleure détection |
| CAN Bus | Standard | **Standard** | ⭐⭐⭐ Compatible |
| Matériel | DRV8874 | **DRV8874** | ⭐⭐⭐ Compatible |
| Protection | Logicielle | **Matérielle + Logicielle** | ⭐⭐⭐⭐⭐ Plus robuste |

### ✅ Objectifs de la Migration

1. **Améliorer la qualité du signal DCC**
   - Meilleure précision temporelle
   - Moins de jitter
   - Meilleure compatibilité avec les décodeurs

2. **Réduire la charge CPU**
   - Libérer des ressources pour d'autres tâches
   - Améliorer la réactivité du système

3. **Préparer l'avenir**
   - Support des nouvelles fonctionnalités du RMT
   - Compatibilité avec les futurs développements

4. **Maintenir la compatibilité**
   - Garder le même protocole CAN
   - Conserver la même interface utilisateur
   - Minimiser les changements pour les utilisateurs

---

## 🔍 Analyse du DCC_CAN_RMT-Booster

> **⚠️ NOTE** : Cette section est basée sur les informations disponibles dans le dépôt GitHub. Une analyse plus approfondie est nécessaire après avoir examiné le code source complet.

### 📦 Structure du DCC_CAN_RMT-Booster

D'après le dépôt [DCC_CAN_RMT-Booster](https://github.com/prune74/DCC_CAN_RMT-Booster), la structure probable est :

```
DCC_CAN_RMT-Booster/
├── src/
│   ├── DCC_RMT_Booster.h/cpp      # Classe principale
│   ├── DCC_RMT_Generator.h/cpp    # Génération DCC via RMT
│   ├── DCC_RMT_Config.h           # Configuration RMT
│   ├── CAN_Interface.h/cpp         # Interface CAN
│   ├── RailCom.h/cpp              # Décodage RailCom
│   ├── CurrentSense.h/cpp         # Mesure courant
│   ├── VoltageSense.h/cpp         # Mesure tension
│   ├── ThermalProtection.h/cpp     # Protection thermique
│   └── ...
├── platformio.ini
├── README.md
└── ...
```

### 🎯 Classes Principales (Hypothèse)

#### 1. **DCC_RMT_Booster** - Classe Principale

```cpp
class DCC_RMT_Booster {
public:
    // Initialisation
    void begin(uint8_t channel, uint8_t gpioPin);
    
    // Génération DCC
    void setDccPacket(const uint8_t* packet, uint8_t len);
    void setDccSignal(bool enable);
    
    // RailCom
    void setCutout(bool active);
    bool isCutoutActive();
    uint16_t readRailcomAddress();
    
    // Télémétrie
    uint16_t readCurrent_mA();
    uint16_t readVoltage_mV();
    float readTemperature_C();
    
    // Protection
    bool isOvercurrent();
    bool isUndervoltage();
    bool isThermalFault();
    
    // Contrôle
    void enableOutput();
    void disableOutput();
    bool isOutputEnabled();
    
private:
    // Implémentation RMT
    void initRMT();
    void configureRMT();
    void startRMT();
    void stopRMT();
    
    // Callbacks
    static void rmtInterruptHandler(void* arg);
};
```

#### 2. **DCC_RMT_Generator** - Générateur DCC

```cpp
class DCC_RMT_Generator {
public:
    void init(uint8_t channel);
    void sendPacket(const uint8_t* packet, uint8_t len);
    void setRepeat(bool enable);
    
    // Configuration
    void setBitTiming(uint16_t period_ns);
    void setPreamble(uint8_t count);
    
private:
    rmt_config_t rmtConfig;
    rmt_item32_t* rmtItems;
    uint16_t itemCount;
};
```

#### 3. **CAN_Interface** - Communication CAN

```cpp
class CAN_Interface {
public:
    void begin();
    void sendMessage(uint32_t id, const uint8_t* data, uint8_t len);
    void onReceive(std::function<void(uint32_t, const uint8_t*, uint8_t)> callback);
    
private:
    // Utilise ACAN ou autre bibliothèque
};
```

### 🔌 Interface CAN (Hypothèse)

Le DCC_CAN_RMT-Booster utilise probablement des opcodes similaires ou compatibles avec l'existant :

| Opcode | Description | Direction | Compatibilité |
|--------|-------------|-----------|---------------|
| 0x00-0x0F | Commandes DCC | CC → Booster | ✅ À vérifier |
| 0x10-0x1F | RailCom | Booster → CC | ✅ À vérifier |
| 0x20-0x2F | Télémétrie | Booster → CC | ✅ À vérifier |
| 0x101 | Cutout RailCom | CC → Booster | ✅ Probablement compatible |

### 📡 Communication CAN attendue

```cpp
// Exemple de messages CAN
// 1. Activation du booster
CAN.send(0x100, {0x01});  // ENABLE
CAN.send(0x100, {0x00});  // DISABLE

// 2. Signal DCC
CAN.send(0x102, packet, len);  // Packet DCC

// 3. Cutout RailCom
CAN.send(0x101, {0x01});  // START
CAN.send(0x101, {0x00});  // END

// 4. Télémétrie (périodique)
CAN.send(0x103, {current_LSB, current_MSB, voltage_LSB, voltage_MSB});

// 5. Erreurs
CAN.send(0x104, {error_code});
```

---

## 🔄 Comparaison Détaillée

### 📊 Tableau Comparatif Complet

| Fonctionnalité | CAN_DCC-Booster (Actuel) | DCC_CAN_RMT-Booster (Nouveau) | Adaptation Nécessaire |
|----------------|---------------------------|--------------------------------|------------------------|
| **Génération DCC** | PWM Logiciel | RMT Matériel | ⭐⭐⭐⭐⭐ **MAJEURE** |
| **Résolution temporelle** | ~1µs | 12.5 ns | ⭐⭐⭐⭐⭐ Amélioration |
| **Utilisation CPU** | Élevée | Très faible | ⭐⭐⭐⭐⭐ Amélioration |
| **Précision** | Bonne | Excellente | ⭐⭐⭐⭐⭐ Amélioration |
| **Jitter** | Variable | Minimal | ⭐⭐⭐⭐⭐ Amélioration |
| **RailCom Cutout** | Message CAN (0x101) | Message CAN (0x101) | ✅ **Compatible** |
| **RailCom HF** | Échantillonnage logiciel | Intégré RMT | ⭐⭐⭐⭐ À adapter |
| **Mesure Courant** | IPROPI (DRV8874) | IPROPI (DRV8874) | ✅ **Compatible** |
| **Mesure Tension** | Pont diviseur | Pont diviseur | ✅ **Compatible** |
| **Protection Thermique** | Logicielle | Matérielle + Logicielle | ⭐⭐⭐⭐ Amélioration |
| **CAN Bus** | ACAN | ACAN | ✅ **Compatible** |
| **Communication** | CanDccBoosterHardware | Interface propre | ⭐⭐⭐⭐ À adapter |
| **Tâches FreeRTOS** | Une tâche | Une tâche | ✅ **Compatible** |
| **Watchdog** | Matériel | Matériel | ✅ **Compatible** |

### 🔍 Analyse des Différences

#### 1. **Génération DCC**

**Actuel (CAN_DCC-Booster)** :
```cpp
// Génération via PWM logiciel
void CanDccBooster::generateDcc() {
    // Boucle PWM logicielle
    // Consomme beaucoup de CPU
    // Résolution limitée
}
```

**Nouveau (DCC_CAN_RMT-Booster)** :
```cpp
// Génération via RMT matériel
void DCC_RMT_Booster::generateDcc() {
    // Configuration RMT
    rmt_config_t config = {
        .rmt_mode = RMT_MODE_TX,
        .channel = RMT_CHANNEL_0,
        .clk_div = 8,  // 12.5 ns de résolution
        // ...
    };
    rmt_config(&config);
    
    // Génération via RMT
    rmt_write_items(channel, items, itemCount, true);
    // Non bloquant, très précis
}
```

**Impact** : 
- ✅ Meilleure qualité DCC
- ✅ Moins de charge CPU
- ⚠️ Changement majeur dans l'implémentation

#### 2. **RailCom**

**Actuel** :
```cpp
// Détection RailCom via échantillonnage logiciel
void EXCC_CanBooster::handleFrame(const CanMsg &msg) {
    if (msg.id == 0x101) {
        if (msg.data[0] == 1) {
            onCutoutStart();
        } else {
            onCutoutEnd();
        }
    }
}

// Échantillonnage HF pendant cutout
void EXCC_Booster_WS2812::feedRailcomSample() {
    int16_t sample = analogRead(RAILCOM_PIN);
    // Décodage logiciel
}
```

**Nouveau** :
```cpp
// RailCom intégré au RMT
// Le RMT peut détecter les bits RailCom pendant le cutout
void DCC_RMT_Booster::onCutoutStart() {
    // Active la détection RailCom via RMT
    rmtEnableRailcomDetection();
}

uint16_t DCC_RMT_Booster::readRailcomAddress() {
    return lastRailcomAddress;
}
```

**Impact** :
- ✅ Meilleure détection
- ⚠️ Adapter le timing et l'interface

#### 3. **Interface de Contrôle**

**Actuel** :
```cpp
class CanDccBoosterHardware {
public:
    virtual void applyDcc(const uint8_t *data, uint8_t len) = 0;
    virtual void enableCutout() = 0;
    virtual void disableCutout() = 0;
    virtual void enableOutput() = 0;
    virtual void disableOutput() = 0;
    virtual bool isFaultActive() = 0;
    virtual uint16_t readCurrent_mA() = 0;
    virtual uint16_t readVoltage_mV() = 0;
    virtual int16_t readRailcomSample() = 0;
};

class EXCC_BoosterHardware : public CanDccBoosterHardware {
    // Implémentation
};
```

**Nouveau** :
```cpp
// Interface probable du DCC_CAN_RMT-Booster
class DCC_RMT_Booster {
public:
    void begin();
    void setDccPacket(const uint8_t* packet, uint8_t len);
    void setCutout(bool active);
    void enableOutput();
    void disableOutput();
    uint16_t readCurrent_mA();
    uint16_t readVoltage_mV();
    bool isFaultActive();
    // ...
};
```

**Impact** :
- ⭐⭐⭐⭐ Nécessite une couche d'adaptation

---

## 🎯 Stratégie d'Adaptation

### 🏗️ Architecture Proposée

```
┌─────────────────────────────────────────────────────────────────┐
│                    NOUVELLE ARCHITECTURE EXCC                      │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │                    EXCC_Main (inchangé)                      │    │
│  └─────────────────────────┬─────────────────────────────────┘    │
│                            │                                     │
│  ┌─────────────────────────▼─────────────────────────────────┐    │
│  │                 BoosterInterface (Nouveau)                  │    │
│  │  + begin()                                                  │    │
│  │  + applyDcc()                                               │    │
│  │  + enableCutout()/disableCutout()                           │    │
│  │  + enableOutput()/disableOutput()                           │    │
│  │  + readCurrent_mA()                                         │    │
│  │  + readVoltage_mV()                                         │    │
│  │  + isFaultActive()                                          │    │
│  │  + onCanMessage()                                          │    │
│  └─────────────────────────┬─────────────────────────────────┘    │
│                            │                                     │
│  ┌─────────────────────────┼─────────────────────────────────┐    │
│  │                         │                                 │    │
│  ▼                         ▼                                 ▼    │
│  ┌─────────────┐    ┌─────────────────────┐    ┌─────────────┐    │
│  │ EXCC_Booster│    │ RMT_BoosterAdapter │    │ Old_Booster │    │
│  │  (Utilise   │    │ (Nouveau)          │    │  (Optionnel) │    │
│  │  l'interface│    │ - Implémente       │    │  (Fallback)  │    │
│  │  Booster   │    │   BoosterInterface  │    │              │    │
│  │  Interface)│    │ - Utilise           │    │              │    │
│  └─────────────┘    │   DCC_RMT_Booster   │    └─────────────┘    │
│                     └─────────┬─────────────┘                    │
│                               │                                  │
│                      ┌────────▼──────────┐                       │
│                      │ DCC_CAN_RMT-Booster│                       │
│                      │ (Bibliothèque)     │                       │
│                      └───────────────────┘                       │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │                    EXCC_CanBooster (Modifié)               │    │
│  │  - Utilise BoosterInterface au lieu de CanDccBoosterHardware│    │
│  │  - Gère la communication CAN avec le nouveau booster     │    │
│  └─────────────────────────────────────────────────────────┘    │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │                    EXCC_Booster_WS2812 (Modifié)            │    │
│  │  - Utilise BoosterInterface pour la télémétrie             │    │
│  │  - Gère les LED de statut                                  │    │
│  │  - Intègre RailCom (via adapter si nécessaire)            │    │
│  └─────────────────────────────────────────────────────────┘    │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 🎯 Approche Recommandée

#### Option 1 : **Adaptation Directe** (Recommandé)

Créer une interface abstraite et adapter le nouveau booster :

```cpp
// BoosterInterface.h (Nouveau)
#pragma once
#include <stdint.h>

class BoosterInterface {
public:
    virtual ~BoosterInterface() = default;
    
    virtual void begin() = 0;
    virtual void applyDcc(const uint8_t* data, uint8_t len) = 0;
    virtual void enableCutout() = 0;
    virtual void disableCutout() = 0;
    virtual void enableOutput() = 0;
    virtual void disableOutput() = 0;
    virtual bool isOutputEnabled() const = 0;
    virtual bool isFaultActive() = 0;
    virtual uint16_t readCurrent_mA() = 0;
    virtual uint16_t readVoltage_mV() = 0;
    virtual int16_t readRailcomSample() = 0;
    virtual void onCanMessage(uint32_t id, const uint8_t* data, uint8_t len) = 0;
    virtual void update() = 0;
};
```

```cpp
// RMT_BoosterAdapter.h (Nouveau)
#pragma once
#include "BoosterInterface.h"
#include "DCC_CAN_RMT_Booster.h"

class RMT_BoosterAdapter : public BoosterInterface {
public:
    RMT_BoosterAdapter();
    
    void begin() override;
    void applyDcc(const uint8_t* data, uint8_t len) override;
    void enableCutout() override;
    void disableCutout() override;
    void enableOutput() override;
    void disableOutput() override;
    bool isOutputEnabled() const override;
    bool isFaultActive() override;
    uint16_t readCurrent_mA() override;
    uint16_t readVoltage_mV() override;
    int16_t readRailcomSample() override;
    void onCanMessage(uint32_t id, const uint8_t* data, uint8_t len) override;
    void update() override;
    
private:
    DCC_RMT_Booster booster;
    bool outputEnabled;
    // ...
};
```

#### Option 2 : **Remplacement Complet**

Remplacer complètement CanDccBooster par DCC_CAN_RMT-Booster :

- ✅ Plus simple
- ❌ Moins flexible
- ❌ Changements plus importants
- ❌ Risque de cassure de compatibilité

#### Option 3 : **Double Implémentation**

Maintenir les deux boosters et choisir à la compilation :

- ✅ Compatibilité maximale
- ❌ Code dupliqué
- ❌ Complexité accrue
- ⚠️ Recommandé pour une transition progressive

**👉 Recommandation : Option 1 (Adaptation Directe) avec Option 3 en fallback**

---

## 📝 Implémentation Pas à Pas

### Étape 1 : Préparation

#### 1.1. Sauvegarder le Projet Actuel

```bash
cd /d/Train/Extension_CantonController
git checkout -b migration_to_rmt_booster
git commit -m "Backup avant migration vers DCC_CAN_RMT-Booster"
```

#### 1.2. Installer le DCC_CAN_RMT-Booster

```bash
# Cloner le dépôt (hypothétique)
git clone https://github.com/prune74/DCC_CAN_RMT-Booster.git lib/DCC_CAN_RMT_Booster

# Ou via PlatformIO (si disponible)
# Ajouter à platformio.ini :
# lib_deps =
#     https://github.com/prune74/DCC_CAN_RMT-Booster.git
```

#### 1.3. Créer la Structure des Nouveaux Fichiers

```
Extension_CantonController/
├── src/
│   ├── BoosterInterface.h          # NOUVEAU
│   ├── RMT_BoosterAdapter.h/cpp    # NOUVEAU
│   ├── Old_BoosterAdapter.h/cpp    # NOUVEAU (Fallback)
│   ├── EXCC_BoosterFactory.h/cpp   # NOUVEAU
│   ├── EXCC_CanBooster.cpp         # À MODIFIER
│   ├── EXCC_Booster_WS2812.cpp    # À MODIFIER
│   └── ...
```

### Étape 2 : Créer l'Interface Abstraite

#### 2.1. BoosterInterface.h

```cpp
// src/BoosterInterface.h
#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>
#include <functional>

/**
 * @brief Interface abstraite pour le booster DCC
 * 
 * Cette interface permet d'utiliser différents types de boosters
 * (RMT, PWM, etc.) de manière interchangeable.
 */
class BoosterInterface {
public:
    virtual ~BoosterInterface() = default;

    // ============================================================
    // Initialisation
    // ============================================================
    
    /**
     * @brief Initialise le booster
     */
    virtual void begin() = 0;

    // ============================================================
    // Génération DCC
    // ============================================================
    
    /**
     * @brief Applique un paquet DCC
     * @param data Pointeur vers les données DCC
     * @param len Longueur des données
     */
    virtual void applyDcc(const uint8_t* data, uint8_t len) = 0;

    // ============================================================
    // RailCom
    // ============================================================
    
    /**
     * @brief Active le cutout RailCom
     */
    virtual void enableCutout() = 0;

    /**
     * @brief Désactive le cutout RailCom
     */
    virtual void disableCutout() = 0;

    /**
     * @brief Indique si le cutout est actif
     * @return true si le cutout est actif
     */
    virtual bool isCutoutActive() const = 0;

    // ============================================================
    // Contrôle de Sortie
    // ============================================================
    
    /**
     * @brief Active la sortie DCC
     */
    virtual void enableOutput() = 0;

    /**
     * @brief Désactive la sortie DCC
     */
    virtual void disableOutput() = 0;

    /**
     * @brief Indique si la sortie est active
     * @return true si la sortie est active
     */
    virtual bool isOutputEnabled() const = 0;

    // ============================================================
    // Télémétrie
    // ============================================================
    
    /**
     * @brief Lit le courant en milliampères
     * @return Courant en mA
     */
    virtual uint16_t readCurrent_mA() = 0;

    /**
     * @brief Lit la tension en millivolts
     * @return Tension en mV
     */
    virtual uint16_t readVoltage_mV() = 0;

    /**
     * @brief Lit un échantillon RailCom
     * @return Valeur de l'échantillon (16 bits)
     */
    virtual int16_t readRailcomSample() = 0;

    // ============================================================
    // Défauts
    // ============================================================
    
    /**
     * @brief Indique si un défaut est actif
     * @return true si un défaut est actif
     */
    virtual bool isFaultActive() = 0;

    /**
     * @brief Vérifie si c'est un court-circuit
     * @return true si court-circuit
     */
    virtual bool isShortCircuit() = 0;

    /**
     * @brief Vérifie si c'est une sous-tension
     * @return true si sous-tension
     */
    virtual bool isUndervoltage() = 0;

    /**
     * @brief Vérifie si c'est une surchauffe
     * @return true si surchauffe
     */
    virtual bool isThermalFault() = 0;

    // ============================================================
    // Communication CAN
    // ============================================================
    
    /**
     * @brief Traite un message CAN reçu
     * @param id Identifiant CAN
     * @param data Données
     * @param len Longueur des données
     */
    virtual void onCanMessage(uint32_t id, const uint8_t* data, uint8_t len) = 0;

    // ============================================================
    // Mise à jour
    // ============================================================
    
    /**
     * @brief Mise à jour périodique (appelée toutes les 1 ms)
     */
    virtual void update() = 0;

    // ============================================================
    // Timeouts
    // ============================================================
    
    /**
     * @brief Obtient le timestamp de la dernière réception CAN
     * @return Timestamp en ms
     */
    virtual uint32_t lastCanRxMs() const = 0;

    /**
     * @brief Obtient le timestamp du dernier cutout
     * @return Timestamp en ms
     */
    virtual uint32_t lastCutoutMs() const = 0;
};
```

### Étape 3 : Créer l'Adaptateur RMT

#### 3.1. RMT_BoosterAdapter.h

```cpp
// src/RMT_BoosterAdapter.h
#pragma once

#include "BoosterInterface.h"

// Forward declaration (à vérifier dans DCC_CAN_RMT-Booster)
class DCC_RMT_Booster;

/**
 * @brief Adaptateur pour le DCC_CAN_RMT-Booster
 * 
 * Implémente BoosterInterface en utilisant DCC_CAN_RMT_Booster.
 */
class RMT_BoosterAdapter : public BoosterInterface {
public:
    RMT_BoosterAdapter();
    ~RMT_BoosterAdapter() override;

    // Initialisation
    void begin() override;

    // Génération DCC
    void applyDcc(const uint8_t* data, uint8_t len) override;

    // RailCom
    void enableCutout() override;
    void disableCutout() override;
    bool isCutoutActive() const override;

    // Contrôle de sortie
    void enableOutput() override;
    void disableOutput() override;
    bool isOutputEnabled() const override;

    // Télémétrie
    uint16_t readCurrent_mA() override;
    uint16_t readVoltage_mV() override;
    int16_t readRailcomSample() override;

    // Défauts
    bool isFaultActive() override;
    bool isShortCircuit() override;
    bool isUndervoltage() override;
    bool isThermalFault() override;

    // Communication CAN
    void onCanMessage(uint32_t id, const uint8_t* data, uint8_t len) override;

    // Mise à jour
    void update() override;

    // Timeouts
    uint32_t lastCanRxMs() const override;
    uint32_t lastCutoutMs() const override;

private:
    DCC_RMT_Booster* booster;
    bool outputEnabled;
    bool cutoutActive;
    uint32_t lastCanRxTimestamp;
    uint32_t lastCutoutTimestamp;
};
```

#### 3.2. RMT_BoosterAdapter.cpp

```cpp
// src/RMT_BoosterAdapter.cpp
#include "RMT_BoosterAdapter.h"
#include "DCC_CAN_RMT_Booster.h"  // À adapter selon le vrai chemin

RMT_BoosterAdapter::RMT_BoosterAdapter() 
    : booster(nullptr), 
      outputEnabled(false), 
      cutoutActive(false),
      lastCanRxTimestamp(0),
      lastCutoutTimestamp(0)
{
    booster = new DCC_RMT_Booster();
}

RMT_BoosterAdapter::~RMT_BoosterAdapter() {
    if (booster) {
        delete booster;
    }
}

void RMT_BoosterAdapter::begin() {
    booster->begin();
    outputEnabled = false;
    cutoutActive = false;
}

void RMT_BoosterAdapter::applyDcc(const uint8_t* data, uint8_t len) {
    booster->setDccPacket(data, len);
}

void RMT_BoosterAdapter::enableCutout() {
    booster->setCutout(true);
    cutoutActive = true;
    lastCutoutTimestamp = millis();
}

void RMT_BoosterAdapter::disableCutout() {
    booster->setCutout(false);
    cutoutActive = false;
    lastCutoutTimestamp = millis();
}

bool RMT_BoosterAdapter::isCutoutActive() const {
    return cutoutActive;
}

void RMT_BoosterAdapter::enableOutput() {
    booster->enableOutput();
    outputEnabled = true;
}

void RMT_BoosterAdapter::disableOutput() {
    booster->disableOutput();
    outputEnabled = false;
}

bool RMT_BoosterAdapter::isOutputEnabled() const {
    return outputEnabled && booster->isOutputEnabled();
}

uint16_t RMT_BoosterAdapter::readCurrent_mA() {
    return booster->readCurrent_mA();
}

uint16_t RMT_BoosterAdapter::readVoltage_mV() {
    return booster->readVoltage_mV();
}

int16_t RMT_BoosterAdapter::readRailcomSample() {
    // À adapter selon l'API du nouveau booster
    // Si le booster gère RailCom directement :
    return booster->readRailcomSample();
    
    // Sinon, implémenter un échantillonnage logiciel
    // (solution de fallback)
    return 0;
}

bool RMT_BoosterAdapter::isFaultActive() {
    return booster->isFaultActive();
}

bool RMT_BoosterAdapter::isShortCircuit() {
    return booster->isOvercurrent();
}

bool RMT_BoosterAdapter::isUndervoltage() {
    return booster->isUndervoltage();
}

bool RMT_BoosterAdapter::isThermalFault() {
    return booster->isThermalFault();
}

void RMT_BoosterAdapter::onCanMessage(uint32_t id, const uint8_t* data, uint8_t len) {
    // Transmettre le message au booster
    // Le booster peut gérer certains messages directement
    
    // Message de cutout RailCom
    if (id == 0x101 && len >= 1) {
        if (data[0] == 1) {
            enableCutout();
        } else {
            disableCutout();
        }
        lastCanRxTimestamp = millis();
    }
    
    // Autres messages...
    booster->onCanMessage(id, data, len);
    lastCanRxTimestamp = millis();
}

void RMT_BoosterAdapter::update() {
    // Mise à jour du booster
    booster->update();
    
    // Vérifier les défauts
    if (isFaultActive()) {
        disableOutput();
    }
}

uint32_t RMT_BoosterAdapter::lastCanRxMs() const {
    return lastCanRxTimestamp;
}

uint32_t RMT_BoosterAdapter::lastCutoutMs() const {
    return lastCutoutTimestamp;
}
```

### Étape 4 : Créer le Factory

#### 4.1. EXCC_BoosterFactory.h

```cpp
// src/EXCC_BoosterFactory.h
#pragma once

#include "BoosterInterface.h"

/**
 * @brief Factory pour créer des instances de booster
 */
namespace EXCC_BoosterFactory {
    
    /**
     * @brief Type de booster disponible
     */
    enum class BoosterType {
        AUTO,           // Détection automatique
        RMT,            // DCC_CAN_RMT-Booster
        OLD,            // CAN_DCC-Booster (ancien)
    };
    
    /**
     * @brief Crée une instance de booster
     * @param type Type de booster
     * @return Pointeur vers BoosterInterface
     */
    BoosterInterface* createBooster(BoosterType type = BoosterType::AUTO);
    
    /**
     * @brief Détruit une instance de booster
     * @param booster Pointeur vers BoosterInterface
     */
    void destroyBooster(BoosterInterface* booster);
    
};
```

#### 4.2. EXCC_BoosterFactory.cpp

```cpp
// src/EXCC_BoosterFactory.cpp
#include "EXCC_BoosterFactory.h"
#include "RMT_BoosterAdapter.h"
#include "Old_BoosterAdapter.h"  // À créer

// Définition du type de booster par défaut
#ifndef EXCC_BOOSTER_TYPE
#define EXCC_BOOSTER_TYPE EXCC_BoosterFactory::BoosterType::AUTO
#endif

BoosterInterface* EXCC_BoosterFactory::createBooster(BoosterType type) {
    if (type == BoosterType::AUTO) {
        #if defined(EXCC_USE_RMT_BOOSTER)
            type = BoosterType::RMT;
        #else
            type = BoosterType::OLD;
        #endif
    }
    
    switch (type) {
        case BoosterType::RMT:
            return new RMT_BoosterAdapter();
        case BoosterType::OLD:
            return new Old_BoosterAdapter();
        default:
            return new RMT_BoosterAdapter();  // Par défaut
    }
}

void EXCC_BoosterFactory::destroyBooster(BoosterInterface* booster) {
    if (booster) {
        delete booster;
    }
}
```

### Étape 5 : Adapter EXCC_CanBooster

#### 5.1. Modifier EXCC_CanBooster.h

```cpp
// src/EXCC_CanBooster.h
#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>
#include "EXCC_Config.h"
#include "BoosterInterface.h"  // NOUVEAU
#include "CanMsg.h"

/*
 * ============================================================================
 *  EXCC_CanBooster.h
 * ============================================================================
 *
 *  Rôle :
 *  ------
 *  Cette classe gère la communication CAN du Booster EXCC.
 *
 *  Modifications pour DCC_CAN_RMT-Booster :
 *  - Utilise maintenant BoosterInterface au lieu de CanDccBoosterHardware
 *  - Compatible avec les deux types de boosters
 */

class EXCC_CanBooster
{
public:
    /*
     * ------------------------------------------------------------------------
     *  begin()
     * ------------------------------------------------------------------------
     *  Initialise le bus CAN Booster et le booster lui-même.
     *  
     *  @param boosterType Type de booster à utiliser
     */
    static void begin(BoosterInterface* booster = nullptr);

    /*
     * ------------------------------------------------------------------------
     *  process()
     * ------------------------------------------------------------------------
     *  Lit toutes les trames CAN disponibles.
     *  Appelée toutes les 1 ms par EXCC_BoosterCore.
     */
    static void process();

    /*
     * ------------------------------------------------------------------------
     *  isCutoutActive()
     * ------------------------------------------------------------------------
     *  Indique si la fenêtre RailCom est active.
     */
    static bool isCutoutActive();

    /*
     * ------------------------------------------------------------------------
     *  lastCanRxMs()
     * ------------------------------------------------------------------------
     *  Timestamp de la dernière trame CAN reçue.
     */
    static uint32_t lastCanRxMs();

    /*
     * ------------------------------------------------------------------------
     *  lastCutoutMs()
     * ------------------------------------------------------------------------
     *  Timestamp du dernier cutout (début ou fin).
     */
    static uint32_t lastCutoutMs();

    /*
     * ------------------------------------------------------------------------
     *  setBooster()
     * ------------------------------------------------------------------------
     *  Définit le booster à utiliser.
     */
    static void setBooster(BoosterInterface* booster);

    /*
     * ------------------------------------------------------------------------
     *  getBooster()
     * ------------------------------------------------------------------------
     *  Obtient le booster actuel.
     */
    static BoosterInterface* getBooster();

private:
    /*
     * ------------------------------------------------------------------------
     *  handleFrame()
     * ------------------------------------------------------------------------
     *  Analyse une trame CAN Booster :
     *      - détecte le cutout (ID 0x101)
     *      - appelle les hooks RailCom du booster
     *      - transmet la trame au booster (onCanMessage)
     */
    static void handleFrame(const CanMsg &msg);

private:
    static BoosterInterface* s_booster;  // NOUVEAU
};
```

#### 5.2. Modifier EXCC_CanBooster.cpp

```cpp
// src/EXCC_CanBooster.cpp
#include "EXCC_CanBooster.h"
#include "CanBus.h"
#include "Protocol.h"

BoosterInterface* EXCC_CanBooster::s_booster = nullptr;

void EXCC_CanBooster::begin(BoosterInterface* booster) {
    // Initialiser le bus CAN
    // (code existant)
    
    // Définir le booster
    setBooster(booster);
    
    if (s_booster) {
        s_booster->begin();
    }
}

void EXCC_CanBooster::setBooster(BoosterInterface* booster) {
    s_booster = booster;
}

BoosterInterface* EXCC_CanBooster::getBooster() {
    return s_booster;
}

void EXCC_CanBooster::process() {
    // Lire les messages CAN (code existant)
    CanMsg msg;
    while (CanBus::bus(1).receive(msg)) {
        handleFrame(msg);
    }
    
    // Mettre à jour le booster
    if (s_booster) {
        s_booster->update();
    }
}

bool EXCC_CanBooster::isCutoutActive() {
    if (s_booster) {
        return s_booster->isCutoutActive();
    }
    return false;
}

uint32_t EXCC_CanBooster::lastCanRxMs() {
    if (s_booster) {
        return s_booster->lastCanRxMs();
    }
    return 0;
}

uint32_t EXCC_CanBooster::lastCutoutMs() {
    if (s_booster) {
        return s_booster->lastCutoutMs();
    }
    return 0;
}

void EXCC_CanBooster::handleFrame(const CanMsg &msg) {
    if (s_booster) {
        s_booster->onCanMessage(msg.id, msg.data, msg.dlc);
    }
}
```

### Étape 6 : Adapter EXCC_Booster_WS2812

#### 6.1. Modifier EXCC_Booster_WS2812.h

```cpp
// src/EXCC_Booster_WS2812.h
#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>
#include "EXCC_Config.h"
#include "BoosterInterface.h"  // NOUVEAU

class BoosterTelemetry;

/*
 * ============================================================================
 *  EXCC_Booster_WS2812
 * ============================================================================
 *
 *  Modifications pour DCC_CAN_RMT-Booster :
 *  - Utilise maintenant BoosterInterface au lieu de communiquer directement
 *    avec le hardware
 */

class EXCC_Booster_WS2812
{
public:
    EXCC_Booster_WS2812();

    // Initialisation
    void begin();

    // Mise à jour périodique
    void update();

    // Activation / désactivation
    void setEnabled(bool enabled);

    // Messages CAN
    void onCanMessage(uint32_t id, const uint8_t *data, uint8_t len);

    // Hooks RailCom
    void onCutoutStart();
    void onCutoutEnd();
    void feedRailcomSample();

    // Accès aux mesures
    uint16_t readCurrent_mA() const;
    uint16_t readVoltage_mV() const;
    bool isThermalFault() const;

    // Accès direct à la télémétrie
    const BoosterTelemetry& getTelemetry() const;

    // Utilisé par EXCC_StatusLed
    bool isEnabled() const { return m_enabled; }

    // NOUVEAU : Définir le booster
    void setBooster(BoosterInterface* booster);

private:
    void updateOccupation(uint16_t courant_mA);

private:
    bool m_enabled;
    BoosterInterface* m_booster;  // NOUVEAU
    
    uint16_t m_current_mA;
    uint16_t m_voltage_mV;
    bool m_faultThermal;
};
```

#### 6.2. Modifier EXCC_Booster_WS2812.cpp

```cpp
// src/EXCC_Booster_WS2812.cpp
#include "EXCC_Booster_WS2812.h"
#include "EXCC_Config.h"

EXCC_Booster_WS2812::EXCC_Booster_WS2812() 
    : m_enabled(false), 
      m_booster(nullptr),
      m_current_mA(0),
      m_voltage_mV(0),
      m_faultThermal(false)
{
}

void EXCC_Booster_WS2812::begin() {
    m_enabled = true;
}

void EXCC_Booster_WS2812::setBooster(BoosterInterface* booster) {
    m_booster = booster;
}

void EXCC_Booster_WS2812::update() {
    if (!m_booster) return;
    
    // Lire les valeurs de télémétrie
    m_current_mA = m_booster->readCurrent_mA();
    m_voltage_mV = m_booster->readVoltage_mV();
    m_faultThermal = m_booster->isThermalFault();
    
    // Mettre à jour l'occupation (si toujours géré ici)
    updateOccupation(m_current_mA);
    
    // Gérer le RailCom si nécessaire
    if (m_booster->isCutoutActive()) {
        feedRailcomSample();
    }
}

void EXCC_Booster_WS2812::setEnabled(bool enabled) {
    m_enabled = enabled;
    if (m_booster) {
        if (enabled) {
            m_booster->enableOutput();
        } else {
            m_booster->disableOutput();
        }
    }
}

void EXCC_Booster_WS2812::onCanMessage(uint32_t id, const uint8_t *data, uint8_t len) {
    // Traiter les messages CAN spécifiques au booster
    if (m_booster) {
        m_booster->onCanMessage(id, data, len);
    }
}

void EXCC_Booster_WS2812::onCutoutStart() {
    if (m_booster) {
        m_booster->enableCutout();
    }
}

void EXCC_Booster_WS2812::onCutoutEnd() {
    if (m_booster) {
        m_booster->disableCutout();
    }
}

void EXCC_Booster_WS2812::feedRailcomSample() {
    if (m_booster) {
        int16_t sample = m_booster->readRailcomSample();
        // Traiter l'échantillon (décodage RailCom)
        // ...
    }
}

uint16_t EXCC_Booster_WS2812::readCurrent_mA() const {
    return m_current_mA;
}

uint16_t EXCC_Booster_WS2812::readVoltage_mV() const {
    return m_voltage_mV;
}

bool EXCC_Booster_WS2812::isThermalFault() const {
    return m_faultThermal;
}

const BoosterTelemetry& EXCC_Booster_WS2812::getTelemetry() const {
    // Retourner la télémétrie
    // (implémentation à compléter)
    static BoosterTelemetry telemetry;
    return telemetry;
}
```

### Étape 7 : Adapter EXCC_Main

#### 7.1. Modifier EXCC_Main.cpp

```cpp
// src/EXCC_Main.cpp
#include "EXCC_Main.h"
#include "EXCC_Config.h"
#include "EXCC_Pins.h"
#include "EXCC_Callbacks.h"
#include "EXCC_CAN_CC.h"
#include "EXCC_CanBooster.h"
#include "EXCC_Booster_WS2812.h"
#include "EXCC_BoosterFactory.h"  // NOUVEAU
#include "BoosterInterface.h"    // NOUVEAU

// ... (autres includes existants)

// Variables globales (existantes)
Adafruit_PWMServoDriver g_pca9685;
EXCC_Signaux_WS2812 signauxH;
EXCC_Signaux_WS2812 signauxAH;
EXCC_LedDirection_WS2812 directionH_WS;
EXCC_LedDirection_WS2812 directionAH_WS;
EXCC_Canton_WS2812 cantonWS;
EXCC_Quadrature quadH;
EXCC_Quadrature quadAH;
EXCC_Booster_WS2812 booster;  // EXISTANT

// NOUVEAU : Variable globale pour le booster
BoosterInterface* g_booster = nullptr;

namespace EXCC_Main
{
    void begin() noexcept
    {
        // Initialisation du PCA9685 (existant)
        g_pca9685.begin();
        g_pca9685.setPWMFreq(50);  // 50 Hz pour les servos
        
        // Initialisation des objets (existant)
        signauxH.begin();
        signauxAH.begin();
        directionH_WS.begin();
        directionAH_WS.begin();
        cantonWS.begin();
        quadH.begin();
        quadAH.begin();
        
        // NOUVEAU : Créer le booster
        #ifdef EXCC_USE_RMT_BOOSTER
            g_booster = EXCC_BoosterFactory::createBooster(
                EXCC_BoosterFactory::BoosterType::RMT);
        #else
            g_booster = EXCC_BoosterFactory::createBooster(
                EXCC_BoosterFactory::BoosterType::OLD);
        #endif
        
        // NOUVEAU : Initialiser le booster
        booster.setBooster(g_booster);
        
        // Initialisation CAN (existant)
        // ...
        
        // NOUVEAU : Initialiser EXCC_CanBooster avec le booster
        EXCC_CanBooster::begin(g_booster);
        
        // Démarrer la tâche booster si nécessaire (existant)
        EXCC_BoosterCore::startTask();
    }

    void loop() noexcept
    {
        // Boucle principale existante
        quadH.process();
        quadAH.process();
        
        // Traitement CAN
        EXCC_CanCommands::process();
        
        // NOUVEAU : Mise à jour du booster
        if (g_booster) {
            g_booster->update();
        }
        
        // Mise à jour des signaux
        signauxH.update();
        signauxAH.update();
        directionH_WS.update();
        directionAH_WS.update();
        cantonWS.update();
        
        // ... (autre code existant)
    }
}
```

### Étape 8 : Créer le Fallback (Old_BoosterAdapter)

Pour maintenir la compatibilité avec l'ancien booster :

#### 8.1. Old_BoosterAdapter.h

```cpp
// src/Old_BoosterAdapter.h
#pragma once

#include "BoosterInterface.h"
#include "CanDccBoosterHardware.h"

/**
 * @brief Adaptateur pour l'ancien CAN_DCC-Booster
 * 
 * Permet d'utiliser l'ancien booster via la nouvelle interface.
 */
class Old_BoosterAdapter : public BoosterInterface, public CanDccBoosterHardware {
public:
    Old_BoosterAdapter();
    ~Old_BoosterAdapter() override;

    // Initialisation
    void begin() override;

    // Génération DCC
    void applyDcc(const uint8_t* data, uint8_t len) override;

    // RailCom
    void enableCutout() override;
    void disableCutout() override;
    bool isCutoutActive() const override;

    // Contrôle de sortie
    void enableOutput() override;
    void disableOutput() override;
    bool isOutputEnabled() const override;

    // Télémétrie
    uint16_t readCurrent_mA() override;
    uint16_t readVoltage_mV() override;
    int16_t readRailcomSample() override;

    // Défauts
    bool isFaultActive() override;
    bool isShortCircuit() override;
    bool isUndervoltage() override;
    bool isThermalFault() override;

    // Communication CAN
    void onCanMessage(uint32_t id, const uint8_t* data, uint8_t len) override;

    // Mise à jour
    void update() override;

    // Timeouts
    uint32_t lastCanRxMs() const override;
    uint32_t lastCutoutMs() const override;

    // Héritage de CanDccBoosterHardware
    void applyDcc(const uint8_t *data, uint8_t len) override;
    void enableCutout() override;
    void disableCutout() override;
    void enableOutput() override;
    void disableOutput() override;
    bool isFaultActive() override;
    uint16_t readCurrent_mA() override;
    uint16_t readVoltage_mV() override;
    int16_t readRailcomSample() override;

private:
    bool outputEnabled;
    bool cutoutActive;
    uint32_t lastCanRxTimestamp;
    uint32_t lastCutoutTimestamp;
};
```

---

## 🧪 Tests et Validation

### 📋 Plan de Test

| Test | Description | Critère de Réussite | Priorité |
|------|-------------|---------------------|----------|
| T1 | Génération DCC | Signal DCC mesuré avec oscilloscope | ⭐⭐⭐⭐⭐ | Haute |
| T2 | Cutout RailCom | Détection du cutout et échantillonnage | ⭐⭐⭐⭐ | Haute |
| T3 | Télémétrie | Lecture courant/tension précise | ⭐⭐⭐⭐ | Haute |
| T4 | Communication CAN | Échange de messages avec CC | ⭐⭐⭐⭐⭐ | Haute |
| T5 | Protection | Détection court-circuit/sous-tension | ⭐⭐⭐⭐ | Haute |
| T6 | RailCom | Décodage correct des adresses | ⭐⭐⭐⭐ | Haute |
| T7 | Compatibilité | Fonctionnement avec l'ancien booster | ⭐⭐⭐ | Moyenne |
| T8 | Performance | Pas de jitter, latence < 1ms | ⭐⭐⭐⭐ | Moyenne |
| T9 | Consommation | CPU < 50%, mémoire < 80% | ⭐⭐⭐ | Moyenne |
| T10 | Stress | Test 24h sans erreur | ⭐⭐⭐ | Moyenne |

### 🔧 Outils de Test

1. **Oscilloscope** : Pour vérifier le signal DCC
2. **Analyseur CAN** : Pour vérifier les messages CAN
3. **Multimètre** : Pour vérifier courant/tension
4. **Analyseur logique** : Pour le RailCom
5. **Tests unitaires** : Avec PlatformIO

### 📝 Procédures de Test

#### Test T1 : Génération DCC

```
1. Brancher un oscilloscope sur la sortie DCC
2. Envoyer une commande DCC (ex: vitesse loco)
3. Mesurer :
   - Période du signal (devrait être ~20ms pour DCC)
   - Largeur des impulsions
   - Jitter (devrait être < 1µs)
4. Vérifier avec plusieurs décodeurs DCC
```

#### Test T2 : Cutout RailCom

```
1. Activer le cutout via message CAN (ID 0x101, data[0]=1)
2. Vérifier que le signal DCC est coupé pendant ~6ms
3. Vérifier que l'échantillonnage RailCom est actif
4. Désactiver le cutout (ID 0x101, data[0]=0)
5. Vérifier que le signal DCC est restauré
```

#### Test T3 : Télémétrie

```
1. Mesurer le courant avec un multimètre
2. Comparer avec readCurrent_mA()
3. Erreur acceptable : < 5%
4. Répéter pour la tension
```

#### Test T4 : Communication CAN

```
1. Envoyer un message PING (0xFC) depuis CC
2. Vérifier que EXCC répond avec PONG (0xD1)
3. Vérifier que l'ID du canton est correct
4. Tester tous les opcodes
```

---

## ⚠️ Points d'Attention

### 🔴 Risques Critiques

1. **Timing RailCom**
   - Le nouveau booster peut avoir un timing différent
   - Vérifier la compatibilité avec les décodeurs RailCom existants
   - Adapter le timing si nécessaire

2. **Génération DCC**
   - Le RMT génère des signaux très précis
   - Vérifier que les décodeurs DCC acceptent cette précision
   - Certains décodeurs anciens peuvent être sensibles

3. **Consommation Mémoire**
   - Le RMT utilise des buffers spécifiques
   - Vérifier que la mémoire est suffisante
   - Adapter la taille des buffers si nécessaire

4. **Priorités d'Interruption**
   - Le RMT peut nécessiter des priorités d'interruption spécifiques
   - Vérifier la compatibilité avec FreeRTOS
   - Adapter les priorités si nécessaire

### 🟡 Risques Moyens

5. **Compatibilité CAN**
   - Vérifier que les opcodes sont identiques
   - Adapter les opcodes si nécessaire
   - Créer une couche de traduction si besoin

6. **Configuration Matérielle**
   - Le RMT utilise des GPIO spécifiques
   - Vérifier que les broches sont compatibles
   - Adapter EXCC_Pins.h si nécessaire

7. **Gestion des Erreurs**
   - Le nouveau booster peut avoir des mécanismes d'erreur différents
   - Adapter la gestion des erreurs
   - Vérifier la compatibilité avec EXCC_Occupation

8. **Documentation**
   - Le DCC_CAN_RMT-Booster peut avoir une documentation limitée
   - Analyser le code source en détail
   - Poser des questions au mainteneur (prune74)

### 🟢 Risques Faibles

9. **Performance**
   - Le RMT devrait améliorer les performances
   - Vérifier que le gain est significatif
   - Adapter si nécessaire

10. **Maintenance**
    - Le nouveau booster peut être mieux maintenu
    - Vérifier la fréquence des mises à jour
    - S'abonner aux notifications

---

## 📊 Checklist de Migration

### ✅ Phase 1 : Préparation

- [ ] Sauvegarder le projet actuel
- [ ] Créer une branche de développement
- [ ] Cloner le dépôt DCC_CAN_RMT-Booster
- [ ] Analyser le code source du nouveau booster
- [ ] Documenter l'API du nouveau booster
- [ ] Identifier les différences majeures
- [ ] Créer un plan de migration détaillé

### ✅ Phase 2 : Développement

- [ ] Créer BoosterInterface.h
- [ ] Créer RMT_BoosterAdapter.h/cpp
- [ ] Créer Old_BoosterAdapter.h/cpp
- [ ] Créer EXCC_BoosterFactory.h/cpp
- [ ] Modifier EXCC_CanBooster.h/cpp
- [ ] Modifier EXCC_Booster_WS2812.h/cpp
- [ ] Modifier EXCC_Main.cpp
- [ ] Adapter platformio.ini
- [ ] Vérifier tous les includes
- [ ] Corriger les erreurs de compilation

### ✅ Phase 3 : Intégration

- [ ] Compiler avec l'ancien booster (fallback)
- [ ] Tester avec l'ancien booster
- [ ] Compiler avec le nouveau booster
- [ ] Tester avec le nouveau booster
- [ ] Vérifier la communication CAN
- [ ] Vérifier la génération DCC
- [ ] Vérifier le RailCom
- [ ] Vérifier la télémétrie

### ✅ Phase 4 : Validation

- [ ] Test T1 : Génération DCC
- [ ] Test T2 : Cutout RailCom
- [ ] Test T3 : Télémétrie
- [ ] Test T4 : Communication CAN
- [ ] Test T5 : Protection
- [ ] Test T6 : RailCom
- [ ] Test T7 : Compatibilité
- [ ] Test T8 : Performance
- [ ] Test T9 : Consommation
- [ ] Test T10 : Stress

### ✅ Phase 5 : Documentation

- [ ] Mettre à jour le README.md
- [ ] Documenter les changements
- [ ] Créer un guide de migration
- [ ] Mettre à jour les diagrammes
- [ ] Documenter les nouvelles fonctionnalités
- [ ] Documenter les points d'attention

### ✅ Phase 6 : Déploiement

- [ ] Valider avec les utilisateurs bêta
- [ ] Corriger les bugs identifiés
- [ ] Finaliser la documentation
- [ ] Publier la nouvelle version
- [ ] Annoncer la migration
- [ ] Former les utilisateurs

---

## 💡 Bonnes Pratiques

### 📝 Bonne Pratique 1 : Tests Incrementaux

```
1. Tester chaque module séparément
2. Tester l'intégration deux à deux
3. Tester le système complet
4. Tester en conditions réelles
```

### 📝 Bonne Pratique 2 : Versioning

```
- Utiliser des tags Git pour chaque version
- Maintenir un changelog
- Documenter les breaking changes
- Fournir des instructions de migration
```

### 📝 Bonne Pratique 3 : Logging

```cpp
// Ajouter du logging détaillé pendant la migration
#define EXCC_MIGRATION_LOG true

#if EXCC_MIGRATION_LOG
#define MIGRATION_LOG(msg) Serial.println(msg)
#else
#define MIGRATION_LOG(msg)
#endif

// Exemple d'utilisation
MIGRATION_LOG("[MIGRATION] Initialisation booster RMT");
```

### 📝 Bonne Pratique 4 : Fallback

```cpp
// Toujours maintenir une solution de fallback
#ifdef EXCC_USE_RMT_BOOSTER
    // Utiliser le nouveau booster
#else
    // Utiliser l'ancien booster
#endif
```

### 📝 Bonne Pratique 5 : Documentation

```markdown
// Documenter chaque changement
## Changements pour la migration vers DCC_CAN_RMT-Booster

### 1. Architecture
- Ajout de BoosterInterface
- Ajout de RMT_BoosterAdapter
- Modification de EXCC_CanBooster

### 2. Configuration
- Nouvelle option : EXCC_USE_RMT_BOOSTER
- Modifier platformio.ini pour inclure DCC_CAN_RMT-Booster

### 3. Compatibilité
- Maintenu compatible avec l'ancien booster
- Voir Old_BoosterAdapter pour le fallback
```

---

## 📚 Annexes

### 🔗 Liens Utiles

- [DCC_CAN_RMT-Booster GitHub](https://github.com/prune74/DCC_CAN_RMT-Booster)
- [CAN_DCC-Booster GitHub](https://github.com/prune74/CAN_DCC-Booster)
- [ACAN_ESP32 GitHub](https://github.com/pierremolinaro/ACAN_ESP32)
- [FastLED GitHub](https://github.com/FastLED/FastLED)
- [PlatformIO](https://platformio.org/)

### 📖 Documentation Technique

- [ESP32 RMT Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/rmt.html)
- [CAN 2.0B Specification](https://www.bosch.com/content/language2/html/22868.htm)
- [DCC Standard (NMRA)](https://www.nmra.org/standards)
- [RailCom Standard](https://www.railcom.org/)

### 🎯 Contacts

- **Mainteneur EXCC** : À définir
- **Mainteneur DCC_CAN_RMT-Booster** : prune74
- **Communauté** : Discord, Forum, GitHub Issues

---

## 🏁 Conclusion

La migration vers le **DCC_CAN_RMT-Booster** représente une **amélioration majeure** du système EXCC avec :

✅ **Meilleure qualité DCC** grâce au RMT
✅ **Moins de charge CPU** pour d'autres tâches
✅ **Meilleure précision temporelle**
✅ **Compatibilité maintenue** avec l'ancien booster
✅ **Amélioration de la fiabilité**

### 🎯 Prochaines Étapes

1. **Analyser le code source** du DCC_CAN_RMT-Booster
2. **Implémenter l'interface** BoosterInterface
3. **Créer les adaptateurs** RMT et Old
4. **Adapter les modules existants**
5. **Tester rigoureusement**
6. **Documenter** tous les changements
7. **Déployer progressivement**

### ⏱️ Estimation du Temps

| Phase | Durée | Ressources |
|-------|-------|------------|
| Analyse | 1-2 semaines | 1 développeur |
| Développement | 2-4 semaines | 1 développeur |
| Tests | 2 semaines | 1-2 développeurs |
| Documentation | 1 semaine | 1 développeur |
| **Total** | **6-9 semaines** | **1-2 développeurs** |

### 💡 Recommandation Finale

**Commencer la migration dès que possible** pour bénéficier des améliorations du RMT, tout en maintenant la compatibilité avec l'ancien booster via le fallback.

---

*Document généré par Mistral Vibe - 11 novembre 2025*
*Version : 1.0 - Guide d'Adaptation au DCC_CAN_RMT-Booster*
