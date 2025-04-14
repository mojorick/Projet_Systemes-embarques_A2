# 📡 Système Embarqué – Modes de Fonctionnement

Ce projet décrit le comportement d’un système embarqué multi-mode avec gestion d'acquisition de capteurs, carte SD, et interface série.

---

## 🚦 Modes de Fonctionnement

Le système dispose de **4 modes préprogrammés**, accessibles via des boutons poussoirs :

### 🔵 Mode Standard
- **Activation** : Démarrage sans appuyer sur un bouton.
- **Fonction** : Acquisition régulière des données capteurs (intervalle par défaut : `10 min`).
- **Horodatage** : Données enregistrées sur une ligne horodatée.
- **Timeout capteur** : Si aucune réponse au bout de `30s`, la donnée = `NA`.
- **Enregistrement** : Sur carte SD dans un fichier de taille max `2Ko`.

#### 🗂️ Nom des fichiers : `AAMMJJ_0.LOG`
Ex : `200531_0.LOG` (pour le 31 mai 2020).

> Une fois le fichier plein, il est archivé avec un numéro de révision.

---

### ⚙️ Mode Configuration
- **Activation** : Démarrage en maintenant le bouton rouge.
- **Fonction** : Configuration via interface série UART.
- **Timeout** : Retour automatique au mode standard après 30 minutes sans activité.

#### 🔧 Commandes Disponibles
| Commande | Description |
|----------|-------------|
| `LOG_INTERVALL=10` | Intervalle entre 2 mesures (minutes) |
| `FILE_MAX_SIZE=4096` | Taille max d’un fichier de log (octets) |
| `RESET` | Réinitialisation des paramètres |
| `VERSION` | Affiche la version logicielle et le numéro de lot |
| `TIMEOUT=30` | Délai maximum d’attente capteur (secondes) |

#### 📈 Paramètres Capteurs (EEPROM)

- **Luminosité**
  - `LUMIN=1`
  - `LUMIN_LOW=200`
  - `LUMIN_HIGH=700`

- **Température de l’air**
  - `TEMP_AIR=1`
  - `MIN_TEMP_AIR=-5`
  - `MAX_TEMP_AIR=30`

- **Hygrométrie**
  - `HYGR=1`
  - `HYGR_MINT=0`
  - `HYGR_MAXT=50`

- **Pression**
  - `PRESSURE=0`
  - `PRESSURE_MIN=450`
  - `PRESSURE_MAX=1030`

#### 🕓 Configuration Date/Heure RTC
| Commande | Format |
|----------|--------|
| `CLOCK` | HEURE:MINUTE:SECONDE |
| `DATE` | MOIS,JOUR,ANNEE |
| `DAY` | MON,TUE,WED,THU,FRI,SAT,SUN |

---

### 🧰 Mode Maintenance
- **Activation** : Depuis le mode standard ou économique. Appui long (5s) sur bouton rouge.
- **Fonction** :
  - Lecture des données en temps réel via port série.
  - **Carte SD peut être retirée** sans corruption.
- **Retour** au mode précédent : Appui long (5s) sur bouton rouge.

---

### 💤 Mode Économie
- **Activation** : Depuis le mode standard. Appui long (5s) sur bouton vert.
- **Fonction** :
  - Acquisition GPS 1 mesure sur 2.
  - `LOG_INTERVAL` est **multiplié par 2**.
- **Retour** au mode standard : Appui long (5s) sur bouton rouge.

---

## 🔴🟡🟢🟠 Indications LED

| Couleur LED | Signification |
|-------------|----------------|
| Verte fixe | Mode standard |
| Jaune fixe | Mode configuration |
| Bleue fixe | Mode économique |
| Orange fixe | Mode maintenance |

### ⚠️ LED clignotantes (1 Hz)

| LED | Signification |
|-----|----------------|
| Rouge + Bleue | Erreur RTC |
| Rouge + Jaune | Erreur GPS |
| Rouge + Verte (1:1) | Erreur capteur |
| Rouge + Verte (1:2) | Données incohérentes (vérification requise) |
| Rouge + Blanche (1:1) | Carte SD pleine |
| Rouge + Blanche (1:2) | Erreur accès/écriture SD |

---

## 📁 Structure des Fichiers SD

- Fichiers de log : `AAMMJJ_0.LOG`
- Taille par défaut : `2 Ko` (`FILE_MAX_SIZE`)
- Une fois plein :
  - Archivé (ex : `200531_1.LOG`)
  - Le fichier `*_0.LOG` est écrasé pour continuer.

---

## 🔚 Fin du document
