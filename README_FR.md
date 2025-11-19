# Testeur GPS pour ESP32-S3

[![Version](https://img.shields.io/badge/version-1.5.1-blue)](CHANGELOG.md)
[![Platform](https://img.shields.io/badge/plateforme-ESP32--S3-green)](https://docs.platformio.org/en/latest/boards/espressif32/esp32-s3-devkitc-1.html)
[![License](https://img.shields.io/badge/licence-MIT-orange)](LICENSE)

Ce projet transforme une carte ESP32-S3 (spécifiquement la variante N16R8) en un outil complet de test GPS. Il peut lire les données d'un module GPS, les afficher sur un écran local TFT ST7789 et fournir une interface web détaillée pour une surveillance en temps réel.

[🇬🇧 English Version](README.md)

## ✨ Fonctionnalités

- **Données en Temps Réel** : Visualisez la position, l'altitude, la vitesse et plus encore sur un écran TFT et un tableau de bord web.
- **Interface Web Moderne** : Une interface utilisateur web réactive accessible via WiFi, avec des mises à jour en direct via WebSockets.
- **Support Écran TFT** : Une interface multi-pages sur un écran ST7789 de 240x240. Le programme fonctionne également sans écran si celui-ci n'est pas connecté.
- **Support WiFi multi-réseaux**: Basculement automatique entre plusieurs réseaux WiFi.
- **Flexibilité des Modules GPS** : Facilement configurable pour différents modules GPS comme le GT-U7 ou le NEO-6M.
- **Indicateurs d'État** : Utilise la LED RGB NeoPixel intégrée et un buzzer optionnel pour un retour d'état clair (fix GPS, connexion WiFi).
- **Robustesse** : Le programme fonctionne sans planter même si l'écran n'est pas connecté.
- **Configuration Facile** : Tous les paramètres principaux sont centralisés dans `include/config.h`.

##  Démarrage Rapide

1.  **Installation**: Suivez le **Guide d'Installation** pour configurer votre matériel et vos logiciels.
2.  **Configuration**: Modifiez `include/secrets.h` et `include/config.h` comme décrit dans le **Guide de Configuration**.
3.  **Utilisation**: Allumez l'appareil et suivez le **Guide d'Utilisation** pour commencer à surveiller les données GPS.

## 📚 Documentation

- **INSTALL_FR.md**: Prérequis matériels et instructions d'installation.
- **CONFIG_FR.md**: Guide détaillé sur toutes les options de configuration.
- **USAGE_FR.md**: Comment utiliser l'appareil, l'interface web et l'écran.
- **ARCHITECTURE_FR.md**: Aperçu de la structure du projet et du flux du code.
- **TROUBLESHOOTING_FR.md**: Solutions aux problèmes courants.
- **CONTRIBUTING_FR.md**: Lignes directrices pour contribuer au projet.

##  Licence

Ce projet est sous licence MIT - voir le fichier LICENSE pour les détails.

## 🙏 Remerciements

- **TinyGPSPlus** par Mikal Hart - Bibliothèque de parsing GPS
- **Adafruit GFX & ST7789 Libraries** - Pour le contrôle de l'écran
- **ESPAsyncWebServer** - Serveur web asynchrone pour ESP32
- **PlatformIO** - Plateforme de développement