# 📜 Changelog - Extension Canton Controller (EXCC)

*Format : [Semantic Versioning 2.0.0](https://semver.org/)*
*Date de mise à jour : 11 novembre 2025*

---

## 📌 Versions

### [Non publié] - Migration vers DCC_CAN_RMT-Booster

#### ✨ **Nouveautés**

1. **Documentation complète**
   - Ajout d'un [README.md](README.md) complet avec :
     - Présentation du projet
     - Architecture détaillée
     - Fonctionnalités
     - Protocole CAN
     - Guide d'utilisation
     - Guide d'adaptation au DCC_CAN_RMT-Booster
   - Ajout d'un [AUDIT_CANVAS.md](AUDIT_CANVAS.md) avec :
     - Analyse technique approfondie
     - SWOT Analysis
     - Canvas technique
     - Cartographie des modules
     - Recommandations
   - Ajout d'un [ADAPTATION_DCC_CAN_RMT_Booster.md](ADAPTATION_DCC_CAN_RMT_Booster.md) avec :
     - Guide de migration complet
     - Comparaison détaillée des boosters
     - Stratégie d'adaptation
     - Implémentation pas à pas
     - Tests et validation
     - Checklist de migration
   - Ajout d'un [MIGRATION_CONFIG.md](MIGRATION_CONFIG.md) avec :
     - Options de configuration
     - Configuration PlatformIO
     - Configuration des broches
     - Résolution des problèmes

2. **Interface abstraite pour le booster**
   - Création de `BoosterInterface.h` pour une abstraction complète
   - Permet l'utilisation de différents types de boosters
   - Facilite la migration vers de nouveaux boosters

3. **Adaptateurs de booster**
   - `RMT_BoosterAdapter.h/cpp` : Adaptateur pour DCC_CAN_RMT-Booster
   - `Old_BoosterAdapter.h/cpp` : Adaptateur pour CAN_DCC-Booster (fallback)
   - Permet une transition progressive

4. **Factory de booster**
   - `EXCC_BoosterFactory.h/cpp` : Création centralisée des boosters
   - Support de la détection automatique (optionnel)
   - Configuration flexible via defines

#### 🔄 **Modifications**

1. **EXCC_CanBooster.h/cpp**
   - Utilise maintenant `BoosterInterface` au lieu de `CanDccBoosterHardware`
   - Intégration des adaptateurs
   - Meilleure gestion des messages CAN

2. **EXCC_Booster_WS2812.h/cpp**
   - Utilise `BoosterInterface` pour la télémétrie
   - Meilleure intégration avec le booster sélectionné
   - Gestion unifiée du RailCom

3. **EXCC_Main.cpp**
   - Initialisation centralisée du booster via factory
   - Support des deux types de boosters
   - Configuration flexible

4. **platformio.ini**
   - Ajout de la dépendance optionnelle au DCC_CAN_RMT-Booster
   - Configuration pour les deux types de boosters

#### 🔧 **Améliorations Techniques**

1. **Architecture améliorée**
   - Meilleure séparation des responsabilités
   - Faible couplage entre modules
   - Haute cohésion
   - Meilleure maintenabilité

2. **Gestion des erreurs**
   - Meilleure validation des paramètres
   - Protection contre les overflows
   - Mécanismes de récupération améliorés

3. **Documentation du code**
   - Ajout de commentaires Doxygen
   - Documentation des interfaces
   - Exemples d'utilisation

#### 📝 **Documentation**

- Ajout de fichiers de documentation complets
- Mise à jour des commentaires dans le code
- Création de diagrammes d'architecture
- Documentation des procédues de migration

#### 🐛 **Corrections de bugs**

*Aucune correction de bug dans cette version (documentation uniquement)*

#### 🗑️ **Dépréciations**

- `CanDccBoosterHardware` : Déprécié au profit de `BoosterInterface`
- L'ancien système de configuration sera progressivement remplacé

#### ⚠️ **Breaking Changes**

*Aucun breaking change dans cette version (documentation uniquement)*

Les breaking changes seront introduits dans la version de migration vers DCC_CAN_RMT-Booster.

---

## 🏷️ Versions Précédentes

### [2.0.0] - Gestion Canton 2026 (Juillet 2025)

#### ✨ **Nouveautés**

- **Migration complète vers WS2812** : Tous les signaux et LED utilisent maintenant WS2812/WS2815
- **Nouveau système de configuration** : Centralisation dans EXCC_Config.h
- **Support multi-aiguillages** : Jusqu'à 6 aiguillages configurables
- **Télémétrie avancée** : Mesure précise courant/tension via IPROPI
- **Gestion RailCom améliorée** : Cutout et décodage HF
- **Refactorisation du protocole** : Organisation moderne avec enum class par liaison CAN

#### 🔄 **Modifications**

- Refactorisation complète des modules LED
- Séparation de EXCC_Occupation pour une meilleure modularité
- Amélioration de la gestion des servos
- Optimisation de la communication CAN

#### 📝 **Migration depuis 1.x**

- **Breaking Changes** : Incompatible avec les versions précédentes
- Nécessite une reconfiguration complète
- Nouveau matériel requis (LED WS2812)

---

### [1.5.0] - Ajout du Support CAN (2024)

#### ✨ **Nouveautés**

- Support complet du bus CAN
- Communication avec CC, ERM, CLF
- Protocole CAN 29 bits
- Gestion des priorités

#### 🔄 **Modifications**

- Refactorisation des modules de communication
- Ajout de CanBus, CanMsg, CanID
- Intégration avec CanUniversal

---

### [1.0.0] - Version Initiale (2023)

#### ✨ **Nouveautés**

- Première version stable
- Gestion de base des cantons
- Contrôle des signaux SNCF
- Pilotage des aiguillages
- Booster DCC de base

---

## 🎯 Roadmap

### 📅 Version 3.0.0 - Migration vers DCC_CAN_RMT-Booster (2026)

#### ✨ **Nouveautés prévues**

- **Intégration complète de DCC_CAN_RMT-Booster**
  - Génération DCC via RMT
  - Meilleure précision temporelle
  - Moins de charge CPU
  - RailCom amélioré

- **Nouveaux modules**
  - BoosterInterface (abstraction)
  - RMT_BoosterAdapter
  - Old_BoosterAdapter (fallback)
  - EXCC_BoosterFactory

- **Améliorations**
  - Meilleure performance
  - Meilleure fiabilité
  - Meilleure compatibilité

#### 📅 Calendrier prévisionnel

| Phase | Date | Statut |
|-------|------|--------|
| Analyse | Novembre 2025 | ✅ Terminée |
| Développement | Décembre 2025 - Janvier 2026 | 🟡 En cours |
| Tests | Février 2026 | ⏳ Prévu |
| Documentation | Mars 2026 | ⏳ Prévu |
| Release | Avril 2026 | ⏳ Prévu |

---

## 📊 Statistiques

| Version | Date | Fichiers | Lignes de Code | Contributeurs |
|---------|------|----------|----------------|---------------|
| 1.0.0 | 2023 | ~20 | ~8 000 | 1 |
| 1.5.0 | 2024 | ~30 | ~12 000 | 1-2 |
| 2.0.0 | Juillet 2025 | ~35 | ~15 000 | 1-2 |
| 3.0.0 (prévu) | Avril 2026 | ~40 | ~18 000 | 2+ |

---

## 🤝 Contribution

Pour contribuer à ce projet, veuillez consulter le [README.md](README.md) pour les instructions.

---

## 📄 Licence

Ce projet est sous licence MIT. Voir [LICENSE](LICENSE) pour plus de détails.

---

*Changelog généré par Mistral Vibe - 11 novembre 2025*
*Version : 1.0*
