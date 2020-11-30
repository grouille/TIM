#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#define MAX_BUFFER  30
#define MAX_TEXT    4096

/* Images : types de données & fonctions */

// Type de données : Pixel
typedef struct Pixel
{
  unsigned char r, g, b;
} Pixel;

// Type de données : Image
typedef struct Image
{
  int width, heigh;
  Pixel* data;
} Image;

// Fonction : permet de sauvegarder une image
int save(char name[], Image* I)
{
  FILE* f = fopen(name, "w");
  if(f == NULL) return -1;
  fprintf(f, "P6\n%d %d\n255\n", I->width, I->heigh);
  for(int i=0; i<I->width*I->heigh; i++)
    fprintf(f, "%c%c%c", I->data[i].r, I->data[i].g, I->data[i].b);
  fclose(f);
  return 0;
}

// Fonction : permet de charger une image
Image* load(char name[])
{
  char buffer[MAX_BUFFER];
  int intensite;
  Image* I = malloc(sizeof(Image));
  FILE* f = fopen(name, "r");
  if(f == NULL) return NULL;
  fscanf(f, "%s %d %d %d\n", buffer, &(I->width), &(I->heigh), &intensite);

  I->data = calloc(1,I->width*I->heigh*sizeof(Pixel*));
  for(int k=0; k<I->width*I->heigh; k++)
    {
      unsigned char r,g,b;
      fscanf(f, "%c%c%c", &r, &g, &b);
      I->data[k].r = r;
      I->data[k].g = g;
      I->data[k].b = b;
    }
  fclose(f);
  return I;
}

// Fonction : permet de supprimer une image
void delete(Image* I)
{
  free(I->data);
  free(I);
}

/* Stéganographie : fonctions */

// Fonction : permet d'extraire 2 bits spécifiés d'un octet
void getPartChar(unsigned char x, unsigned char* a, int i)
{
  // 1 : 00000011
  // 4 : 11000000
  unsigned char mask[4] = {0x03, 0x0C, 0x30, 0xC0};
  unsigned char part = (x & mask[i-1]);
  *a = part >> 2*(i-1);
}

// Fonction : permet de modifier 2 bits spécifiques d'un octet
void setPartChar(unsigned char* x, unsigned char a, int i)
{
  // 1 : 11111100
  // 4 : 00111111
  unsigned char mask[4] = {0xFC, 0xF3, 0xCF, 0x3F};
  *x = (*x & mask[i-1])|(a << 2*(i-1));
}

// Fonction : code un texte dans une image à partir d'une clé
void code(Image* I, char* text, char* key)
{
  // Gestion de la clé
  int rank = 0;
  for(int j=0; j<strlen(key); j++)
  {
    rank += key[j];
  }
  rank = rank%strlen(key)+1;

  // Codage de la taille du texte
  int size = strlen(text);
  unsigned char xsize[2];
  memcpy(xsize, &size, 2);

  for(int i=0; i<8; i++)
  {
    unsigned char k;
    getPartChar((unsigned char)xsize[i/4], &k, 4-i%4);
    setPartChar(&(I->data[(i+1)%2*i/2*rank+(i%2)*(I->width*I->heigh-1-i/2*rank)].b), k, 1);
  }

  assert(I && size*4*rank*0.5 <= I->width*I->heigh);

  // Codage du texte
  for(int i=0; i<size*4; i++)
  {
    unsigned char t;
    getPartChar((unsigned char)text[i/4], &t, 4-i%4);
    setPartChar(&(I->data[(i+1)%2*(i/2+4)*rank+(i%2)*(I->width*I->heigh-1-(i/2+4)*rank)].b), t, 1);
  }
}

// Fonction : décode le texte ancré dans l'image grâce à une clé
char* decode(Image* I, char* key)
{
  // Gestion de la clé
  int rank = 0;
  for(int j=0; j<strlen(key); j++)
  {
    rank += key[j];
  }
  rank = rank%strlen(key)+1;

  // Décodage de la taille du texte
  int size = 0;
  unsigned char usize[2];
  for(int i=0; i<8; i++)
  {
    unsigned char k;
    getPartChar(I->data[(i+1)%2*i/2*rank+(i%2)*(I->width*I->heigh-1-i/2*rank)].b, &k, 1);
    setPartChar(&(usize[i/4]), k, 4-i%4);
  }
  memcpy(&size, usize, 2);

  // Décodage du texte
  char* text = malloc(size+1);
  for(int i=0; i<size*4; i++)
  {
    unsigned char k;
    getPartChar(I->data[(i+1)%2*(i/2+4)*rank+(i%2)*(I->width*I->heigh-1-(i/2+4)*rank)].b, &k, 1);
    setPartChar((unsigned char *)&(text[i/4]), k, 4-i%4);
  }
  text[size] = '\0';
  return text;
}

/* Vigenere : fonctions */

// Fonction : encode un texte à partir d'une clé, selon la technique de Vigenere
char* vigenere(char* text, char* key)
{
  int size = strlen(text);
  char* encoded = malloc(size);
  int step = 0;
  for(int i=0; i<size; i++)
    {
      if(step == strlen(key)) step = 0;
      if(text[i] >= 65 && text[i] <= 90)
      {
        char change = text[i]+key[step++]-65;
        if(change > 90) change -= 26;
        encoded[i] = change;
      }
      else
	      encoded[i] = text[i];
    }
  return encoded;
}

// Fonction : décode un texte grâce à une clé, selon la technique de Vigenere
char* erenegiv(char* text, char* key)
{
  int size = strlen(text);
  char* decoded = malloc(size+1);
  int step = 0;
  for(int i=0; i<size; i++)
    {
      if(step == strlen(key)) step = 0;
      if(text[i] >= 65 && text[i] <= 90)
      {
        char change = text[i]-key[step++]+65;
        if(change < 65) change += 26;
        decoded[i] = change;
      }
      else
	      decoded[i] = text[i];
    }
  decoded[size] = '\0';
  return decoded;
}

/* Programme principal */

// Fonction : ancre une phrase encodée par Vigenere dans une image, puis la décode
int main(int argc, char* argv[])
{
  // argv[1] : clé de Vigenere
  if(argc < 2) { printf("Utilisation : %s : <clé de Vigenere>", argv[0]); exit(0); }
  assert(argc == 2);
  char* key = argv[1];
  for(int i=0; i<strlen(key); i++)
  {
    if(key[i]<65 || key[i]>90)
    {
      printf("La clé doit être en majuscules !\n");
      exit(0);
    }
  }

  // Récupération du texte
  printf("Entrer le texte à coder :\n");
  char text[MAX_TEXT];
  fgets(text, MAX_TEXT, stdin);
  text[strlen(text)-1] = '\0';

  // Encodage selon Vigenere
  char* encoded = vigenere(text, key);
  
  // Codage du texte
  Image* I = load("image.ppm");
  code(I, encoded, key);
  save("test.ppm", I);
  delete(I);

  // Décodage du texte
  Image* J = load("test.ppm");
  char* stegano = decode(J, key);
  delete(J);

  // Décodage selon Vigenere
  char* decoded = erenegiv(stegano, key);
  printf("\nTexte décodé :\n%s\n", decoded);

  // Désallocation de la mémoire allouée
  free(encoded);
  free(stegano);
  free(decoded);

  return 0;
}
