# Testeur GPS GT-U7

![Version](https://img.shields.io/badge/version-1.0.00--dev-blue)
![Plateforme](https://img.shields.io/badge/plateforme-ESP32--S3-green)
![Licence](https://img.shields.io/badge/licence-MIT-orange)

Un testeur complet pour module GPS GT-U7, construit sur la plateforme ESP32-S3 DevKitC-1 N16R8 avec PlatformIO.

[🇬🇧 English Version](README.md)

## 📋 Table des matières

- [Fonctionnalités](#fonctionnalités)
- [Matériel requis](#matériel-requis)
- [Configuration des broches](#configuration-des-broches)
- [Logiciels requis](#logiciels-requis)
- [Installation](#installation)
- [Configuration](#configuration)
- [Utilisation](#utilisation)
- [Interface Web](#interface-web)
- [Pages de l'écran TFT](#pages-de-lécran-tft)
- [Dépannage](#dépannage)
- [Développement](#développement)
- [Licence](#licence)

## ✨ Fonctionnalités

- **Affichage GPS en temps réel**: Visualisation de la position, altitude, vitesse, cap, et plus
- **Interface Web moderne**: Tableau de bord web professionnel avec mises à jour WebSocket
- **Écran TFT**: Affichage multi-pages montrant données GPS, diagnostics et infos satellites
- **Support WiFi multi-réseaux**: Basculement automatique entre plusieurs réseaux WiFi
- **Outils de diagnostic**: Diagnostics complets et statistiques du module GPS
- **Retour sonore**: Notifications par buzzer pour acquisition et perte du fix GPS
- **Indicateurs visuels**: LED RGB d'état
- **Reset GPS à chaud**: Réinitialisation du GPS via interface web ou commande
- **Suivi des satellites**: Affichage du nombre de satellites et qualité du signal
- **Faible empreinte mémoire**: Optimisé pour ESP32-S3 pour éviter les bootloop

## 🔧 Matériel requis

### Composants principaux

- **ESP32-S3 DevKitC-1 N16R8** (Flash 16MB, PSRAM OPI 8MB)
- **Module GPS GT-U7** avec antenne
- **Écran TFT ST7789** (240x320 pixels, interface SPI)
- **LED RGB** (Cathode commune)
- **Buzzer** (Actif ou Passif)
- **2x Boutons poussoirs**
- **Capteur de lumière** (Optionnel, compatible ADC)

### Composants optionnels

- **Capteur BME280** (Température, Humidité, Pression via I2C)
- **Écran OLED** (Écran I2C additionnel)

## 📌 Configuration des broches

### GPS GT-U7 (UART2)
- **GPS RX** → GPIO 16 (ESP32 TX)
- **GPS TX** → GPIO 17 (ESP32 RX)
- **GPS PPS** → GPIO 38 (Pulse Per Second)

### Écran TFT ST7789 (SPI)
- **CS** → GPIO 5
- **DC** → GPIO 19
- **RST** → GPIO 4
- **BL** → GPIO 15 (Rétroéclairage)
- **SCL** → GPIO 18 (Horloge SPI)
- **MOSI** → GPIO 12 (Données SPI)

### LED RGB (Cathode commune)
- **Rouge** → GPIO 14
- **Vert** → GPIO 13
- **Bleu** → GPIO 10

### Boutons
- **Bouton 1** → GPIO 1 (Changement de page)
- **Bouton 2** → GPIO 2 (Réservé)

### Autres périphériques
- **Buzzer** → GPIO 3
- **Capteur lumière** → GPIO 6 (ADC)
- **I2C SDA** → GPIO 21
- **I2C SCL** → GPIO 20

## 💻 Logiciels requis

- **PlatformIO IDE** ou **PlatformIO Core**
- **Visual Studio Code** (recommandé)
- **Python 3.x** (pour PlatformIO)

## 📥 Installation

### 1. Cloner le dépôt

```bash
git clone <url-du-dépôt>
cd Test_GPS_GTU7
```

### 2. Installer PlatformIO

Si vous n'avez pas installé PlatformIO :

**Avec VS Code :**
- Installer l'extension PlatformIO IDE depuis le marketplace VS Code

**Avec CLI :**
```bash
pip install platformio
```

### 3. Configurer les secrets

Créer le fichier `include/secrets.h` avec vos identifiants WiFi :

```cpp
#ifndef SECRETS_H
#define SECRETS_H

#define WIFI_SSID_1         "votre-ssid-1"
#define WIFI_PASSWORD_1     "votre-mot-de-passe-1"

#define WIFI_SSID_2         "votre-ssid-2"
#define WIFI_PASSWORD_2     "votre-mot-de-passe-2"

#endif
```

### 4. Compiler et téléverser

```bash
# Compiler le projet
pio run

# Téléverser vers l'ESP32-S3
pio run --target upload

# Moniteur série
pio device monitor
```

## ⚙️ Configuration

Toute la configuration est centralisée dans `include/config.h` :

- **Définitions des broches**: Toutes les assignations GPIO
- **Paramètres écran**: Résolution TFT, couleurs, rotation
- **Paramètres GPS**: Vitesse de transmission, intervalles de mise à jour, timeouts
- **Paramètres WiFi**: Timeouts de connexion, délais de retry
- **Paramètres buzzer**: Fréquences et durées
- **Paramètres LED**: Motifs de clignotement
- **Paramètres mémoire**: Tailles de buffer, configuration sprite
- **Paramètres debug**: Vitesse série, flags de debug

## 🚀 Utilisation

### Démarrage initial

1. Connecter l'ESP32-S3 à l'alimentation
2. L'appareil tentera de se connecter aux réseaux WiFi configurés
3. La LED verte indique une connexion WiFi réussie
4. Le module GPS commence à chercher les satellites
5. La LED bleue clignote pendant la recherche du fix GPS
6. Le buzzer émet un bip lorsque le fix GPS est acquis

### Contrôles des boutons

- **Bouton 1 (GPIO 1)**: Parcourir les pages d'affichage
  - Page 1 : Données GPS (position, vitesse, altitude)
  - Page 2 : Diagnostics (statistiques, HDOP, taux de succès)
  - Page 3 : Satellites (nombre de satellites, qualité du signal)

### Indicateurs LED d'état

- **LED Rouge**: Fix GPS perdu / Échec connexion WiFi
- **LED Verte**: Fix GPS acquis / WiFi connecté
- **Jaune (Rouge+Vert)**: Recherche de fix
- **Rouge clignotant**: Pas de fix GPS (clignotement lent)

### Retour sonore du buzzer

- **Bip 2000 Hz**: Fix GPS acquis
- **Bip 1000 Hz**: Fix GPS perdu
- Peut être désactivé dans `config.h` en mettant `BUZZER_ENABLED` à `false`

## 🌐 Interface Web

### Accès à l'interface

Une fois connecté au WiFi, l'adresse IP s'affiche sur l'écran TFT.

Accéder à l'interface web à : `http://<ADRESSE-IP>/`

### Fonctionnalités du tableau de bord Web

- **Mises à jour en temps réel**: Données GPS en direct via WebSocket
- **État GPS**: État du fix, nombre de satellites, HDOP
- **Données de position**: Latitude, longitude, altitude, vitesse, cap
- **Date et heure**: Date et heure UTC du GPS
- **Diagnostics**: Phrases valides, checksums échoués, taux de succès
- **Reset GPS**: Bouton de réinitialisation à chaud du module GPS
- **Design responsive**: Fonctionne sur mobile, tablette et desktop
- **Interface moderne**: Design professionnel avec gradient et animations

### API WebSocket

L'appareil envoie des données JSON toutes les secondes à tous les clients WebSocket connectés :

```json
{
  "fix": true,
  "satellites": 8,
  "hdop": "1.2",
  "uptime": "0h 5m 32s",
  "latitude": "48.856614",
  "longitude": "2.352222",
  "altitude": "35.0 m",
  "speed": "0.0 km/h",
  "course": "0.0°",
  "date": "18/11/2025",
  "time": "14:23:45",
  "age": "120 ms",
  "validSentences": 1234,
  "failedChecksums": 2,
  "totalChars": 45678,
  "successRate": "99.8%"
}
```

## 📱 Pages de l'écran TFT

### Page 1 : Données GPS
- Latitude et Longitude (6 décimales)
- Altitude (mètres)
- Vitesse (km/h)
- Cap (degrés)
- Nombre de satellites
- Date et heure UTC

### Page 2 : Diagnostics
- Phrases NMEA valides reçues
- Nombre de checksums échoués
- Caractères traités
- Pourcentage de taux de succès
- HDOP (Dilution de précision horizontale)
- Âge des données (temps depuis dernière mise à jour)
- Uptime système

### Page 3 : Satellites
- Nombre de satellites
- Graphique en barres de qualité du signal
- Valeur HDOP
- Temps avec fix / État de recherche

## 🔍 Dépannage

### Le GPS n'obtient pas de fix

1. **Assurer une vue dégagée du ciel**: Le GPS nécessite une ligne de vue vers les satellites
2. **Vérifier la connexion d'antenne**: Vérifier que l'antenne GPS est correctement connectée
3. **Attendre plus longtemps**: Le fix initial peut prendre 30-60 secondes (démarrage à froid)
4. **Vérifier le câblage**: Vérifier que TX/RX sont correctement croisés
5. **Vérifier la vitesse**: Le GT-U7 par défaut est à 9600 bauds

### L'écran TFT ne fonctionne pas

1. **Vérifier les connexions SPI**: Vérifier que toutes les broches SPI sont correctement câblées
2. **Rétroéclairage**: S'assurer que GPIO 15 est HIGH (rétroéclairage allumé)
3. **Alimentation**: Vérifier l'alimentation 3.3V ou 5V (selon l'écran)
4. **Configuration TFT_eSPI**: Vérifier les build flags dans platformio.ini

### Échec de connexion WiFi

1. **Vérifier les identifiants**: Vérifier SSID et mot de passe WiFi dans `secrets.h`
2. **Force du signal**: S'assurer que l'ESP32 est à portée WiFi
3. **Réseau 2.4GHz**: L'ESP32 ne supporte que 2.4GHz, pas 5GHz
4. **Paramètres routeur**: Vérifier si le filtrage MAC est activé

### Problèmes de bootloop

1. **Configuration PSRAM**: Vérifier les paramètres OPI PSRAM dans platformio.ini
2. **Conflits de broches**: Vérifier les conflits avec les broches de strapping
3. **Alimentation**: S'assurer d'une alimentation adéquate (500mA+)
4. **Réduire l'usage sprite**: Mettre `USE_TFT_SPRITE` à `false` dans config.h
5. **Vérifier le moniteur série**: Chercher les messages de crash au démarrage

### L'interface Web ne se charge pas

1. **Vérifier l'adresse IP**: Vérifier l'IP correcte depuis l'écran TFT ou le moniteur série
2. **Pare-feu**: Vérifier si le pare-feu bloque le port 80
3. **Connexion WiFi**: S'assurer que l'appareil est connecté au même réseau
4. **Cache navigateur**: Vider le cache du navigateur ou essayer le mode navigation privée

## 🛠️ Développement

### Structure du projet

```
Test_GPS_GTU7/
├── include/
│   ├── config.h         # Configuration matérielle et logicielle
│   └── secrets.h        # Identifiants WiFi (pas dans Git)
├── src/
│   └── main.cpp         # Code principal de l'application
├── lib/                 # Bibliothèques personnalisées (si nécessaire)
├── test/                # Tests unitaires (si nécessaire)
├── platformio.ini       # Configuration PlatformIO
├── .gitignore          # Règles d'exclusion Git
├── README.md           # Documentation anglaise
├── README_FR.md        # Ce fichier (Français)
└── CHANGELOG.md        # Historique des versions
```

### Ajout de nouvelles fonctionnalités

1. Mettre à jour la version dans `include/config.h`
2. Implémenter la fonctionnalité dans `src/main.cpp`
3. Mettre à jour la documentation
4. Mettre à jour CHANGELOG.md
5. Tester minutieusement sur le matériel
6. Commit avec versioning sémantique

### Débogage

Activer la sortie de debug dans `include/config.h` :

```cpp
#define DEBUG_ENABLED       true
#define SERIAL_DEBUG_BAUD   115200
```

Voir la sortie de debug :
```bash
pio device monitor -b 115200
```

### Optimisation mémoire

- L'ESP32-S3 N16R8 a 8MB de PSRAM
- Les buffers sprite TFT sont désactivés par défaut pour éviter le bootloop
- Un buffer plein écran serait de 153 600 octets (240×320×2)
- La configuration actuelle utilise le dessin direct pour minimiser l'utilisation mémoire

### Compilation pour production

1. Mettre `DEBUG_ENABLED` à `false` dans `config.h`
2. Changer `build_type` à `release` dans `platformio.ini`
3. Retirer le suffixe `-dev` de la version
4. Compiler et tester :
```bash
pio run --environment esp32-s3-devkitc-1
```

## 📄 Licence

Ce projet est sous licence MIT - voir le fichier LICENSE pour les détails.

## 🤝 Contributions

Les contributions sont les bienvenues ! Veuillez :

1. Forker le dépôt
2. Créer une branche de fonctionnalité
3. Commit vos changements
4. Push vers la branche
5. Ouvrir une Pull Request

## 📞 Support

Pour les problèmes, questions ou suggestions :

- Ouvrir une issue sur GitHub
- Consulter la documentation existante
- Revoir le CHANGELOG pour les changements récents

## 🙏 Remerciements

- **TinyGPSPlus** par Mikal Hart - Bibliothèque de parsing GPS
- **TFT_eSPI** par Bodmer - Bibliothèque d'affichage TFT
- **ESPAsyncWebServer** - Serveur web asynchrone pour ESP32
- **PlatformIO** - Plateforme de développement

---

**Version :** 1.0.00-dev
**Plateforme :** ESP32-S3 DevKitC-1 N16R8
**Module GPS :** GT-U7
**Dernière mise à jour :** 2025-11-18