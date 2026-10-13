# 🌐 Répertoire des Sources & Ressources Internet : Mapping Mémoire et Gestion de la Mémoire Physique

> **Projet :** Kernel From Scratch (KFS)  
> **Objet :** Recensement exhaustif et commenté des sources, documentations, cours académiques, livres blancs et dépôts open source traitant du mapping mémoire et des algorithmes de gestion de mémoire physique.

---

## Sommaire des Ressources
1. [Spécifications Matérielles & Standards Bootloader](#1-spécifications-matérielles--standards-bootloader)
2. [La Référence Communautaire : OSDev Wiki](#2-la-référence-communautaire--osdev-wiki)
3. [Noyau Linux : Documentation Officielle & Livres de Référence](#3-noyau-linux--documentation-officielle--livres-de-référence)
4. [Cours Universitaires & Manuels Académiques (Libre Accès)](#4-cours-universitaires--manuels-académiques-libre-accès)
5. [Tutoriels Pratiques & Projets Open Source de Référence](#5-tutoriels-pratiques--projets-open-source-de-référence)
6. [Articles Scientifiques & Papiers Fondateurs sur les Algorithmes](#6-articles-scientifiques--papiers-fondateurs-sur-les-algorithmes)

---

## 1. Spécifications Matérielles & Standards Bootloader

Ces documents constituent la source de vérité ultime au niveau électronique et micro-architectural pour x86.

* **[Intel® 64 and IA-32 Architectures Software Developer’s Manual — Volume 3A: System Programming Guide](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)**
  - **Chapitre 3 :** *Protected-Mode Memory Management* (Segmentation, Sélecteurs, Descripteurs GDT/LDT).
  - **Chapitre 4 :** *Paging* (Description exhaustive bit par bit des répertoires de pages, tables de pages, répertoires PSE 4 Mo, PAE, bits de contrôle `CR0`, `CR3`, `CR4` et comportement de la faute `#PF`).
  - *Utilité :* La référence absolue pour vérifier les flags matériels exacts et le fonctionnement du CPU.

* **[Spécification Multiboot 1 (GNU GRUB)](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)**
  - **Section 3.3 :** *Boot information format* (`multiboot_info_t`, flags `MULTIBOOT_INFO_MEMORY` et `MULTIBOOT_INFO_MEM_MAP`).
  - **Section 3.4 :** *Memory map format* (`multiboot_memory_map_t`, types de mémoire 1 à 5).
  - *Utilité :* Indispensable pour parser correctement la mémoire passée par GRUB dans `kmain.c`.

---

## 2. La Référence Communautaire : OSDev Wiki

OSDev.org est la plus grande base de connaissances au monde dédiée aux développeurs de noyaux de systèmes d'exploitation.

### Algorithmes de Gestion de Mémoire Physique (PMM)
* **[OSDev Wiki — Page Frame Allocation](https://wiki.osdev.org/Page_Frame_Allocation)**
  - Décrit le rôle du PMM, l'exclusion des zones réservées, et compare les différentes approches : Bitmap (Bit Array), Pile (Stack), Liste Chaînée (Free List) et Buddy Allocator.
* **[OSDev Wiki — Detecting Memory (x86)](https://wiki.osdev.org/Detecting_Memory_(x86))**
  - Guide complet sur la façon d'obtenir la carte mémoire sous BIOS (`INT 0x15, EAX=0xE820`), via Multiboot ou via UEFI `GetMemoryMap`.
* **[OSDev Wiki — Memory Map (x86)](https://wiki.osdev.org/Memory_Map_(x86))**
  - Carte détaillée de la mémoire sous le premier mégaoctet (IVT, BDA, EBDA, zone vidéo VGA `0xA0000 - 0xBFFFF`, ROM).
* **[OSDev Wiki — Memory Management Overview](https://wiki.osdev.org/Memory_Management)**
  - Synthèse globale articulant PMM, VMM, Kernel Heap et User Heap.

### Pagination et Mémoire Virtuelle (VMM)
* **[OSDev Wiki — X86 Paging](https://wiki.osdev.org/X86_Paging)**
  - Vue détaillée de la pagination 32 bits standard, tables de pages, registres de contrôle `CR0`/`CR3` et drapeaux PDE/PTE.
* **[OSDev Wiki — Setting Up Paging](https://wiki.osdev.org/Setting_Up_Paging)**
  - Tutoriel étape par étape pour construire les premières tables de pages et basculer en mode paginé sans faire crasher le CPU.
* **[OSDev Wiki — Fractal Page Mapping (Recursive Paging)](https://wiki.osdev.org/Fractal_Page_Mapping)**
  - Explication mathématique et pratique de l'astuce consistant à pointer la 1023ème entrée du Page Directory vers lui-même pour mapper toutes les tables de pages.
* **[OSDev Wiki — Higher Half x86 Bare Bones](https://wiki.osdev.org/Higher_Half_x86_Bare_Bones)**
  - Patron de conception pour exécuter le noyau à l'adresse virtuelle `0xC0000000` (3 Go) tout en étant chargé physiquement à `0x00100000` (1 Mo).
* **[OSDev Wiki — Translation Lookaside Buffer (TLB)](https://wiki.osdev.org/TLB)**
  - Fonctionnement du cache TLB, règles de purge et utilisation de l'instruction assembleur `INVLPG`.
* **[OSDev Wiki — Page Fault Handling](https://wiki.osdev.org/Page_Fault)**
  - Détail de l'interruption 14, décodage du registre `CR2` et des bits d'erreurs empilés par le processeur.

---

## 3. Noyau Linux : Documentation Officielle & Livres de Référence

Le sous-système mémoire de Linux représente l'état de l'art mondial en matière de gestion de mémoire physique et virtuelle.

* **[Understanding the Linux Virtual Memory Manager — Mel Gorman (Livre Libre Complet)](https://www.kernel.org/doc/gorman/html/understand/)**
  - *Le chef-d'œuvre de référence sur la mémoire de Linux.*
  - **Chapitre 2 : Describing Physical Memory :** Nœuds NUMA, zones (`ZONE_DMA`, `ZONE_NORMAL`, `ZONE_HIGHMEM`), pages physiques (`struct page`).
  - **Chapitre 3 : Page Table Management :** Architecture des tables de pages multi-niveaux.
  - **Chapitre 5 : Boot Memory Allocator :** Fonctionnement de `bootmem` (l'ancêtre du bump allocator de Linux).
  - **Chapitre 6 : Physical Page Allocation (Buddy System) :** Analyse mathématique et algorithmique détaillée du Buddy Allocator dans Linux (`alloc_pages`, `free_pages`, calculs d'ordres).
  - **Chapitre 8 : Slab Allocator :** Gestionnaire d'objets assis au-dessus du Buddy System.

* **[Documentation Officielle du Noyau Linux — Memory Management (MM)](https://docs.kernel.org/mm/)**
  - Documentation moderne maintenue par les mainteneurs du sous-système mémoire de Linux.
  - Couvre la gestion physique des cadres, la compaction de mémoire, le zonage et la pagination.

* **[Code Source Linux : mm/page_alloc.c](https://github.com/torvalds/linux/blob/master/mm/page_alloc.c)**
  - L'implémentation réelle en langage C du Buddy Allocator de Linux. Observer comment les listes `free_area` et les compagnons sont fusionnés avec l'opérateur XOR.

* **[Linux Kernel Development (3rd Edition) — Robert Love](https://www.oreilly.com/library/view/linux-kernel-development/9780768696974/)**
  - **Chapitre 12 : Memory Management :** Vulgarisation claire et accessible du Buddy Allocator, des zones physiques, et des allocateurs SLAB/SLUB.

---

## 4. Cours Universitaires & Manuels Académiques (Libre Accès)

Ces cours dispensés dans les plus prestigieuses universités offrent une pédagogie exemplaire accompagnée de code source fonctionnel.

* **[Operating Systems: Three Easy Pieces (OSTEP) — Remzi H. Arpaci-Dusseau & Andrea C. Arpaci-Dusseau (Université du Wisconsin-Madison)](https://pages.cs.wisc.edu/~remzi/OSTEP/)**
  - *L'un des meilleurs manuels d'OS modernes, entièrement gratuit en ligne.*
  - **Chapitre 18 : [Paging: Introduction (PDF)](https://pages.cs.wisc.edu/~remzi/OSTEP/vm-intro.pdf)** : Découpage en pages virtuelles et cadres physiques, structure de base de la table de pages.
  - **Chapitre 19 : [Paging: Faster Translations (TLBs) (PDF)](https://pages.cs.wisc.edu/~remzi/OSTEP/vm-tlb.pdf)** : Rôle et métriques du TLB.
  - **Chapitre 20 : [Paging: Smaller Tables (PDF)](https://pages.cs.wisc.edu/~remzi/OSTEP/vm-smalltables.pdf)** : Pagination multi-niveaux, arbres de tables.
  - **Chapitre 17 : [Free-Space Management (PDF)](https://pages.cs.wisc.edu/~remzi/OSTEP/vm-freespace.pdf)** : Algorithmes de gestion de l'espace libre (First-Fit, Best-Fit, Buddy System).

* **[MIT 6.S081 / 6.828: Operating System Engineering (xv6)](https://pdos.csail.mit.edu/6.828/)**
  - **Livre de cours xv6 (PDF) :** [xv6 text (RISC-V/x86)](https://pdos.csail.mit.edu/6.828/2021/xv6/book-riscv-rev2.pdf)
  - **Chapitre 3 : Page Tables :** Explication complète d'une implémentation réelle d'un PMM par pile/liste chaînée (`kalloc.c` : `freerange()`, `kalloc()`, `kfree()`) et de tables de pages (`vm.c` : `mappages()`, `walk()`).

* **[Stanford CS140: Operating Systems](https://web.stanford.edu/class/cs140/)**
  - Slides et cours sur le projet **Pintos** (gestion de la pagination, gestion de la mémoire physique et du heap noyau).

* **[UC Berkeley CS162: Operating Systems and System Support](https://cs162.org/)**
  - Conférences de référence sur l'adressage virtuel, la MMU et les algorithmes de remplacement de pages.

---

## 5. Tutoriels Pratiques & Projets Open Source de Référence

Guides pratiques pas à pas écrits par des développeurs de systèmes d'exploitation indépendants.

* **[Philipp Oppermann — Writing an OS in Rust: Memory Management](https://os.phil-opp.com/)**
  - **[Introduction to Paging](https://os.phil-opp.com/paging-introduction/) :** Très claires visualisations de la traduction d'adresse, de la fragmentation interne/externe et des tables multi-niveaux.
  - **[Paging Implementation](https://os.phil-opp.com/paging-implementation/) :** Accès aux tables de pages, mapping de la mémoire physique (offset mapping vs recursive paging).
  - **[Allocator Designs](https://os.phil-opp.com/allocator-designs/) :** Implémentation pratique d'un Bump Allocator, d'un Linked List Allocator et d'un Fixed-Size Block Allocator.

* **[BrokenThorn Entertainment — Operating System Development Series](http://www.brokenthorn.com/Resources/OSDevIndex.html)**
  - Série culte de tutoriels en C/Assembleur pour x86 :
  - **Chapitre 18 :** *Physical Memory Management I (Concepts)*
  - **Chapitre 19 :** *Physical Memory Management II (Bitmap Implementation)*
  - **Chapitre 20 :** *Virtual Memory Management I (Paging Architecture)*
  - **Chapitre 21 :** *Virtual Memory Management II (Page Directory & Tables Implementation)*

* **[James Molloy's Kernel Development Tutorial — Chapter 6: Paging](http://www.jamesmolloy.co.uk/tutorial_html/6.-Paging.html)**
  - Tutoriel historique de référence pour les étudiants développant un noyau x86 32 bits (utilise une Bitmap pour allouer les cadres physiques et configure un Page Directory).

* **[The Little Book about OS Development (Erik Helin & Adam Renberg)](https://littleosbook.github.io/)**
  - **Chapitre : Managing Memory :** Guide concis et direct sur la mise en place de la mémoire physique et virtuelle sur x86.

* **Projets Noyaux Open Source Éducatifs à Consulter :**
  - **[ToaruOS (GitHub)](https://github.com/klange/toaruos) :** Système d'exploitation complet avec interface graphique, code source très propre en C pour le PMM (bitmap) et la pagination x86.
  - **[SerenityOS (GitHub)](https://github.com/SerenityOS/serenity) :** Noyau moderne en C++ avec un gestionnaire mémoire très documenté (`Memory/PhysicalPage.cpp`, `Memory/MemoryManager.cpp`).

---

## 6. Articles Scientifiques & Papiers Fondateurs sur les Algorithmes

Les publications académiques à l'origine des algorithmes étudiés :

* **[Knowlton, Kenneth C. (1965) — "A Fast Storage Allocator"](https://doi.org/10.1145/365628.365655)**
  - *Communications of the ACM, Vol. 8, No. 10, pp. 623–625.*
  - **Le papier originel introduisant l'algorithme du Buddy System binaire.**

* **[Peterson, J. L., & Norman, T. A. (1977) — "Buddy Systems"](https://doi.org/10.1145/359605.359626)**
  - *Communications of the ACM, Vol. 20, No. 6, pp. 421–431.*
  - Analyse comparative formelle des différentes variantes du Buddy System (Binaire, Fibonacci, Weighted).

* **[Donald E. Knuth — The Art of Computer Programming, Volume 1: Fundamental Algorithms](https://www-cs-faculty.stanford.edu/~knuth/taocp.html)**
  - **Section 2.5 :** *Dynamic Storage Allocation*. Démonstrations mathématiques de la fragmentation et de l'efficacité de la fusion des compagnons.
