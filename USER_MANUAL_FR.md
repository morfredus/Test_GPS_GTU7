# Manuel Utilisateur - Testeur GPS GT-U7

**Version du document :** For software v1.2.9

## 1. Introduction

Ce document décrit le fonctionnement du projet `Test_GPS_GTU7` sur une carte de développement ESP32-S3 DevKitC-1 N16R8.

## 2. Fonctionnalités

- Affichage des données GPS (coordonnées, vitesse, altitude, etc.) sur un écran TFT.
- Indication du statut via une LED RGB NeoPixel intégrée.
- Interface web pour visualiser les données à distance.
- Boutons pour naviguer entre les différents écrans d'information.

## 3. Indicateurs Visuels (LED NeoPixel)
- **Bleu fixe :** Initialisation en cours.
- **Vert clignotant :** Recherche d'un fix GPS.
- **Vert fixe :** Fix GPS acquis.
- **Rouge :** Erreur (ex: GPS non détecté, timeout).