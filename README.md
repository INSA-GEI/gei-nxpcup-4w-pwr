# GEI NXP Cup — Carte puissance

Dépôt regroupant les fichiers de conception matérielle et logicielle de la carte de puissance du vehicule développé au département Génie Électrique et Informatique (GEI) de l'INSA Toulouse pour la compétition NXP Cup.

La carte de puissance est installée à l'arrière du véhicule. Elle assure la commande des actionneurs et la gestion de plusieurs périphériques du vehicule. Elle est construite autour d'un microcontrôleur **NXP MCXA153**.

## Fonctions de la carte

La carte de puissance assure les fonctions suivantes :

- **Commande des moteurs** de propulsion du véhicule.
- **Commande du servomoteur** de direction.
- **Mesure du niveau de batterie**, afin de surveiller l'alimentation du vehicule.
- **Commande des LED d'éclairage** du véhicule.
- **Connexion de périphériques I²C**.
- **Communication avec la carte principale** basée sur la FRDM-IMX8MP, via le bus I²C et une nappe de liaison.

La carte assure ainsi l'interface entre la carte principale, les actionneurs et les périphériques matériels du véhicule.

## Organisation du dépôt

Le dépôt est organisé en deux répertoires principaux.

| Répertoire | Contenu |
|---|---|
| [`hardware/`](hardware/) | Projet de conception électronique sous KiCad et nomenclature des composants (BOM) |
| [`software/`](software/) | Projet MCUXpresso IDE contenant le firmware en langage C destiné au microcontrôleur MCXA153 |

## 1. Conception matérielle — `hardware/`

Le répertoire `hardware/` contient les fichiers du projet KiCad utilisés pour concevoir la carte électronique de puissance.

Ils permettent notamment de consulter et de modifier le schéma électronique et le circuit imprimé (PCB).

La nomenclature des composants (**BOM — Bill of Materials**) est également fournie afin d'identifier les composants nécessaires à la fabrication de la carte.

### Outils nécessaires

- [KiCad](https://www.kicad.org/) : consultation et modification du schéma électronique et du circuit imprimé.
- Un outil de tableur peut être utilisé pour consulter la BOM, selon le format du fichier fourni.

Avant toute fabrication ou modification matérielle, vérifier la cohérence entre le schéma, le PCB, la nomenclature et la révision de carte utilisée sur le véhicule.

## 2. Firmware — `software/`

Le répertoire `software/` contient le projet **MCUXpresso IDE** et le code source C exécuté par le microcontrôleur NXP MCXA153.

Ce firmware assure les fonctions de commande de la carte de puissance et la communication avec la carte principale via le bus I²C.

### Outils nécessaires

- [MCUXpresso IDE](https://www.nxp.com/mcuxpresso) — version **26.2 ou supérieure**.
- Les composants logiciels et le SDK compatibles avec le projet et le microcontrôleur MCXA153.
- Une interface de programmation et de débogage compatible avec le matériel utilisé.

### Ouverture et compilation du projet

1. Installer MCUXpresso IDE dans une version compatible avec le projet.
2. Ouvrir l'environnement de développement et sélectionner un espace de travail (*workspace*).
3. Importer le projet présent dans le répertoire `software/`.
4. Vérifier que les composants logiciels et le SDK requis sont disponibles.
5. Compiler le projet et corriger les éventuelles erreurs de configuration.
6. Programmer le microcontrôleur avec le firmware compilé, en utilisant une interface de programmation compatible.

Les modalités exactes de programmation dépendent de la configuration matérielle et de l'interface de débogage utilisée.

## Ressources

- [NXP MCXA153 — informations produit et documentation](https://www.nxp.com/products/MCXA153)
- [MCUXpresso — environnement de développement NXP](https://www.nxp.com/mcuxpresso)
- [KiCad — logiciel de conception électronique](https://www.kicad.org/)

