# 🧠 Rapport Technique : Mapping de la Mémoire & Algorithmes de Gestion de Mémoire Physique

> **Projet :** Kernel From Scratch (KFS) — Architecture x86 (IA-32 / i386)  
> **Auteur :** Antigravity AI Assistant  
> **Contexte :** Conception et implémentation du sous-système mémoire (PMM, VMM, Pagination)

---

## Table des Matières
1. [Introduction & Vue d'Ensemble](#1-introduction--vue-densemble)
2. [Partie 1 : Le Mapping de la Mémoire (Memory Mapping & Mémoire Virtuelle)](#partie-1--le-mapping-de-la-mémoire)
   - [1.1 Détection et Cartographie de la Mémoire Physique](#11-détection-et-cartographie-de-la-mémoire-physique)
   - [1.2 Mécanismes Matériels de Pagination x86 (IA-32)](#12-mécanismes-matériels-de-pagination-x86-ia-32)
   - [1.3 Le Translation Lookaside Buffer (TLB)](#13-le-translation-lookaside-buffer-tlb)
   - [1.4 Gestion des Fautes de Page (Page Fault `#PF`)](#14-gestion-des-fautes-de-page-page-fault-pf)
   - [1.5 Stratégies d'Agencement de l'Espace Virtuel](#15-stratégies-dagencement-de-lespace-virtuel)
3. [Partie 2 : Algorithmes de Gestion de la Mémoire Physique (PMM)](#partie-2--algorithmes-de-gestion-de-la-mémoire-physique-pmm)
   - [2.1 Le Rôle du Page Frame Allocator](#21-le-rôle-du-page-frame-allocator)
   - [2.2 Bump / Watermark Allocator (Allocateur de démarrage)](#22-bump--watermark-allocator-allocateur-de-démarrage)
   - [2.3 L'Allocateur par Champ de Bits (Bitmap / Bitset Allocator)](#23-lallocateur-par-champ-de-bits-bitmap--bitset-allocator)
   - [2.4 L'Allocateur par Liste Chaînée / Pile (Free List & Frame Stack)](#24-lallocateur-par-liste-chaînée--pile-free-list--frame-stack)
   - [2.5 Le Système des Compagnons (Buddy Allocator)](#25-le-système-des-compagnons-buddy-allocator)
   - [2.6 Architecture par Zones (Memory Zones)](#26-architecture-par-zones-memory-zones)
   - [2.7 Tableau Comparatif des Algorithmes PMM](#27-tableau-comparatif-des-algorithmes-pmm)
4. [Partie 3 : Guide d'Intégration & Recommandations pour KFS-3](#partie-3--guide-dintégration--recommandations-pour-kfs-3)

---

## 1. Introduction & Vue d'Ensemble

Dans un système d'exploitation moderne, la gestion de la mémoire est structurée selon une hiérarchie stricte en plusieurs couches d'abstraction :

```
+--------------------------------------------------------------------+
|                Allocateur d'Objets & Tas Noyau                     |
|           (kmalloc, kfree, SLAB, SLUB, Buddy au niveau objet)       |
+--------------------------------------------------------------------+
                                 ▲
                                 │ Demande de pages virtuelles
                                 ▼
+--------------------------------------------------------------------+
|               VMM (Virtual Memory Manager) & Pagination            |
|       (Page Directories, Page Tables, Espaces d'adressage MMU)     |
+--------------------------------------------------------------------+
                                 ▲
                                 │ Demande de cadres physiques (Frames de 4 Ko)
                                 ▼
+--------------------------------------------------------------------+
|             PMM (Physical Memory Manager / Frame Allocator)        |
|           (Bitmap Allocator, Buddy System, Free List)              |
+--------------------------------------------------------------------+
                                 ▲
                                 │ Carte mémoire brute au boot
                                 ▼
+--------------------------------------------------------------------+
|            Matériel & Bootloader (Multiboot 1 / BIOS E820)         |
+--------------------------------------------------------------------+
```

- **PMM (Physical Memory Manager) :** Gère la RAM physique brute comme un ensemble de blocs indivisibles de taille fixe appelés **cadres** (ou *page frames*, généralement de 4096 octets / 4 Ko). Il ignore totalement les adresses virtuelles et ne manipule que des adresses physiques.
- **VMM (Virtual Memory Manager) :** Configure le matériel (MMU) pour traduire les adresses virtuelles vues par le CPU en adresses physiques réelles, gère les permissions d'accès (Lecture/Écriture, Ring 0 / Ring 3) et l'isolation des processus.
- **Heap Allocator (kmalloc) :** Offre une allocation dynamique de granularité fine (ex: allouer 32 octets pour une structure `task_struct`), assis au-dessus du PMM/VMM.

---

## Partie 1 : Le Mapping de la Mémoire

### 1.1 Détection et Cartographie de la Mémoire Physique

Avant de pouvoir allouer le moindre octet, le noyau doit interroger le matériel pour savoir où se trouve la RAM disponible et quelles zones sont réservées pour le BIOS, les cartes graphiques et les périphériques (MMIO).

#### La Carte Mémoire Multiboot
Dans une architecture x86 utilisant GRUB (standard Multiboot 1, comme c'est le cas dans notre projet via `multiboot_info_t`), le bootloader interroge l'interruption BIOS `INT 0x15, EAX=0xE820` et remplit une structure en mémoire :

```c
typedef struct multiboot_memory_map {
    uint32_t size;          // Taille de la structure (sans compter 'size')
    uint64_t addr;          // Adresse physique de début du bloc
    uint64_t len;           // Longueur de la région en octets
    uint32_t type;          // Type de la région
} __attribute__((packed)) multiboot_memory_map_t;
```

Les types de mémoire standardisés sont :
- **Type 1 (`MULTIBOOT_MEMORY_AVAILABLE`) :** RAM utilisable pour le PMM.
- **Type 2 (`MULTIBOOT_MEMORY_RESERVED`) :** Mémoire réservée matériel / firmware (ne jamais toucher !).
- **Type 3 (`MULTIBOOT_MEMORY_ACPI_RECLAIMABLE`) :** Tables ACPI, réutilisables après lecture d'ACPI.
- **Type 4 (`MULTIBOOT_MEMORY_NVS`) :** Mémoire non-volatile pour le firmware.
- **Type 5 (`MULTIBOOT_MEMORY_BADRAM`) :** Cellules de mémoire défectueuses signalées par le BIOS.

#### Zones Critiques à Préserver Impérativement
Même à l'intérieur d'une région déclarée comme "Type 1" (RAM disponible), certaines zones physiques **ne doivent absolument pas être allouées** par le PMM :
1. **`0x00000000 - 0x00000400` :** Table des vecteurs d'interruption du mode réel (Real Mode IVT).
2. **`0x00000400 - 0x00000500` :** BIOS Data Area (BDA).
3. **`0x0009FC00 - 0x000FFFFF` :** Extended BIOS Data Area (EBDA), mémoire vidéo texte VGA (`0xB8000`), mémoire vidéo graphique (`0xA0000 - 0xBFFFF`), ROM BIOS.
4. **`_kernel_start - _kernel_end` :** L'image du noyau lui-même (sections `.text`, `.rodata`, `.data`, `.bss`). Le PMM doit exclure toutes les pages physiques occupées par le binaire du noyau.
5. **Structures Multiboot :** L'adresse `mb_info_addr`, la table des sections ELF (`mbi->u.elf_sec`), et la carte mémoire `mbi->mmap_addr` passées par GRUB doivent être copiées ou protégées avant d'écraser la RAM.

---

### 1.2 Mécanismes Matériels de Pagination x86 (IA-32)

Sur l'architecture 32 bits (i386 standard, sans PAE), le MMU utilise un système de pagination à deux niveaux (**Two-Level Paging**).

#### Décomposition d'une Adresse Virtuelle (32 bits)
Toute adresse linéaire de 32 bits est découpée en trois champs :

```
31        22 21        12 11          0
+-----------+-----------+-------------+
| Directory |   Table   |   Offset    |
|   Index   |   Index   |   (12 bits) |
| (10 bits) | (10 bits) |             |
+-----------+-----------+-------------+
```

- **Directory Index (Bits 31-22, 10 bits) :** Index dans le Répertoire de Pages (Page Directory, $2^{10} = 1024$ entrées).
- **Table Index (Bits 21-12, 10 bits) :** Index dans la Table de Pages (Page Table, $2^{10} = 1024$ entrées).
- **Offset (Bits 11-0, 12 bits) :** Décalage dans la page physique ($2^{12} = 4096$ octets = 4 Ko).

#### Les Registres de Contrôle
- **`CR3` (Page Directory Base Register - PDBR) :** Contient l'adresse **physique** du Page Directory (qui doit être alignée sur 4 Ko, ses 12 bits de poids faible étant à zéro ou servant de flags de cache).
- **`CR0` :**
  - **Bit 31 (`PG`, Paging) :** Mettre ce bit à 1 active la pagination matérielle.
  - **Bit 16 (`WP`, Write Protect) :** Si activé, le processeur en Ring 0 (noyau) ne peut pas écrire dans les pages en lecture seule (sécurité contre les failles).
- **`CR4` :**
  - **Bit 4 (`PSE`, Page Size Extension) :** Permet d'activer les pages géantes de 4 Mo directement au niveau du Page Directory (contournant la Page Table).

#### Structure d'une Entrée de Répertoire ou de Table (PDE / PTE)
Chaque entrée fait exactement 32 bits (4 octets) :

```
31                                12 11  9 8  7 6 5 4 3 2 1 0
+-----------------------------------+-----+-+-+-+-+-+-+-+-+-+
|   Adresse physique de base        | AVL |G|S|D|A|C|W|U|R|P|
|  (Page Table ou Page Frame 4 Ko)  |     | | | | |D|T|/|/| |
|         (20 bits hauts)           |     | | | | | | |S|W| |
+-----------------------------------+-----+-+-+-+-+-+-+-+-+-+
```

| Bit | Nom | Description |
|---|---|---|
| **0** | **P (Present)** | 1 = La page / table est présente en RAM. Si 0, tout accès déclenche une faute `#PF`. |
| **1** | **R/W (Read/Write)** | 0 = Lecture seule, 1 = Lecture et Écriture. |
| **2** | **U/S (User/Supervisor)**| 0 = Ring 0, 1, 2 uniquement (Noyau). 1 = Accessible aussi en Ring 3 (User-space). |
| **3** | **PWT (Page Write-Through)** | Contrôle la politique d'écriture de cache (1 = Write-through, 0 = Write-back). |
| **4** | **PCD (Page Cache Disable)**| 1 = Désactive le cache L1/L2/L3 pour cette page (indispensable pour le MMIO matériel !). |
| **5** | **A (Accessed)** | Mis à 1 par le CPU dès que la page a été lue ou écrite (utilisé par les algos LRU de swap). |
| **6** | **D (Dirty)** | *(PTE seulement)* Mis à 1 par le CPU lors d'une écriture dans la page. |
| **7** | **PS (Page Size)** | *(PDE seulement)* 0 = Page standard 4 Ko (pointe vers une Page Table). 1 = Page de 4 Mo. |
| **8** | **G (Global)** | Si activé (avec `CR4.PGE`), cette entrée n'est pas purgée du TLB lors du rechargement de `CR3`. Idéal pour le code noyau partagé entre tous les processus. |
| **9-11** | **AVL (Available)** | 3 bits réservés pour le système d'exploitation (utilisation libre par le dev du kernel). |
| **12-31** | **Frame Address** | Les 20 bits de poids fort de l'adresse physique (les 12 bits bas étant toujours `0x000` car alignés sur 4 Ko). |

---

### 1.3 Le Translation Lookaside Buffer (TLB)

Traduire chaque adresse mémoire en parcourant le Page Directory puis la Page Table nécessiterait deux accès mémoire supplémentaires par instruction, divisant les performances par trois. Le matériel intègre donc un cache associatif ultra-rapide : le **TLB (Translation Lookaside Buffer)**.

- **Cohérence du TLB :** Lorsque le noyau modifie une entrée dans une Page Table (par exemple pour libérer une page ou changer ses droits), le processeur **ne met pas à jour le TLB automatiquement**.
- **Invalidation :**
  - Invalider une seule adresse virtuelle : `invlpg [addr]` (instruction assembleur).
  - Invalider tout le TLB : Recharger le registre `CR3` (`mov %cr3, %eax; mov %eax, %cr3`). *Attention : ne purge pas les pages marquées du bit Global (G).*

---

### 1.4 Gestion des Fautes de Page (Page Fault `#PF`)

Dès qu'un accès mémoire viole les permissions ou touche une page non présente (`P = 0`), le processeur déclenche l'interruption matérielle **14 (`#PF`)**.

1. Le processeur place l'adresse virtuelle ayant provoqué la faute dans le registre spécial **`CR2`**.
2. Le processeur empile un **code d'erreur** 32 bits sur la pile d'interruption :
   - **Bit 0 (P) :** 0 = Page non présente, 1 = Violation de protection de droits.
   - **Bit 1 (W/R) :** 0 = Faute provoquée par une lecture, 1 = Faute provoquée par une écriture.
   - **Bit 2 (U/S) :** 0 = Le CPU était en mode noyau (Ring 0), 1 = Le CPU était en mode utilisateur (Ring 3).
   - **Bit 3 (RSVD) :** 1 = Un bit réservé a été écrit à 1 dans le PDE/PTE.
   - **Bit 4 (I/D) :** 1 = Faute survenue lors d'un fetch d'instruction (NX / No-Execute bit).

Ce mécanisme permet d'implémenter :
- L'allocation paresseuse (*Demand Paging* ou *Lazy Allocation*).
- Le *Copy-On-Write (COW)* lors d'un `fork()`.
- La mémoire virtuelle paginée sur disque (*Swapping*).
- La protection contre les pointeurs `NULL` (en ne mappant jamais l'adresse `0x00000000`).

---

### 1.5 Stratégies d'Agencement de l'Espace Virtuel

#### 1.5.1 Identity Paging (Mapping Direct 1:1)
Fait correspondre l'adresse virtuelle $V$ directement à l'adresse physique $P = V$.  
- **Usage :** Indispensable lors du bootstrap au tout début de l'activation de la pagination, car le pointeur d'instruction `EIP` exécute du code à son adresse physique d'origine au moment précis où `CR0.PG` passe à 1.

#### 1.5.2 Le Noyau en Espace Supérieur (Higher-Half Kernel)
Dans une conception propre, le noyau ne doit pas résider en bas de la mémoire (`0x00100000`), car cette zone doit être réservée au programme utilisateur (qui commence traditionnellement à `0x08048000` ou similaire).

On mappe donc le noyau à **`0xC0000000` (3 Go)** :
- **`0x00000000` à `0xBFFFFFFF` (0 à 3 Go) :** Espace Utilisateur (User-space), spécifique à chaque processus.
- **`0xC0000000` à `0xFFFFFFFF` (3 à 4 Go) :** Espace Noyau (Kernel-space), identique et partagé dans la table des pages de tous les processus.

```
0xFFFFFFFF +------------------------+
           | Noyau (Kernel Space)   |  (Partagé, Ring 0 uniquement)
0xC0000000 +------------------------+  <-- KERNEL_BASE_VIRTUAL
           | Espace Utilisateur     |  (Spécifique par processus,
           | Pile, Heap, Code, BSS  |   Ring 3 accessible)
0x00000000 +------------------------+
```

#### 1.5.3 Le Paging Récursif (Fractal / Recursive Mapping)
**Le problème fondamental :** Une fois la pagination activée, tout accès mémoire passe par la MMU (adresses virtuelles). Or, le `CR3` stocke l'adresse **physique** du Page Directory, et les PDE stockent les adresses **physiques** des Page Tables. Comment le noyau peut-il modifier le contenu d'une Page Table ou du Page Directory s'il n'a pas d'adresse virtuelle pour y accéder ?

**La solution magique :** Faire pointer la dernière entrée du Répertoire de Pages (entrée 1023) vers... le Répertoire de Pages lui-même !

```
Page Directory (Physique: 0x1000)
+------------------------------------+
| Entrée 0    -> Page Table 0        |
| ...                                |
| Entrée 1023 -> 0x1000 (Lui-même !) |
+------------------------------------+
```

Grâce à cette astuce :
1. Le Page Directory se comporte à la fois comme un Répertoire et comme sa propre Table de Pages !
2. Toutes les 1024 Tables de Pages deviennent accessibles linéairement dans l'espace virtuel à l'adresse :
   $$\text{PT\_VIRTUAL\_BASE} = 1023 \ll 22 = \text{0xFFC00000}$$
   Pour accéder à la Table de Page d'index $i$ : `((uint32_t*)0xFFC00000) + (i * 1024)`
3. Le Page Directory lui-même devient accessible à l'adresse virtuelle :
   $$\text{PD\_VIRTUAL\_BASE} = (1023 \ll 22) | (1023 \ll 12) = \text{0xFFFFF000}$$

Cette technique économise des mappings dynamiques complexes et fonctionne entièrement à coût zéro en mémoire !

---

## Partie 2 : Algorithmes de Gestion de la Mémoire Physique (PMM)

### 2.1 Le Rôle du Page Frame Allocator

Le PMM répond à un contrat strict :
- Fournir un cadre de mémoire physique libre de 4 Ko : `uintptr_t pmm_alloc_frame(void);`
- Libérer un cadre physique précédemment alloué : `void pmm_free_frame(uintptr_t frame);`
- (Optionnel) Allouer $N$ cadres physiquement contigus (crucial pour le DMA matériel).

---

### 2.2 Bump / Watermark Allocator (Allocateur de démarrage)

Un allocateur extrêmement rudimentaire composé d'un simple pointeur qui avance séquentiellement.

```
[ RAM Utilisable ..................................................... ]
▲                     ▲
└── Mémoire allouée ──┴── Prochaine alloc (next_free_frame) --->
```

- **Fonctionnement :** `alloc()` renvoie `next_free_frame` et incrémente le pointeur de 4096. `free()` ne fait rien.
- **Complexité :** Allocation en $\mathcal{O}(1)$. Libération impossible.
- **Utilité :** Très utile au démarrage immédiat du noyau pour allouer la structure du PMM principal (par exemple le tableau de la Bitmap ou les listes du Buddy System) avant que le PMM complet ne soit opérationnel. Dans Linux, ce rôle était historiquement tenu par `bootmem` puis `memblock`.

---

### 2.3 L'Allocateur par Champ de Bits (Bitmap / Bitset Allocator)

L'approche de référence pour débuter un noyau éducatif ou Unix-like (ex: James Molloy, ToaruOS).

#### Principe
La mémoire physique est vue comme un immense tableau de bits. Chaque bit représente un cadre physique de 4096 octets :
- `bit = 0` : Le cadre est libre.
- `bit = 1` : Le cadre est alloué ou réservé.

```
Cadre physique :  0     1     2     3     4     5     6     7
État :          [Libre|Occup|Occup|Libre|Libre|Occup|Occup|Occup]
Bitmap bits :     0     1     1     0     0     1     1     1  = 0xE6
```

#### Empreinte Mémoire (Overhead)
Pour 4 Go de mémoire RAM physique :
$$\text{Nombre de cadres} = \frac{4 \times 1024 \times 1024 \times 1024}{4096} = 1\,048\,576\text{ cadres}$$
$$\text{Taille de la Bitmap} = \frac{1\,048\,576\text{ bits}}{8\text{ bits/octet}} = 131\,072\text{ octets} = 128\text{ Ko}$$
> **Remarque :** Gérer l'intégralité de 4 Go de mémoire physique ne coûte que **128 Ko** de RAM, ce qui est négligeable !

#### Algorithme & Optimisations
1. **Recherche Naïve :** Parcourir bit par bit jusqu'à trouver un `0`. Pire cas : $\mathcal{O}(N)$.
2. **Recherche Accélérée par Mots (32 bits / 64 bits) :**
   On inspecte un entier `uint32_t` à la fois. Si `word == 0xFFFFFFFF`, les 32 cadres sont occupés, on passe au mot suivant en 1 cycle !
   Dès qu'un mot est `< 0xFFFFFFFF`, on trouve le premier bit à 0 instantanément grâce à l'instruction x86 `BSF` (*Bit Scan Forward*) ou la fonction intrinsèque GCC `__builtin_ctz(~word)`.
3. **Pointeur d'indice (Next-Fit hint) :** Conserver l'index de la dernière allocation évite de repartir du début de la mémoire à chaque requête.

#### Avantages & Inconvénients
-  Très simple à implémenter, déboguer et fiabiliser.
-  Overhead mémoire fixe et extrêmement bas (128 Ko pour 4 Go).
-  Libération d'un cadre en $\mathcal{O}(1)$ (`bitmap[idx] &= ~(1 << bit)`).
- ❌ Allocation de plusieurs cadres physiquement contigus lente et sujette à la fragmentation externe.

---

### 2.4 L'Allocateur par Liste Chaînée / Pile (Free List & Frame Stack)

Utilisé dans des systèmes comme **xv6** (MIT) ou les kernels orientés micro-noyaux.

#### Principe de la Pile / Free List "In-Place"
Puisqu'une page libre n'est pas utilisée, le noyau peut écrire **à l'intérieur même de la page libre** pour y stocker un pointeur vers la page libre suivante !

```
+-----------+       +-----------+       +-----------+
| Frame 0x10| ----> | Frame 0x54| ----> | Frame 0x8A| ----> NULL
| (Pointeur)|       | (Pointeur)|       | (Pointeur)|
+-----------+       +-----------+       +-----------+
```

```c
struct free_frame {
    struct free_frame *next;
};

static struct free_frame *free_list_head = NULL;

void pmm_free_frame(uintptr_t phys_addr) {
    struct free_frame *frame = (struct free_frame *)phys_addr;
    frame->next = free_list_head;
    free_list_head = frame;
}

uintptr_t pmm_alloc_frame(void) {
    if (!free_list_head) return 0; // Out of memory
    struct free_frame *frame = free_list_head;
    free_list_head = frame->next;
    return (uintptr_t)frame;
}
```

#### Avantages & Inconvénients
-  Allocation en temps constant strict $\mathcal{O}(1)$ (simple dépilement LIFO).
-  Libération en temps constant strict $\mathcal{O}(1)$ (simple empilement).
-  **Overhead mémoire nul (0 octet)** pour les métadonnées !
- ❌ **Piège majeur en mode paginé :** Pour écrire le pointeur `next` dans la page lors d'un `free()`, la page physique **doit être accessible via une adresse virtuelle valide**. Si la page n'est pas mappée dans le noyau, cela provoque un crash `#PF` immédiat !
- ❌ Impossible d'allouer des blocs contigus sans parcourir et reconstruire la liste.

---

### 2.5 Le Système des Compagnons (Buddy Allocator)

L'algorithme roi des noyaux de production (utilisé au cœur du noyau Linux dans `mm/page_alloc.c`).

#### Principe
La mémoire est divisée en blocs dont la taille est une **puissance de 2** :
$$\text{Taille} = 2^{\text{ordre}} \times 4096\text{ octets}$$

On définit des ordres allant de $0$ (1 page = 4 Ko) jusqu'à un maximum (ex: ordre 10 = $2^{10} = 1024$ pages = 4 Mo).
L'allocateur maintient une liste chaînée de blocs libres pour chaque ordre :
- `free_area[0]` : Blocs de 1 page (4 Ko)
- `free_area[1]` : Blocs de 2 pages (8 Ko)
- `free_area[2]` : Blocs de 4 pages (16 Ko)
- ...
- `free_area[10]` : Blocs de 1024 pages (4 Mo)

#### Fonctionnement de l'Allocation (Division / Splitting)
Pour allouer un bloc d'ordre $k$ :
1. Si un bloc est disponible dans `free_area[k]`, on le retire de la liste et on le renvoie.
2. Sinon, on cherche dans l'ordre supérieur $k+1$.
3. On retire le bloc d'ordre $k+1$ et on le coupe en deux moitiés égales appelées **compagnons** (*buddies*).
4. L'un des compagnons est renvoyé à l'appelant, l'autre est inséré dans `free_area[k]`.
5. Ce processus est récursif si l'on doit scinder un bloc d'ordre encore plus grand.

#### Fonctionnement de la Libération (Fusion / Coalescing)
Lorsqu'un bloc d'ordre $k$ à l'adresse $A$ est libéré :
1. On calcule l'adresse de son compagnon unique grâce à une simple opération binaire `XOR` :
   $$\text{Buddy\_Address} = A \oplus (2^k \times 4096)$$
2. On vérifie si ce compagnon est également libre (au même ordre $k$).
3. **Si le compagnon est libre :** On le retire de la liste `free_area[k]`, on fusionne les deux blocs pour reformer un bloc d'ordre $k+1$ à l'adresse $\min(A, \text{Buddy\_Address})$, et on tente à nouveau de fusionner récursivement au niveau supérieur !
4. **Si le compagnon est occupé :** On insère simplement $A$ dans `free_area[k]`.

```
[            Bloc d'ordre 2 (16 Ko)             ]
                     ▲
            Scindage / Fusion
                     ▼
[ Compagnon 1 (8 Ko)    ] [ Compagnon 2 (8 Ko)    ]
         ▲
  Scindage / Fusion
         ▼
[ 4 Ko  ] [ 4 Ko  ]
```

#### Avantages & Inconvénients
-  Support natif et instantané des allocations multi-pages contiguës (idéal pour les buffers DMA ou les tables de pages géantes).
-  Éradique très efficacement la **fragmentation externe** grâce à la fusion automatique des compagnons.
-  Allocation et libération ultra-rapides en $\mathcal{O}(\text{MAX\_ORDER}) \approx \mathcal{O}(1)$.
- ❌ **Fragmentation interne :** Une demande de 5 pages sera obligatoirement arrondie à 8 pages (ordre 3), gaspillant 3 pages physiques.
- ❌ Implémentation nettement plus complexe (gestion des arbres ou des masques d'états).

---

### 2.6 Architecture par Zones (Memory Zones)

Dans le code source de notre noyau (`arch/i386/mm/mmzone.h`), nous retrouvons la structure de zonage inspirée de Linux :

```c
enum zone_type {
    ZONE_DMA = 0,
    ZONE_NORMAL,
    ZONE_HIGHMEM,
    ZONE_MOVEABLE,
    ZONE_MAX,
};
```

#### Explication des Zones sur x86 (IA-32) :
1. **`ZONE_DMA` (`0` à `16 Mo`) :**  
   Le bus ISA et certains contrôleurs DMA anciens ne disposent que de 24 bits d'adressage et ne peuvent pas accéder au-delà de 16 Mo physique. Cette zone est sanctuarisée pour les périphériques qui en ont besoin (disquette, vieux contrôleurs de son).
2. **`ZONE_NORMAL` (`16 Mo` à `896 Mo`) :**  
   C'est la RAM standard directement mappable de façon permanente dans l'espace d'adressage virtuel du noyau (Higher Half Kernel entre `0xC0000000` et `0xFFFFFFFF`).
3. **`ZONE_HIGHMEM` (`> 896 Mo`) :**  
   Sur un CPU 32 bits, le noyau ne dispose que de 1 Go d'espace virtuel (`0xC0000000` à `0xFFFFFFFF`). Il est mathématiquement impossible de mapper directement 4 Go de RAM physique dans 1 Go d'espace virtuel !  
   Toute mémoire physique au-dessus de 896 Mo est donc classée en `HIGHMEM`. Le noyau ne peut y accéder qu'en créant des mappings temporaires au vol (mécanisme `kmap`).

---

### 2.7 Tableau Comparatif des Algorithmes PMM

| Algorithme | Complexité Alloc | Complexité Free | Support Multi-pages contiguës | Overhead Mémoire | Complexité d'Implémentation | Cas d'Usage Idéal |
|---|---|---|---|---|---|---|
| **Bump / Watermark** | $\mathcal{O}(1)$ | Impossible ($\mathcal{O}(\infty)$) |  Oui (trivial) | 1 pointeur (4 octets) | Très basse (10 lignes) | Early boot (Bootmem) |
| **Bitmap Allocator** | $\mathcal{O}(N)$ pire cas / $\mathcal{O}(1)$ moyen | $\mathcal{O}(1)$ | ⚠️ Difficile & lent | 1 bit / 4 Ko (~128 Ko pour 4 Go) | Faible (100-200 lignes) | **Recommandé pour KFS-3** |
| **Free List / Stack** | $\mathcal{O}(1)$ | $\mathcal{O}(1)$ | ❌ Non | **0 octet** (in-place) | Moyenne (attention aux mappings VMM) | Kernels minimalistes (xv6) |
| **Buddy Allocator** | $\mathcal{O}(\log M) \approx \mathcal{O}(1)$ | $\mathcal{O}(\log M) \approx \mathcal{O}(1)$ |  Excellente et native | Listes + nœuds / tables d'état | Élevée (400-800 lignes) | Linux, OS de production |

---

## Partie 3 : Guide d'Intégration & Recommandations pour KFS-3

Pour implémenter avec succès le sous-système mémoire dans notre kernel x86 32 bits, voici l'ordre chronologique d'initialisation recommandé :

```
[ Multiboot Info MBI ]
         │
         ▼
[ 1. Parsing mmap (e820) ] ──> Calculer RAM max & plages utilisables
         │
         ▼
[ 2. Init Bitmap PMM ] ──────> Marquer tout comme OCCUPÉ (1)
         │                     Puis libérer (0) uniquement les zones USABLE
         │                     Réserver la zone du kernel (_kernel_start à _kernel_end)
         ▼
[ 3. Init VMM & Pagination ] ─> Créer Page Directory initial (Boot PD)
         │                     - Identity mapping du 1er Mo (pour l'exécution)
         │                     - Higher Half mapping (0xC0000000 -> 0x00000000)
         │                     - Recursive mapping (PDE[1023] = Page Directory Phys)
         ▼
[ 4. Activation Pagination ] ─> Charger CR3, Activer CR0.PG
         │
         ▼
[ 5. Installation ISR 14 ] ───> Enregistrer le gestionnaire de #PF (Page Fault Handler)
         │
         ▼
[ 6. Heap Noyau (kmalloc) ] ──> Allocateur de tas virtuel pour structures dynamiques
```

### Pièges Fréquents à Éviter :
1. **L'alignement 4 Ko :** Toute adresse passée à `CR3`, tout Page Directory et toute Page Table **doivent** être alignés sur une frontière de 4096 octets (`address % 4096 == 0`).
2. **Écrasement du binaire noyau :** Définir des symboles d'ancrage dans le linker script (`linker.ld`) :
   ```ld
   _kernel_start = .;
   /* sections .text, .rodata, .data, .bss */
   _kernel_end = .;
   ```
   Et marquer chaque cadre entre `_kernel_start` et `_kernel_end` comme occupé dans le PMM.
3. **Oubli de l'invalidation TLB :** Toujours exécuter `invlpg` lorsqu'une page existante est réassignée ou libérée.
4. **Mélange Adresses Physiques vs Virtuelles :** Une fois `CR0.PG = 1`, le CPU ne comprend **que** le virtuel. Les tables de pages contiennent des adresses **physiques**, mais votre code C s'exécutant dans le noyau doit y accéder via leurs adresses **virtuelles** (grâce au higher-half ou au mapping récursif).
