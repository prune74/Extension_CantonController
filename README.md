# EXCC — Gestion Canton 2026  
Firmware du module EXCC (Extension Satellite Autonome)  
Architecture modulaire, non bloquante, orientée protocole CC ↔ EXCC.

---

## 📌 Présentation générale

Le module **EXCC** est un satellite autonome chargé de gérer :

- la signalisation SNCF (aspects, transitions, clignotements)
- les aiguilles (servos + micro-switchs)
- les LED directionnelles
- le comptage essieux (quadrature)
- le canton (LEDs occupation + mouvement)
- la communication RS485 avec le CC
- le **Booster DCC** (si présent) : courant, tension, cutout RailCom, occupation

L’architecture Gestion Canton 2026 sépare clairement :

| Domaine | Module |
|--------|--------|
| Logique locale | `EXCC_Runtime` |
| Initialisation | `EXCC_System` |
| Protocole RS485 | `EXCC_UartRx` / `EXCC_UartTx` |
| Booster | `EXCC_BoosterCore` |
| Calibration | `EXCC_Calibration` |
| Signaux | `EXCC_Signaux` |
| Servos | `EXCC_Servo` |
| Canton | `EXCC_Canton` |

---

## 🧩 Architecture générale

```text
EXCC_Main
├── EXCC_System      → Initialisation complète
├── EXCC_Runtime     → Boucle temps réel (hors booster)
├── EXCC_UartRx      → Parsing trames CC → EXCC
├── EXCC_UartTx      → Envoi trames EXCC → CC
├── EXCC_BoosterCore → Tâche FreeRTOS (DCC, courant, RailCom)
├── EXCC_Calibration → Machine à états non bloquante
├── EXCC_Signaux     → Gestion aspects SNCF
├── EXCC_Servo       → Servos + anti-blocage
├── EXCC_Canton      → LEDs canton + mouvement
└── EXCC_Quadrature  → Comptage essieux
```
---

## 🔌 Protocole CC ↔ EXCC (RS485)

Toutes les trames suivent le format :

[0xAA][OPCODE][DATA...]

### 🟦 CC → EXCC

| Opcode | Fonction |
|--------|----------|
| `32` | PING |
| `E4` | Topologie CAN |
| `E5` | Config signaux |
| `E6` | Aspect horaire |
| `E7` | Aspect antihoraire |
| `E8` | Direction horaire |
| `E9` | Direction antihoraire |
| `EA` | **Occupation voisins** |
| `F0` | **Servo move** |
| `F1` | **Servo config** |
| `F2` | **Servo test** |
| `F3` | **Recalibration Booster** |
| `F4` | **Application seuils calibrés** |
| `F5` | **Booster ON/OFF** |

### 🟩 EXCC → CC

| Opcode | Fonction |
|--------|----------|
| `03` | Ponctuel |
| `04` | Occupation |
| `05` | Delta axe |
| `06` | Position aiguille |
| `07` | Booster (courant, tension, état) |
| `08` | **RailCom adresse** |
| `09` | **Seuils calibrés (libre/occupé)** |

---

## ⚡ Calibration Booster (Gestion Canton 2026)

La calibration du courant est désormais :

- **non bloquante**
- **gérée par EXCC_BoosterCore**
- **pilotée par le CC**
- **persistante via JSON côté CC**

### 🔁 Cycle complet

1. **CC → EXCC : F3**  
   Demande de recalibration automatique.

2. **EXCC → CC : 09**  
   Envoi des seuils calibrés :  
   - seuilLibre  
   - seuilOccupe  

3. **CC stocke dans settings.json**

4. **CC → EXCC : F4**  
   Restaure les seuils au démarrage.

5. **EXCC_Calibration::setSeuils()**  
   Application immédiate.

---

## 🏗️ EXCC_System (initialisation)

- Lecture DIP (horaire / booster)
- Initialisation :
  - PCA9685
  - MCP23017
  - Quadrature (ISR + queue)
  - Signaux
  - Canton
  - Servos
- Démarrage tâche BoosterCore (si présent)
- Initialisation EXCC_Calibration

---

## 🔄 EXCC_Runtime (boucle temps réel)

- Lecture quadrature (queue FreeRTOS)
- Gestion essieux (delta + ponctuel)
- Mise à jour signaux SNCF
- Mise à jour canton (LEDs + mouvement)
- Anti-blocage servos
- **Aucune gestion booster ici**

---

## 🧪 EXCC_Calibration

Machine à états non bloquante :

- IDLE  
- SAMPLE_LIBRE  
- SAMPLE_OCCUPE  
- COMPUTE  
- SEND_RESULT (trame 09)

---

## 📁 Organisation du code

```text
src/
├── EXCC_Main.*
├── EXCC_System.*
├── EXCC_Runtime.*
├── EXCC_UartRx.*
├── EXCC_UartTx.*
├── EXCC_BoosterCore.*
├── EXCC_Calibration.*
├── EXCC_Signaux.*
├── EXCC_Servo.*
├── EXCC_Canton.*
├── EXCC_Quadrature.*
├── EXCC_Switches.*
└── Protocol.h
```
---

## 📝 Licence

Projet personnel — usage libre pour développement, test et documentation.

---

## 👤 Auteur

Développé par **Bruno**  
Architecture Gestion Canton 2026 — modules EXCC / CC / Booster# Extension_CantonController
