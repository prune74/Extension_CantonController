# Plan de migration vers un booster externe à deux ESP32

**Date :** 13 novembre 2026  
**Statut :** intégration logicielle UART réalisée et builds EXCC/Booster réussis; essais matériels et conception PCB reportés.

## 1. Objectif

Séparer le traitement temps réel du booster DCC des fonctions de l’EXCC, sans perdre les fonctions de canton, signaux, quadrature, compteur d’essieux, servos, switches, LEDs ni communication avec le CantonController (CC).

La carte support finale regroupe deux DevKit ESP32 au format 30 broches, utilisés comme deux contrôleurs indépendants. Le module retenu est l’ESP32-WROOM-32 :

- **DevKit ESP32 EXCC** : logique canton, signaux, servos, capteurs, WS2812 et liaison CAN de commande avec le CC.
- **DevKit ESP32 Booster** : réception des trames DCC CAN, régénération DCC, DRV8874, RailCom et mesures/protections du booster.

Le DCC ne passe pas par l’UART interprocesseur. Il reste sur son bus CAN dédié entre le convertisseur et le contrôleur booster.

## 2. Architecture firmware cible retenue

```text
                         CAN DCC dédié
LaBox → Hub → DCC_CAN_RMT-Booster ─────────→ TWAI de l’ESP32 Booster
                                                   │
                                                   ├─ RMT DCC → DRV8874 → voie
                                                   ├─ RailCom
                                                   └─ courant / tension / protections

LaBox ↔ CAN général ↔ CantonController
                         │
                         └─ CAN Command (CC MCP2515 ↔ EXCC TWAI)
                                                   │
ESP32 EXCC ═══════════════ UART ══════════════════ ESP32 Booster
          commandes booster →     ← télémétrie, RailCom et état
```

Les bus **CAN DCC**, **CAN général** et **CAN Command** sont des réseaux physiques distincts. La liaison entre les deux ESP32 est un UART point à point; il n’y a pas de troisième bus CAN entre eux dans cette architecture.

**Exception de sécurité retenue :** la trame 0x102 reste sur le CAN DCC pour la protection des autres boosters. La supervision propre à l’EXCC (commandes, mesures 1 kHz, état détaillé et RailCom) passe par UART.

Le dépôt du CC déclare déjà deux bus : bus 0 TWAI pour le réseau général et bus 1 MCP2515 pour EXCC ([CC_CAN_Config.cpp](https://github.com/prune74/CantonController/blob/main/src/CC_CAN_Config.cpp)). L’EXCC utilisera son TWAI pour le CAN Command. Le booster utilisera son TWAI pour le CAN DCC.

### Débits de référence relevés dans les configurations

| Réseau | Débit de référence | Source / état |
|---|---:|---|
| CAN général | 500 kbit/s | `CAN_BITRATE` du CC |
| CAN Command CC↔EXCC | 250 kbit/s | `CAN_BITRATE_MCP2515` du CC; TWAI EXCC configuré au même débit |
| CAN DCC convertisseur↔booster | 500 kbit/s | `CFG_CAN_BITRATE` du firmware booster |

Ces valeurs sont les cibles logicielles tirées des dépôts actuels. La configuration EXCC a été adaptée au CAN Command à 250 kbit/s, sur le bus 0 TWAI avec loopback désactivé.

### Bases PlatformIO actuellement constatées

| Projet | Cible PlatformIO | Framework | Dépendances CAN / WS2812 déclarées |
|---|---|---|---|
| EXCC | `esp32dev`, `espressif32@6.9.0` | Arduino | CanUniversal `3.1.1`; Ws2812Universal `1.1.0` |
| Booster | `esp32doit-devkit-v1`, `espressif32@6.9.0` | Arduino | CanUniversal `3.1.1`; Ws2812Universal `1.1.0` |

Les deux firmwares ciblent l’ESP32 classique et la même plateforme `espressif32@6.9.0`; les identifiants de carte restent distincts. Les builds de référence ont résolu les bibliothèques communes aux mêmes versions; elles sont maintenant épinglées explicitement dans les deux `platformio.ini`.

### UART interprocesseur — configuration firmware

- EXCC : UART2, TX candidat GPIO16, RX candidat GPIO17.
- Booster : UART1, TX candidat GPIO17, RX candidat GPIO16; l’UART2 du booster reste affecté au RailCom.
- Les GPIO16/17 sont configurés dans les deux firmwares; leur validation physique et le câblage TX↔RX relèvent de la phase PCB.

### Pré-bilan GPIO après séparation du booster

Brochage candidat basé sur les fichiers actuels; ce n’est pas encore un brochage PCB approuvé.

| ESP32 EXCC — fonction conservée | GPIO / ressource candidate |
|---|---|
| 7 sorties WS2812 | GPIO18, 19, 23, 25, 26, 27, 15; RMT 0–6 |
| I2C servos / switches | SDA 21, SCL 22 |
| Quadrature | H : GPIO12/14; AH : GPIO34/35 |
| CAN Command vers CC | TWAI TX4 / RX5, cible 250 kbit/s |
| UART vers Booster | UART2 TX16 / RX17 |
| Ressources libérées du booster local | GPIO2, 13, 32, 33, 36; RMT DCC canal 7 |

| ESP32 Booster — fonctions transférées depuis EXCC | GPIO / ressource de référence |
|---|---|
| CAN DCC depuis le convertisseur | TWAI TX5 / RX4, 500 kbit/s |
| DCC et DRV8874 | RMT DCC canal 5 → GPIO32/33; nSLEEP GPIO27 |
| Mesures booster | IPROPI GPIO34; VRAIL GPIO35 |
| RailCom | UART2 RX GPIO36 |
| Digipot | I2C SDA21 / SCL22 |
| LED de statut booster | GPIO26, RMT canal 7 |
| UART vers EXCC | UART1 TX17 / RX16 |

Le CAN Command EXCC↔CC utilise le TWAI existant à 250 kbit/s; le CAN DCC reste sur le TWAI du Booster. **Aucun MCP2515 supplémentaire n’est prévu sur EXCC** dans cette architecture UART. Le pré-bilan est une cible firmware et ne valide pas le routage PCB. GPIO12, GPIO15 et GPIO5 sont des broches de strap; GPIO34/35 sont des entrées uniquement. Les contraintes électriques de ces broches seront traitées à la phase PCB.

## 3. Répartition des responsabilités

| Fonction | ESP32 EXCC | ESP32 Booster |
|---|---:|---:|
| Signaux, directions et WS2812 EXCC | Oui | Non |
| Servos, switches, quadrature et compteur d’essieux | Oui | Non |
| Décision finale d’occupation du canton | Oui : calibration courant à 1 kHz et fusion courant/compteur/capteurs ponctuels | Échantillonne le courant et transmet les valeurs à EXCC par UART |
| Réception du CAN Command du CC | Oui | Non |
| Relais des commandes booster par UART | Oui | Exécute |
| Réception des blocs DCC `0x100` et marqueurs `0x101` | Non | Oui, depuis le TWAI DCC |
| RMT TX DCC, cutout et DRV8874 | Non | Oui |
| RailCom et mesures courant/tension booster | Non | Oui |
| Télémétrie booster vers le CC | Relaye sur CAN Command | Envoie à EXCC par UART |

**Dépendance confirmée :** la calibration EXCC prend actuellement un échantillon de courant par milliseconde depuis le booster (`EXCC_Calibration::process()`), puis `EXCC_Occupation::majCourant()` applique les seuils et fusionne cette source avec le compteur et les capteurs ponctuels. Cette chaîne doit être préservée; déplacer le matériel booster ne doit pas supprimer la détection d’occupation par courant.

## 4. Étapes de réalisation

### Étape 0 — Geler l’architecture et les cibles firmware

- [x] Retenir deux ESP32-WROOM-32 classiques au format DevKit 30 broches : un contrôleur EXCC et un contrôleur Booster.
- [x] Séparer physiquement les CAN DCC, CAN général et CAN Command; relier EXCC et Booster en UART, sans faire transiter le DCC par l’UART.
- [x] Relever les débits de référence des projets : CAN général 500 kbit/s, CAN Command 250 kbit/s, CAN DCC 500 kbit/s.
- [x] Retenir GPIO16/17 comme candidats UART; EXCC UART2, Booster UART1, TX/RX croisés.
- [x] Établir le pré-bilan GPIO EXCC/Booster après transfert des fonctions booster.
- [x] Garder dans EXCC la calibration courant, les seuils et la fusion finale d’occupation; transférer le courant du Booster à 1 kHz par UART.
- [x] Conserver les cibles PlatformIO actuelles (`esp32dev` pour EXCC, `esp32doit-devkit-v1` pour le Booster) et la plateforme commune `espressif32@6.9.0`.
- [x] Compiler les deux projets de référence et relever les versions réellement résolues.
- [x] Aligner et épingler les versions exactes compatibles des bibliothèques communes.
- [x] Définir le protocole UART : débit, trames, CRC, commandes, télémétrie, erreurs et délais.
- [x] Figer la configuration logicielle CAN EXCC : bus 0 en CAN Command à 250 kbit/s, loopback désactivé en exploitation, réception/émission sur le bon index.
- [x] Recompiler les deux firmwares après épinglage; les deux builds réussissent avec `CanUniversal 3.1.1` et `Ws2812Universal 1.1.0`.

**Critère de sortie :** interfaces et comportements logiciels documentés; les choix électriques et le brochage PCB ne bloquent pas le gel firmware.

### Étape 1 — Auditer les dépendances booster dans EXCC

- [x] Lister les modules et appels qui pilotent le DRV8874, le RMT DCC, le cutout, RailCom, IPROPI/VRAIL, digipot et télémétrie.
- [x] Repérer les commandes du CC liées à l’alimentation booster, aux seuils, à la calibration et au profil de voie.
- [x] Identifier les LEDs de statut qui doivent afficher l’état distant du booster.
- [x] Vérifier les dépendances entre mesure courant, calibration et détection d’occupation.
- [x] Noter les fonctions à préserver : signaux, canton, quadrature, essieux, servos, switches et échanges CC.

**Critère de sortie :** aucune fonction existante n’est supprimée sans propriétaire de remplacement clairement désigné.

**Résultat de l’audit initial (avant migration) :**

- Avant migration, le DCC/CAN, le FIFO, le minutage du cutout, le RMT TX, le DRV8874, RailCom, les mesures courant/tension et le digipot étaient pilotés localement par les modules Booster d’EXCC. Ils ont été retirés du firmware EXCC et restent dans le projet Booster.
- La calibration et l’occupation restent dans EXCC; la lecture locale du courant a été remplacée par une mesure UART récente et explicitement valide. Les seuils et la fusion d’occupation restent côté EXCC.
- Les commandes de puissance et de profil provenant du CC sont relayées par UART. Recalibration et seuils restent dans EXCC. Les LEDs d’état Booster restent dans EXCC et affichent l’état distant.
- Le Booster transmet le courant et la tension à cadence de 1 kHz. La trame 0x102 historique et son payload de 5 octets sont conservés chaque seconde sur le CAN DCC pour la protection globale entre boosters; la télémétrie détaillée de l’EXCC passe par UART.
- Le défaut de réception CAN a été corrigé : `EXCC_CanCommands::process()` est appelé sur le bus 0, configuré en CAN Command à 250 kbit/s sans loopback en exploitation.

### Étape 2 — Définir le protocole UART EXCC–Booster

- [x] Confirmer UART2 côté EXCC, UART1 côté Booster, le débit 460800 bauds 8N1 et le budget de bande passante; le câblage physique relève du PCB.
- [x] Définir les messages versionnés pour heartbeat, commande/ACK, courant/tension/état, RailCom et défauts.
- [x] Définir le cadrage, la longueur maximale, le CRC, le séquencement et le rejet des messages inconnus ou corrompus.
- [x] Définir un heartbeat EXCC→Booster toutes les 100 ms; couper la sortie après 250 ms sans heartbeat valide.
- [x] Définir les comportements sûrs : échantillon courant périmé après 10 ms → canton marqué occupé et défaut signalé; état du booster transmis explicitement au CC.
- [x] Garder dans EXCC la calibration courant, les seuils et la fusion finale d’occupation; fournir les mesures de courant à 1 kHz par UART.
- [x] Définir les unités, plages, débit d’échantillons courant, fréquence de télémétrie et budget de bande passante UART.

**Critère de sortie :** les deux firmwares partagent une spécification de messages avant leur implémentation.

**Spécification de référence retenue :**

- Trame binaire : `A5 5A | version:u8 | type:u8 | séquence:u16 LE | longueur:u8 | payload | CRC:u16 LE`. CRC-16/CCITT-FALSE (polynôme `0x1021`, initialisation `0xFFFF`, sans réflexion, xorout `0x0000`) calculé de `version` à la fin du payload. Champs multi-octets du payload little-endian; payload maximal 32 octets.
- Types minimaux : `0x01` heartbeat EXCC→Booster (100 ms), `0x02` commande ON/OFF, `0x03` profil N/HO, `0x04` ACK, `0x10` échantillon Booster→EXCC (1 kHz : courant mA, tension mV, état, défauts et validité), `0x11` événement RailCom (type/canal et adresse).
- Les commandes de puissance et profil sont idempotentes; l’ACK référence le type et le numéro de séquence de la commande. Recalibration et seuils ne sont pas des commandes UART : ils restent gérés par EXCC.
- Le paquet d’échantillon courant est le heartbeat Booster→EXCC. Une donnée absente, invalide ou vieille de plus de 10 ms ne doit pas être réutilisée comme mesure fraîche.
- Le CAN DCC conserve la trame historique 0x102 (ID booster, type, courant/10, tension/100, état) chaque seconde afin que les autres boosters maintiennent leur protection globale.
- Un échantillon de 7 octets plus l’en-tête et le CRC représente environ 160 kbit/s à 1 kHz en UART 8N1, soit environ 35 % du débit de 460800 bauds, avant les commandes et événements occasionnels.

### Étape 3 — Préparer le firmware du Booster

- [x] Garder le TWAI du booster exclusivement sur le CAN DCC du convertisseur.
- [x] Garder la réception et le décodage DCC, le FIFO, la régénération RMT et le minutage local du cutout sur l’ESP32 Booster.
- [x] Maintenir le DRV8874, RailCom, IPROPI, VRAIL, protections courant/tension et digipot sur l’ESP32 Booster.
- [x] Ajouter le port UART vers EXCC pour commandes et télémétrie; ne pas retransmettre les blocs DCC à EXCC.
- [x] Transmettre les mesures de courant à EXCC à 1 kHz avec un indicateur de validité.
- [x] Couper la sortie à la perte de DCC ou après 250 ms sans heartbeat UART valide; appliquer les limites de courant/tension.
- [x] Maintenir le rapport périodique d’état et les événements RailCom vers EXCC par UART.
- [x] Préserver la trame 0x102 sur CAN DCC pour la protection globale entre boosters, sans y faire passer la supervision propre à EXCC.

**Critère de sortie :** le booster reçoit et régénère le DCC indépendamment de l’EXCC; les commandes de supervision passent par UART.

### Étape 4 — Adapter EXCC en passerelle de supervision

- [x] Retirer de l’initialisation EXCC les pilotes locaux DRV8874, RMT DCC, RailCom et mesures dédiées au booster externe.
- [x] Ajouter le client UART et la gestion non bloquante des messages, erreurs, délais et reconnexions.
- [x] Conserver le CAN Command avec le CC; vérifier l’index de bus, le mode normal (pas loopback en production) et l’appel effectif au traitement des commandes.
- [x] Configurer le TWAI EXCC au débit du CAN Command (250 kbit/s) et désactiver le loopback matériel en exploitation.
- [x] Faire suivre au Booster les commandes de puissance et de profil de voie; conserver recalibration et seuils dans EXCC.
- [x] Relayer vers le CC la télémétrie, les défauts et les adresses RailCom reçus du Booster.
- [x] Maintenir les LEDs d’état existantes alimentées par l’état distant du Booster.
- [x] Préserver la fusion d’occupation EXCC et la calibration courant en garantissant une source de courant fraîche au rythme requis.

**Critère de sortie :** EXCC ne commande plus directement le DRV8874, mais le CC peut toujours superviser le booster au travers de l’EXCC.

### Étape 5 — Vérifier progressivement

- [x] Compiler EXCC seul avec les fonctions canton et signaux conservées.
- [x] Compiler le firmware Booster seul avec le DCC sur son TWAI et la liaison UART.
- [ ] Tester le protocole UART avec messages valides, CRC invalide, trames tronquées, messages inconnus et perte de liaison.
- [ ] Tester séparément les CAN général, Command et DCC; vérifier qu’aucune trame DCC ne circule sur CAN Command.
- [ ] Vérifier le démarrage EXCC sans CantonController, puis le comportement en cas de perte du CC après connexion.
- [ ] Tester commandes CC → EXCC → UART → Booster et retour télémétrie/RailCom Booster → UART → EXCC → CC.
- [ ] Tester les défauts et timeouts : absence de trames DCC, perte UART, surintensité, sous-tension et FAULT DRV8874.
- [ ] Après fabrication du PCB, valider au banc les formes DCC, cutout, RailCom, alimentation, LEDs, servos, switches et occupation du canton.

**Critère final :** les fonctions existantes d’EXCC sont préservées, le DCC est régénéré par le contrôleur booster et une panne de communication conduit à un état sûr et observable.

## 5. Fichiers et projets modifiés

- **EXCC** : passerelle `src/EXCC_BoosterUart.*`, protocole `src/UartProtocol.h`, runtime et occupation, calibration, commandes CAN, configuration CAN, brochage et configuration PlatformIO.
- **Booster** : liaison `src/BoosterUartLink.*`, copie synchronisée de `src/UartProtocol.h`, cœur, brochage et point d’entrée pour les mesures, commandes UART et failsafe.
- Les anciens modules locaux de génération DCC, DRV8874 et RailCom ont été supprimés du firmware EXCC.
- Le protocole CAN du CantonController reste inchangé; l’EXCC utilise son interface TWAI pour CAN Command.

## 6. Décisions gelées et validation restante

- CAN Command : bus 0 TWAI EXCC à 250 kbit/s, loopback désactivé; traitement CAN appelé depuis le runtime.
- L’EXCC attend une première trame CAN valide du CC avant d’émettre ses propres messages, afin d’éviter des émissions non acquittées au démarrage isolé.
- Les LEDs de statut Booster restent dans EXCC et reflètent l’état distant reçu par UART.
- La trame 0x102 reste sur le CAN DCC pour la protection globale entre boosters.
- Restent à faire : tests UART (CRC, trames tronquées/inconnues, perte de liaison), essais des trois réseaux CAN et tests de bout en bout avec le matériel.

## 7. Règle de migration

L’intégration logicielle est réalisée et les deux firmwares compilent. Garder les fonctions non liées au booster dans EXCC. Le chemin de commande CC et la télémétrie forment une passerelle CAN↔UART, pas une fonction locale de puissance. La validation logicielle ne remplace pas les mesures à l’oscilloscope et les essais sur la voie.

## 8. Travaux PCB reportés

À reprendre après le gel et la validation logicielle; la conception du PCB est prise en charge par l’utilisateur.

- Choisir et dimensionner la source 5 V commune aux DevKit, la masse, la dissipation et la protection contre un retour d’alimentation par USB.
- Finaliser les empreintes, connecteurs, accès USB et affectations électriques des broches, y compris les contraintes de strap.
- Vérifier les transceivers CAN réellement présents et définir les terminaisons de chaque réseau; ne pas relier ensemble les réseaux CAN distincts.
- Valider l’isolation, l’alimentation booster/voie, le front-end RailCom et les mesures analogiques.
