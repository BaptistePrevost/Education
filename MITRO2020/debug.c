#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(int argc , char * argv[]) {

  //argv[1] : nom du fichier .txt à traiter
  //argv[2] : nombre de sommets du graphe
  //argv[3] : nombre d'arêtes du graphe
  //argv[4] : nombre d'arêtes du graphe


  double temps;
  clock_t t1;
  clock_t t2;

  if(argc != 4) {
    printf("Nombre d'argument incorrect");
    return 0;
  }

  int nbr_sommets = atoi(argv[2]);
  int nbr_aretes = atoi(argv[3]);

  typedef struct Sommet Sommet;
  typedef struct Liste_chainee Liste_chainee;

  struct Sommet {
    int nom;
    int degre;
    Liste_chainee* adjacents;
  };

  struct Liste_chainee {
      int sommet;
      Liste_chainee* next;
  };


//Declaration de mes 2 structures principales
  Sommet* sommets = (Sommet*)malloc(nbr_sommets * sizeof(Sommet));
  Liste_chainee* tri_sommet = (Liste_chainee*)malloc(nbr_sommets * sizeof(Liste_chainee));

  char* deleted = (char*)malloc(nbr_sommets * sizeof(char));


  FILE* fichier = fopen(argv[1],"r");

// ************ LECTURE DU FICHIER
  int iter = 0;
  int vertex[2];

  //initialisation des tableaux
  for(iter=0;iter<nbr_sommets;iter++) {

    sommets[iter].nom = iter;
    sommets[iter].degre = 0;
    sommets[iter].adjacents = NULL;

    deleted[iter] = 0;

    tri_sommet[iter].next=NULL;
    tri_sommet[iter].sommet = 0;

  }

  if (fichier != NULL) {
    // Boucle de lecture des caractères un à un
    for(iter=0;iter<nbr_aretes;iter++) {
      fscanf(fichier, "%d\n", vertex);
      fscanf(fichier, "%d\n", vertex+1);

      sommets[vertex[0]].degre+=1;
      sommets[vertex[1]].degre+=1;

      Liste_chainee* ajout1 = (Liste_chainee*)malloc(sizeof(Liste_chainee));
      ajout1->sommet = vertex[1];
      ajout1->next = sommets[vertex[0]].adjacents;
      sommets[vertex[0]].adjacents = ajout1;


      Liste_chainee* ajout2 = (Liste_chainee*)malloc(sizeof(Liste_chainee));
      ajout2->sommet = vertex[0];
      ajout2->next = sommets[vertex[1]].adjacents;
      sommets[vertex[1]].adjacents = ajout2;

    }

    fclose(fichier);
  } else {
    printf("Fichier null");
  }


//Rangement des sommets par degrés croissants dans tri_sommet

int degre;
Liste_chainee* ajout;

    for(iter=0;iter<nbr_sommets;iter++) {
      degre = sommets[iter].degre;
      ajout = malloc(sizeof(Liste_chainee));

      ajout->sommet = iter;
      ajout->next = tri_sommet[degre].next;
      tri_sommet[degre].next = ajout;
    }


// J'ai a present toutes les structures dont j'ai besoin pour faire tourner l'algorithme en temps lineaire

  t1 = clock();

  //Sommet saved_sommets[nbr_sommets];
  double density = (double)nbr_aretes/(double)nbr_sommets;
  int nbr_sommets_total = nbr_sommets;
  int nbr_aretes_total = nbr_aretes;
  int nbr_sommets_saved = nbr_sommets_total;
  int nbr_aretes_saved = nbr_aretes_total;

  Liste_chainee* update = (Liste_chainee*)malloc(sizeof(Liste_chainee));
  update->next = NULL;

  iter = 0;
  while(iter<nbr_sommets_total && nbr_aretes>0) {
    Liste_chainee* lc = &(tri_sommet[iter]);
    if(lc->next != NULL) {
      lc = lc->next;

      while(lc !=NULL && nbr_aretes>0) { //lc n'est pas une liste chaînée vide


        if(sommets[lc->sommet].degre > 0) { //il faut vérifier que le sommet n'est pas un dupliqué, ou isolé

          nbr_aretes-=sommets[lc->sommet].degre;
          nbr_sommets-=1; //on met à jour les éléments de calcul de la densité

          Liste_chainee* lc2 = (&sommets[lc->sommet])->adjacents; //on va maintenant parcourir la liste de ses sommets adjacents, pour les update

          while(lc2 != NULL) {
            //on place chaque sommet en tête de la liste chaînée indicée par son nouveau degré

            if(sommets[lc2->sommet].degre > 0) {
              //si le degre est <0 (donc == -1), on s'en fiche de le repositionner : il a déjà été supprimé
              //le degré ne peut pas être 0 : c'est un successeur de lc->sommet

              Liste_chainee* ajout = malloc(sizeof(Liste_chainee));
              sommets[lc2->sommet].degre-=1;


              if(sommets[lc2->sommet].degre <= sommets[lc->sommet].degre) { //le sommet update devient de degré <= celui du sommet qu'on supprime (éventuellement différent de iter !)

                Liste_chainee* inser = lc;
                while(inser->next != NULL && sommets[lc2->sommet].degre > sommets[inser->next->sommet].degre) { //seul le dernier noeud de la LC (qui n'est pas un sommet) a comme next NULL
                  inser = inser->next;

                    //il faut donc que le successeur soit le premier du degré que l'on cherche
                }


                ajout->sommet = lc2->sommet;
                ajout->next = inser->next;
                inser->next = ajout;


              } else { //on le met directement en tête de la liste chaînée de son nouveau degré

                ajout->sommet = lc2->sommet;
                ajout->next = tri_sommet[sommets[lc2->sommet].degre].next;
                tri_sommet[sommets[lc2->sommet].degre].next = ajout;

              }

            }

            lc2 = lc2->next;

          }

          Liste_chainee* add = malloc(sizeof(Liste_chainee));
          add->sommet = lc->sommet;
          add->next = update->next;
          update->next = add;

          if(((float)nbr_aretes/(float)nbr_sommets) > density) {

            density = (float)nbr_aretes/(float)nbr_sommets;
            nbr_aretes_saved = nbr_aretes;
            nbr_sommets_saved = nbr_sommets;

            update = update->next;
            while(update!=NULL) {
              deleted[update->sommet] = 1;
              update = update->next;
            }

            update = (Liste_chainee*)malloc(sizeof(Liste_chainee));
            update->next = NULL;

          }

        sommets[lc->sommet].degre = -1; //on symbolise ainsi qu'il a été supprimé

      } else if(sommets[lc->sommet].degre == 0) {

        nbr_sommets-=1;
        sommets[lc->sommet].degre = -1;

        Liste_chainee* add = malloc(sizeof(Liste_chainee));
        add->sommet = lc->sommet;
        add->next = update->next;
        update->next = add;

        if(((float)nbr_aretes/(float)nbr_sommets) > density) {
          density = (float)nbr_aretes/(float)nbr_sommets;
          nbr_sommets_saved = nbr_sommets;

          update = update->next;
          while(update!=NULL) {
            deleted[update->sommet] = 1;
            update = update->next;
          }

          update = (Liste_chainee*)malloc(sizeof(Liste_chainee));
          update->next = NULL;
        }

      }

        lc = lc->next;

      }
    }
    iter++;
  }


  t2 = clock();

  temps = (double)(t2-t1)/CLOCKS_PER_SEC;
  printf("\n Temps de calcul = %lf", temps);

  printf("\n\n DENSITE : %f", density);
  printf("\n sommets parcourus: %d, aretes parcourues : %d", nbr_sommets_total - nbr_sommets, nbr_aretes_total - nbr_aretes); //mesure le nombre de
  printf("\n\n Sommets du sous graphe : %d - Aretes du sous graphe : %d", nbr_sommets_saved, nbr_aretes_saved);


  FILE* fichier_r = fopen(argv[1],"r");
  FILE* fichier_w = fopen("output.txt","w+");

  for(iter=0;iter<nbr_aretes_total;iter++) {
    fscanf(fichier, "%d\n", vertex);
    fscanf(fichier, "%d\n", vertex+1);

    if(deleted[vertex[0]] !=1 && deleted[vertex[1]] != 1) {
      fprintf(fichier_w, "%d %d\n", vertex[0], vertex[1]);
    }

  }

  fclose(fichier_r);
  fclose(fichier_w);

  return 0;
}
