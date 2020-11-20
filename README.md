# TP Cryptographie

## Structures de données

### Pixel

Un pixel peut être représenté par ses 3 composantes :
- R(ed) : pour l'intensité de la couleur rouge (0-255)
- G(reen) : pour l'intensité de la couleur verte (0-255)
- B(leu) : pour l'intensité de la couleur bleue (0-255)

J'ai choisi de représenter un pixel ainsi, en utilisant des *unsigned char* (octets).

### Image

Une image est un ensemble de pixels. Elle a des dimensions (hauteur, largeur).<br/>
Pour représenter une image, j'ai donc choisi de stocker ses dimensions, ainsi qu'un tableau de pixels, comme défini précédement.

## Partie 1 : Stéganographie

Dans une première partie, l'objectif consistait à "enfouir" un texte dans une image, sans que celle-ci n'apparaisse modifiée visuellement.<br/>
Pour cela, nous allons utiliser des images au format PPM, car ce format est facilement exploitable dans le langage C.

Une image PPM se compose :
- D'une entète de **15 octets** :
```bash
P6          # type de données
350 286     # dimensions
255         # intensité maximale possible des couleurs
```
- D'un bloc de données écrites en **binaire**. Un triplet d'octets lu correspond aux 3 valeurs RGB d'un pixel.

### Chargement & Sauvegarde d'une image

**Chargement**

Pour charger une image, il faut l'ouvrir et, dans un premier temps, récupérer son entête.<br/>
Je connais ainsi les dimensions de l'image, et donc le nombre de pixel N = hauteur*largeur.

Ensuite, j'ai créé une Image (selon le type de données décrit plus haut). J'y ai sauvegardé les dimensions de l'image et j'ai alloué en mémoire un tableau de N Pixels (selon le type de données décrit plus haut).

La dernière étape consistait à parcourir l'image grâce à une boucle pour lire les triplets d'octets et enregistrer leur valeur dans le tableau.

**Sauvegarde**

Pour sauvegarder une image, c'est le principe inverse : il faut recréer l'entête.<br/>
J'attribue les mêmes caractéristiques à l'image : même type (P6), même intensité (255), et mêmes dimensions.

Ensuite, il suffit de parcourir le tableau enregistré dans le type de données Image pour l'écrire dans le fichier image.

### Ancrage du texte

L'oeil étant moins sensible aux variations de bleu que de rouge et de vert, je vais utiliser la composante bleue des pixels pour ancrer le texte.<br/>
En utilisant les 2 bits de poid faible, j'altère très peu la donnée initiale (variable de +/- 3 maximum sur une plage de 256 valeurs).

#### Modification & extraction de bits

Dans un premier temps, il a fallu créer des fonctions permettant d'extraire 2 bits d'un octet et de modifier 2 bits d'un octet :
- *Extraire 2 bits* : permet de découper chaque octet du texte à ancrer en 4 parties.
- *Modifier les 2 bits de poid faible* : permet d'inclure les 4 parties de chaque octet du texte dans 4 pixels.
- *Extraire les 2 bits de poid faible* : permet de récupérer les 4 parties de chaque octet du texte dans 4 pixels.
- *Modifier 2 bits* : permet de reconstituer des octets avec les 4 parties récupérées.

Pour cela, j'ai créé 2 fonctions : 
- getPartChar : récupère les 2 bits n°i dans un octet (si i = 1, on récupère les 2 bits de poid faible) et décale ces 2 bits pour en faire des bits de poid faible. Le reste de l'octet est passé à 0.
- setPartChar : modifie les 2 bits n°i d'un octet en leur donnant la valeur souhaitée.

#### Codage

Dans un premier temps, j'utilisais simplement les fonctions décrites précédemment pour récupérer l'information sur mon texte et l'inclure dans l'image. Je codais donc la taille du texte, puis le texte dans les premiers pixels de l'image.

Ensuite, j'ai eu l'idée d'alterner les pixels : 0 -> N -> 1 -> N-1 -> 2 -> N-2 ...<br/>
Le problème restait le même, l'ordre avec lequel les pixels étaient codés était déterministe.

C'est là qu'interviennent les variables *key* et *rank* :
- *key* : clé entrée par l'utilisateur, composée uniquement de lettres majuscules.
- *rank* : somme des lettres du la clé modulo son nombre de lettres.

La variable *rank* détermine l'écart entre 2 pixels servant au codage : 0 -> N -> *rank* -> N-*rank* -> 2*rank* -> N-2*rank* ...<br/>
Ainsi, l'écart entre chaque pixel codé devient pseudo-aléatoire, et dépend de la clé que va entrer l'utilisateur.

#### Décodage

Pour décoder le texte d'une image, j'effectue les opérations inverses.<br/>
Dans un premier temps, je récupère la taille du texte, stockée dans les 8 premiers pixels, donc sur 2 octets (valeur entre 0 et 65535).<br/>
Ensuite, je décode le texte avec les fonctions getPartChar et setPartChar.

## Partie 2 : Vigenere

J'ai remarqué que la transformation de Vigenere était assez simple : considérons 2 lettres.<br/>
En considérant la place d'une lettre dans l'alphabet (a : 1 ; z : 26). Le caractère issu de ces 2 lettres est la somme de ces lettres modulo 27.<br/>
Par exemple : X + K = (24 + 11) % 27 = (8) = H

Le mot de passe permettant l'encryptage est donc additionné au texte modulo 27 pour obtenir un nouveau texte encrypté selon le mot de passe.<br/>
Cependant, pour des raisons de complexité, seuls les lettres majuscules sont encryptées, le reste du texte est en clair.

## Partie 3 : Pour finir...

Pour combiner l'ensemble de ce qui a été fait aux parties 1 et 2, je propose que le mot de passe de Vigenere soit égal à la clé déterminant l'espace entre chaque pixel en Stéganographie.<br/>
Ensuite, le texte est d'abord encrypté selon Vigenere avant d'être ancré dans l'image. Le processus inverse est réalisé pour restituer le texte.

Je vous propose dans la fonction *main* du projet, un petit exemple utilisant une image nommée image.ppm.

## Auteur

* **Guillaume ROUILLÉ**
