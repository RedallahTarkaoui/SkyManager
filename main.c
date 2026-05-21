
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

/* ============================================================
   STRUCTURES
   ============================================================ */
typedef struct vol{
    int id;
    char ville_depart[100];
    char ville_arrivee[100];
    char heure_depart[100];
    char heure_arrivee[100];
    int nb_places;
}vol;

typedef struct passager{
    int id;
    int age;
    char nom[50];
    char prenom[50];
    char num_passeport[50];
    char nationalite[50];
}passager;

typedef struct reservation{
    int id;
    int id_vol;
    int id_passager;
    int nb_places;
    char date_reservation[20];
}reservation;

typedef struct noeud_v{
    vol data;
    struct noeud_v *next;
}noeud_v;

typedef struct noeud_p{
    passager data;
    struct noeud_p *next;
}noeud_p;

typedef struct noeud_r{
    reservation data;
    struct noeud_r *next;
}noeud_r;

/* ============================================================
   COMPTEURS D'IDS AUTO-INCREMENTES
   ============================================================ */
int compteur_vol = 1;
int compteur_passager = 1;
int compteur_reservation = 1;

/* ============================================================
   FONCTIONS UTILITAIRES / CONTROLES DE SAISIE
   ============================================================ */

/* Vider le buffer du clavier */
void vider_buffer(){
    int c;
    while((c = getchar()) != '\n' && c != EOF);
}

/* Afficher une ligne de separation */
void afficher_ligne(char c, int n){
    for(int i=0; i<n; i++) printf("%c", c);
    printf("\n");
}

/* Saisir un entier avec controle (min/max) */
int saisir_entier(const char *message, int min, int max){
    int valeur;
    int ok = 0;
    do {
        printf("%s", message);
        if(scanf("%d", &valeur) != 1){
            printf("  [!] Saisie invalide. Entrez un nombre entier.\n");
            vider_buffer();
            ok = 0;
        } else if(valeur < min || valeur > max){
            printf("  [!] Valeur hors limites (%d - %d).\n", min, max);
            vider_buffer();
            ok = 0;
        } else {
            vider_buffer();
            ok = 1;
        }
    } while(!ok);
    return valeur;
}

/* Saisir une chaine non vide */
void saisir_chaine(const char *message, char *dest, int taille){
    int ok = 0;
    do {
        printf("%s", message);
        if(fgets(dest, taille, stdin) == NULL){
            dest[0] = '\0';
        }
        /* Supprimer le \n final */
        int len = strlen(dest);
        if(len > 0 && dest[len-1] == '\n') dest[len-1] = '\0';
        /* Verifier non vide */
        if(strlen(dest) == 0){
            printf("  [!] Ce champ ne peut pas etre vide.\n");
            ok = 0;
        } else {
            ok = 1;
        }
    } while(!ok);
}

/* Valider format heure HH:MM */
int valider_heure(const char *heure){
    if(strlen(heure) != 5) return 0;
    if(!isdigit(heure[0]) || !isdigit(heure[1])) return 0;
    if(heure[2] != ':') return 0;
    if(!isdigit(heure[3]) || !isdigit(heure[4])) return 0;
    int h = (heure[0]-'0')*10 + (heure[1]-'0');
    int m = (heure[3]-'0')*10 + (heure[4]-'0');
    return (h >= 0 && h <= 23 && m >= 0 && m <= 59);
}

/* Saisir une heure au format HH:MM */
void saisir_heure(const char *message, char *dest){
    int ok = 0;
    do {
        printf("%s", message);
        if(fgets(dest, 10, stdin) == NULL) dest[0] = '\0';
        int len = strlen(dest);
        if(len > 0 && dest[len-1] == '\n') dest[len-1] = '\0';
        if(!valider_heure(dest)){
            printf("  [!] Format invalide. Utilisez HH:MM (ex: 08:30).\n");
            ok = 0;
        } else {
            ok = 1;
        }
    } while(!ok);
}

/* Valider format date YYYY-MM-DD avec verification stricte des jours/mois */
int valider_date(const char *date){
    if(strlen(date) != 10) return 0;
    for(int i=0;i<10;i++){
        if(i==4||i==7){ if(date[i]!='-') return 0; }
        else { if(!isdigit(date[i])) return 0; }
    }
    int annee = atoi(date);
    int mois  = atoi(date+5);
    int jour  = atoi(date+8);
    if(annee < 2000 || annee > 9999) return 0;
    if(mois < 1 || mois > 12) return 0;
    if(jour < 1) return 0;
    /* Jours max par mois */
    int jours_max[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    /* Annee bissextile : divisible par 4, sauf centenaires non divisibles par 400 */
    int bissextile = (annee % 4 == 0 && (annee % 100 != 0 || annee % 400 == 0));
    if(mois == 2 && bissextile) jours_max[2] = 29;
    return (jour <= jours_max[mois]);
}

/* Saisir une date au format YYYY-MM-DD */
void saisir_date(const char *message, char *dest){
    int ok = 0;
    do {
        printf("%s", message);
        if(fgets(dest, 20, stdin) == NULL) dest[0] = '\0';
        int len = strlen(dest);
        if(len > 0 && dest[len-1] == '\n') dest[len-1] = '\0';
        if(!valider_date(dest)){
            printf("  [!] Format invalide. Utilisez YYYY-MM-DD (ex: 2025-06-15).\n");
            ok = 0;
        } else {
            ok = 1;
        }
    } while(!ok);
}

/* Attendre que l utilisateur appuie sur Entree */
void attendre_entree(){
    printf("\n  Appuyez sur Entree pour continuer...");
    vider_buffer();
}

/* ============================================================
   FONCTIONS METIER VOLS
   ============================================================ */
noeud_v* creer_vol(vol v){
    noeud_v*newE=(noeud_v*)malloc(sizeof(noeud_v));
    if(newE==NULL){ printf("Echec d'allocation!\n"); return newE; }
    newE->data=v;
    newE->next=NULL;
    return newE;
}

noeud_v* AjoutVol(noeud_v *tete,vol v){
    noeud_v *newE=creer_vol(v);
    if(newE==NULL){ printf("Echec d allocation!\n"); return tete; }
    if(tete==NULL||strcmp(v.heure_depart,tete->data.heure_depart)<0){
        newE->next=tete; tete=newE;
        printf("  [OK] Vol ajoute avec succes!\n");
        return tete;
    }
    noeud_v*temp=tete;
    while(temp->next!=NULL && strcmp(temp->next->data.heure_depart,v.heure_depart)<0)
        temp=temp->next;
    newE->next=temp->next;
    temp->next=newE;
    printf("  [OK] Vol ajoute avec succes!\n");
    return tete;
}

void afficherVols(noeud_v *tete){
    if(tete==NULL){ printf("  Aucun vol disponible.\n"); return; }
    noeud_v *temp=tete;
    printf("\n");
    afficher_ligne('-', 72);
    printf("  %-4s %-20s %-20s %-8s %-8s %-6s\n",
           "ID","Depart","Arrivee","H.Dep","H.Arr","Places");
    afficher_ligne('-', 72);
    while(temp!=NULL){
        printf("  %-4d %-20s %-20s %-8s %-8s %-6d\n",
               temp->data.id,
               temp->data.ville_depart,
               temp->data.ville_arrivee,
               temp->data.heure_depart,
               temp->data.heure_arrivee,
               temp->data.nb_places);
        temp=temp->next;
    }
    afficher_ligne('-', 72);
}

noeud_v* chercherVol(noeud_v* tete,int id){
    noeud_v*temp=tete;
    while(temp!=NULL){
        if(temp->data.id==id) return temp;
        temp=temp->next;
    }
    return NULL;
}

void saisie_modif(vol *v){
    printf("\n  --- Modification du vol ---\n");
    saisir_chaine("  Nouvelle ville de depart  : ", v->ville_depart, 100);
    saisir_chaine("  Nouvelle ville d'arrivee  : ", v->ville_arrivee, 100);
    saisir_heure ("  Nouvelle heure de depart  : ", v->heure_depart);
    saisir_heure ("  Nouvelle heure d'arrivee  : ", v->heure_arrivee);
    v->nb_places = saisir_entier("  Nouveau nombre de places  : ", 1, 9999);
    printf("  [OK] Vol modifie avec succes!\n");
}

noeud_v* ModifierVol(noeud_v*tete,int idf){
    if(tete==NULL){ printf("  Aucun vol a modifier, la liste est vide.\n"); return tete; }
    noeud_v* vol_modif=chercherVol(tete,idf);
    if(vol_modif==NULL){ printf("  [!] Aucun vol avec l'ID %d.\n", idf); return tete; }
    saisie_modif(&vol_modif->data);
    return tete;
}

noeud_v* SupprimerVol(noeud_v*tete,noeud_r**listeR,int id){
    if(tete==NULL){ printf("  Aucun vol a supprimer, la liste est vide.\n"); return tete; }
    noeud_v*tempV=tete;
    noeud_v*prevV=NULL;
    while(tempV!=NULL&&tempV->data.id!=id){ prevV=tempV; tempV=tempV->next; }
    if(tempV==NULL){ printf("  [!] Aucun vol avec l'ID %d.\n", id); return tete; }
    /* Supprimer toutes les reservations liees au vol (les places n ont plus d utilite) */
    noeud_r*tempR=*listeR;
    noeud_r*prevR=NULL;
    int nb_res_supprimees = 0;
    while(tempR!=NULL){
        if(tempR->data.id_vol==id){
            noeud_r*sup=tempR;
            if(prevR==NULL){ *listeR=tempR->next; tempR=*listeR; }
            else{ prevR->next=tempR->next; tempR=prevR->next; }
            free(sup);
            nb_res_supprimees++;
        } else { prevR=tempR; tempR=tempR->next; }
    }
    /* Supprimer le vol lui-meme */
    if(prevV==NULL) tete=tete->next;
    else prevV->next=tempV->next;
    free(tempV);
    if(nb_res_supprimees > 0)
        printf("  [i] %d reservation(s) liee(s) supprimee(s).\n", nb_res_supprimees);
    printf("  [OK] Vol et ses reservations supprimes!\n");
    return tete;
}

/* ============================================================
   FONCTIONS METIER PASSAGERS
   ============================================================ */
noeud_p* creer_passager(passager p){
    noeud_p*newE=(noeud_p*)malloc(sizeof(noeud_p));
    if(newE==NULL){ printf("Echec d'allocation!\n"); return newE; }
    newE->data=p;
    newE->next=NULL;
    return newE;
}

noeud_p* AjoutPassager(noeud_p *tete,passager p){
    noeud_p *newE=creer_passager(p);
    if(newE==NULL){ printf("Echec d allocation!\n"); return tete; }
    if(tete==NULL||p.id<tete->data.id){
        newE->next=tete; tete=newE;
        printf("  [OK] Passager ajoute avec succes!\n");
        return tete;
    }
    noeud_p*temp=tete;
    while(temp->next!=NULL && temp->next->data.id<p.id)
        temp=temp->next;
    newE->next=temp->next;
    temp->next=newE;
    printf("  [OK] Passager ajoute avec succes!\n");
    return tete;
}

void afficherPassagers(noeud_p *tete){
    if(tete==NULL){ printf("  Aucun passager disponible.\n"); return; }
    noeud_p *temp=tete;
    printf("\n");
    afficher_ligne('-', 78);
    printf("  %-4s %-15s %-15s %-4s %-12s %-15s\n",
           "ID","Nom","Prenom","Age","Passeport","Nationalite");
    afficher_ligne('-', 78);
    while(temp!=NULL){
        printf("  %-4d %-15s %-15s %-4d %-12s %-15s\n",
               temp->data.id,
               temp->data.nom,
               temp->data.prenom,
               temp->data.age,
               temp->data.num_passeport,
               temp->data.nationalite);
        temp=temp->next;
    }
    afficher_ligne('-', 78);
}

noeud_p* chercherPassager(noeud_p* tete,int id){
    noeud_p*temp=tete;
    while(temp!=NULL){
        if(temp->data.id==id) return temp;
        temp=temp->next;
    }
    return NULL;
}

void saisie_modif_P(passager *p){
    printf("\n  --- Modification du passager ---\n");
    p->age = saisir_entier("  Nouvel age            : ", 1, 120);
    saisir_chaine("  Nouveau nom           : ", p->nom, 50);
    saisir_chaine("  Nouveau prenom        : ", p->prenom, 50);
    saisir_chaine("  Nouveau num passeport : ", p->num_passeport, 50);
    saisir_chaine("  Nouvelle nationalite  : ", p->nationalite, 50);
    printf("  [OK] Passager modifie avec succes!\n");
}

noeud_p* ModifierPassager(noeud_p*tete,int idf){
    if(tete==NULL){ printf("  Aucun passager a modifier, la liste est vide.\n"); return tete; }
    noeud_p* psg_modif=chercherPassager(tete,idf);
    if(psg_modif==NULL){ printf("  [!] Aucun passager avec l'ID %d.\n", idf); return tete; }
    saisie_modif_P(&psg_modif->data);
    return tete;
}

noeud_p* SupprimerPassager(noeud_p*tete,noeud_r**listeR,noeud_v*listeV,int id){
    if(tete==NULL){ printf("  Aucun passager a supprimer, la liste est vide.\n"); return tete; }
    noeud_p*tempP=tete;
    noeud_p*prevP=NULL;
    while(tempP!=NULL&&tempP->data.id!=id){ prevP=tempP; tempP=tempP->next; }
    if(tempP==NULL){ printf("  [!] Aucun passager avec l'ID %d.\n", id); return tete; }
    noeud_r*tempR=*listeR;
    noeud_r*prevR=NULL;
    while(tempR!=NULL){
        if(tempR->data.id_passager==id){
            noeud_r*sup=tempR;
            /* CORRECTION: restituer les places au vol */
            noeud_v*vol=chercherVol(listeV, sup->data.id_vol);
            if(vol!=NULL) vol->data.nb_places+=sup->data.nb_places;
            if(prevR==NULL){ *listeR=tempR->next; tempR=*listeR; }
            else{ prevR->next=tempR->next; tempR=prevR->next; }
            free(sup);
        } else { prevR=tempR; tempR=tempR->next; }
    }
    if(prevP==NULL) tete=tete->next;
    else prevP->next=tempP->next;
    free(tempP);
    printf("  [OK] Passager et ses reservations supprimes!\n");
    return tete;
}

/* ============================================================
   FONCTIONS METIER RESERVATIONS
   ============================================================ */
noeud_r* creer_reservation(reservation r){
    noeud_r*newE=(noeud_r*)malloc(sizeof(noeud_r));
    if(newE==NULL){ printf("Echec d'allocation!\n"); return newE; }
    newE->data=r;
    newE->next=NULL;
    return newE;
}

noeud_r* AjoutReservationTriee(noeud_r *tete,reservation r){
    noeud_r *newE=creer_reservation(r);
    if(newE==NULL){ printf("Echec d allocation!\n"); return tete; }
    if(tete==NULL||strcmp(r.date_reservation,tete->data.date_reservation)<0){
        newE->next=tete; tete=newE;
        printf("  [OK] Reservation effectuee avec succes!\n");
        return tete;
    }
    noeud_r*temp=tete;
    while(temp->next!=NULL && strcmp(temp->next->data.date_reservation,r.date_reservation)<0)
        temp=temp->next;
    newE->next=temp->next;
    temp->next=newE;
    printf("  [OK] Reservation effectuee avec succes!\n");
    return tete;
}

noeud_r* AjoutReservation(noeud_r*tete,noeud_v*listeV,noeud_p*listeP,reservation r){
    noeud_v*vol=chercherVol(listeV,r.id_vol);
    if(vol==NULL){ printf("  [!] Vol introuve (ID %d), echec de reservation!\n", r.id_vol); return tete; }
    noeud_p *passager=chercherPassager(listeP,r.id_passager);
    if(passager==NULL){ printf("  [!] Passager introuvable (ID %d), echec de reservation!\n", r.id_passager); return tete; }
    if(vol->data.nb_places < r.nb_places){
        printf("  [!] Places insuffisantes (%d disponibles, %d demandees).\n", vol->data.nb_places, r.nb_places);
        return tete;
    }
    /* Toutes les verifications passees : attribuer l'ID et creer la reservation */
    r.id = compteur_reservation++;
    printf("  [i] Reservation ID auto: %d\n", r.id);
    vol->data.nb_places-=r.nb_places;
    tete=AjoutReservationTriee(tete,r);
    return tete;
}

void afficherReservations(noeud_r *tete){
    if(tete==NULL){ printf("  Aucune reservation.\n"); return; }
    noeud_r *temp=tete;
    printf("\n");
    afficher_ligne('-', 60);
    printf("  %-5s %-6s %-8s %-12s %-7s\n",
           "ID","Vol","Passager","Date","Places");
    afficher_ligne('-', 60);
    while(temp!=NULL){
        printf("  %-5d %-6d %-8d %-12s %-7d\n",
               temp->data.id,
               temp->data.id_vol,
               temp->data.id_passager,
               temp->data.date_reservation,
               temp->data.nb_places);
        temp=temp->next;
    }
    afficher_ligne('-', 60);
}

noeud_r* SupprimerReservation(noeud_r*tete,noeud_v*listeV,int id){
    if(tete==NULL){ printf("  Aucune reservation a supprimer, la liste est vide.\n"); return tete; }
    noeud_r*temp=tete;
    noeud_r*prev=NULL;
    while(temp!=NULL&&temp->data.id!=id){ prev=temp; temp=temp->next; }
    if(temp==NULL){ printf("  [!] Aucune reservation avec l'ID %d.\n", id); return tete; }
    noeud_v*vol=chercherVol(listeV,temp->data.id_vol);
    if(vol==NULL) printf("  [!] Attention: vol lie introuvable, places non restituees.\n");
    else vol->data.nb_places+=temp->data.nb_places;
    if(prev==NULL) tete=temp->next;
    else prev->next=temp->next;
    free(temp);
    printf("  [OK] Reservation supprimee avec succes!\n");
    return tete;
}

/* ============================================================
   FONCTIONS DE SAISIE METIER
   ============================================================ */
vol saisir_vol(){
    vol v;
    v.id = compteur_vol++;
    printf("\n  --- Nouveau vol (ID auto: %d) ---\n", v.id);
    saisir_chaine("  Ville de depart   : ", v.ville_depart, 100);
    saisir_chaine("  Ville d'arrivee   : ", v.ville_arrivee, 100);
    saisir_heure ("  Heure de depart   : ", v.heure_depart);
    saisir_heure ("  Heure d'arrivee   : ", v.heure_arrivee);
    v.nb_places = saisir_entier("  Nombre de places  : ", 1, 9999);
    return v;
}

passager saisir_passager(){
    passager p;
    p.id = compteur_passager++;
    printf("\n  --- Nouveau passager (ID auto: %d) ---\n", p.id);
    p.age = saisir_entier("  Age              : ", 1, 120);
    saisir_chaine("  Nom              : ", p.nom, 50);
    saisir_chaine("  Prenom           : ", p.prenom, 50);
    saisir_chaine("  Num. passeport   : ", p.num_passeport, 50);
    saisir_chaine("  Nationalite      : ", p.nationalite, 50);
    return p;
}

reservation saisir_reservation(){
    reservation r;
    /* L'ID sera attribue apres validation dans AjoutReservation */
    r.id = 0;
    printf("\n  --- Nouvelle reservation ---\n");
    r.id_vol      = saisir_entier("  ID du vol        : ", 1, 99999);
    r.id_passager = saisir_entier("  ID du passager   : ", 1, 99999);
    r.nb_places   = saisir_entier("  Nombre de places : ", 1, 9999);
    saisir_date("  Date (YYYY-MM-DD) : ", r.date_reservation);
    return r;
}

/* ============================================================
   MENUS
   ============================================================ */
void afficher_entete(const char *titre){
    printf("\n");
    afficher_ligne('=', 60);
    printf("  SkyManager  |  %s\n", titre);
    afficher_ligne('=', 60);
}

void menu_vols(noeud_v**listeV, noeud_r**listeR){
    int choix;
    do {
        afficher_entete("GESTION DES VOLS");
        printf("  [1] Ajouter un vol\n");
        printf("  [2] Afficher tous les vols\n");
        printf("  [3] Rechercher un vol\n");
        printf("  [4] Modifier un vol\n");
        printf("  [5] Supprimer un vol\n");
        printf("  [0] Retour au menu principal\n");
        afficher_ligne('-', 60);
        choix = saisir_entier("  Votre choix : ", 0, 5);

        switch(choix){
            case 1:{
                vol v = saisir_vol();
                *listeV = AjoutVol(*listeV, v);
                attendre_entree();
                break;
            }
            case 2:{
                afficher_entete("LISTE DES VOLS");
                afficherVols(*listeV);
                attendre_entree();
                break;
            }
            case 3:{
                afficher_entete("RECHERCHE VOL");
                afficherVols(*listeV);
                int id = saisir_entier("  ID du vol a rechercher : ", 1, 99999);
                noeud_v* v = chercherVol(*listeV, id);
                if(v==NULL){
                    printf("  [!] Aucun vol avec l'ID %d.\n", id);
                } else {
                    afficher_ligne('-', 60);
                    printf("  ID       : %d\n", v->data.id);
                    printf("  Depart   : %s\n", v->data.ville_depart);
                    printf("  Arrivee  : %s\n", v->data.ville_arrivee);
                    printf("  H.Dep.   : %s\n", v->data.heure_depart);
                    printf("  H.Arr.   : %s\n", v->data.heure_arrivee);
                    printf("  Places   : %d\n", v->data.nb_places);
                    afficher_ligne('-', 60);
                }
                attendre_entree();
                break;
            }
            case 4:{
                afficher_entete("MODIFIER VOL");
                afficherVols(*listeV);
                int id = saisir_entier("  ID du vol a modifier : ", 1, 99999);
                *listeV = ModifierVol(*listeV, id);
                attendre_entree();
                break;
            }
            case 5:{
                afficher_entete("SUPPRIMER VOL");
                afficherVols(*listeV);
                int id = saisir_entier("  ID du vol a supprimer : ", 1, 99999);
                printf("  [?] Confirmer la suppression du vol %d et toutes ses reservations ? (1=Oui / 0=Non) : ", id);
                int conf = saisir_entier("", 0, 1);
                if(conf == 1) *listeV = SupprimerVol(*listeV, listeR, id);
                else printf("  Suppression annulee.\n");
                attendre_entree();
                break;
            }
        }
    } while(choix != 0);
}

void menu_passagers(noeud_p**listeP, noeud_r**listeR, noeud_v*listeV){
    int choix;
    do {
        afficher_entete("GESTION DES PASSAGERS");
        printf("  [1] Ajouter un passager\n");
        printf("  [2] Afficher tous les passagers\n");
        printf("  [3] Rechercher un passager\n");
        printf("  [4] Modifier un passager\n");
        printf("  [5] Supprimer un passager\n");
        printf("  [0] Retour au menu principal\n");
        afficher_ligne('-', 60);
        choix = saisir_entier("  Votre choix : ", 0, 5);

        switch(choix){
            case 1:{
                passager p = saisir_passager();
                *listeP = AjoutPassager(*listeP, p);
                attendre_entree();
                break;
            }
            case 2:{
                afficher_entete("LISTE DES PASSAGERS");
                afficherPassagers(*listeP);
                attendre_entree();
                break;
            }
            case 3:{
                afficher_entete("RECHERCHE PASSAGER");
                afficherPassagers(*listeP);
                int id = saisir_entier("  ID du passager a rechercher : ", 1, 99999);
                noeud_p* p = chercherPassager(*listeP, id);
                if(p==NULL){
                    printf("  [!] Aucun passager avec l'ID %d.\n", id);
                } else {
                    afficher_ligne('-', 60);
                    printf("  ID          : %d\n",  p->data.id);
                    printf("  Nom         : %s\n",  p->data.nom);
                    printf("  Prenom      : %s\n",  p->data.prenom);
                    printf("  Age         : %d\n",  p->data.age);
                    printf("  Passeport   : %s\n",  p->data.num_passeport);
                    printf("  Nationalite : %s\n",  p->data.nationalite);
                    afficher_ligne('-', 60);
                }
                attendre_entree();
                break;
            }
            case 4:{
                afficher_entete("MODIFIER PASSAGER");
                afficherPassagers(*listeP);
                int id = saisir_entier("  ID du passager a modifier : ", 1, 99999);
                *listeP = ModifierPassager(*listeP, id);
                attendre_entree();
                break;
            }
            case 5:{
                afficher_entete("SUPPRIMER PASSAGER");
                afficherPassagers(*listeP);
                int id = saisir_entier("  ID du passager a supprimer : ", 1, 99999);
                printf("  [?] Confirmer la suppression du passager %d et toutes ses reservations ? (1=Oui / 0=Non) : ", id);
                int conf = saisir_entier("", 0, 1);
                if(conf == 1) *listeP = SupprimerPassager(*listeP, listeR, listeV, id);
                else printf("  Suppression annulee.\n");
                attendre_entree();
                break;
            }
        }
    } while(choix != 0);
}

void menu_reservations(noeud_r**listeR, noeud_v*listeV, noeud_p*listeP){
    int choix;
    do {
        afficher_entete("GESTION DES RESERVATIONS");
        printf("  [1] Ajouter une reservation\n");
        printf("  [2] Afficher toutes les reservations\n");
        printf("  [3] Supprimer une reservation\n");
        printf("  [0] Retour au menu principal\n");
        afficher_ligne('-', 60);
        choix = saisir_entier("  Votre choix : ", 0, 3);

        switch(choix){
            case 1:{
                afficher_entete("NOUVELLE RESERVATION");
                printf("  Vols disponibles :\n");
                afficherVols(listeV);
                printf("  Passagers enregistres :\n");
                afficherPassagers(listeP);
                reservation r = saisir_reservation();
                *listeR = AjoutReservation(*listeR, listeV, listeP, r);
                attendre_entree();
                break;
            }
            case 2:{
                afficher_entete("LISTE DES RESERVATIONS");
                afficherReservations(*listeR);
                attendre_entree();
                break;
            }
            case 3:{
                afficher_entete("SUPPRIMER RESERVATION");
                afficherReservations(*listeR);
                int id = saisir_entier("  ID de la reservation a supprimer : ", 1, 99999);
                printf("  [?] Confirmer la suppression ? (1=Oui / 0=Non) : ");
                int conf = saisir_entier("", 0, 1);
                if(conf == 1) *listeR = SupprimerReservation(*listeR, listeV, id);
                else printf("  Suppression annulee.\n");
                attendre_entree();
                break;
            }
        }
    } while(choix != 0);
}

/* ============================================================
   LIBERATION MEMOIRE
   ============================================================ */
void liberer_vols(noeud_v *tete){
    noeud_v *tmp;
    while(tete != NULL){ tmp = tete; tete = tete->next; free(tmp); }
}

void liberer_passagers(noeud_p *tete){
    noeud_p *tmp;
    while(tete != NULL){ tmp = tete; tete = tete->next; free(tmp); }
}

void liberer_reservations(noeud_r *tete){
    noeud_r *tmp;
    while(tete != NULL){ tmp = tete; tete = tete->next; free(tmp); }
}

/* ============================================================
   MAIN
   ============================================================ */
int main(){
    noeud_v* listeV = NULL;
    noeud_p* listeP = NULL;
    noeud_r* listeR = NULL;

    int choix;
    do {
        afficher_entete("MENU PRINCIPAL");
        printf("  [1] Gestion des Vols\n");
        printf("  [2] Gestion des Passagers\n");
        printf("  [3] Gestion des Reservations\n");
        afficher_ligne('-', 60);
        printf("  [0] Quitter\n");
        afficher_ligne('=', 60);
        choix = saisir_entier("  Votre choix : ", 0, 3);

        switch(choix){
            case 1: menu_vols(&listeV, &listeR);               break;
            case 2: menu_passagers(&listeP, &listeR, listeV);  break;
            case 3: menu_reservations(&listeR, listeV, listeP); break;
            case 0:
                afficher_ligne('=', 60);
                printf("  Au revoir!\n");
                afficher_ligne('=', 60);
                break;
        }
    } while(choix != 0);

    /* Liberation de toute la memoire allouee dynamiquement */
    liberer_reservations(listeR);
    liberer_passagers(listeP);
    liberer_vols(listeV);

    return 0;
}
