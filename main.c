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
   AUTO-INCREMENTED ID COUNTERS
   ============================================================ */
int compteur_vol = 1;
int compteur_passager = 1;
int compteur_reservation = 1;

/* ============================================================
   UTILITY FUNCTIONS / INPUT VALIDATION
   ============================================================ */

/* Clear keyboard buffer */
void vider_buffer(){
    int c;
    while((c = getchar()) != '\n' && c != EOF);
}

/* Print a separator line */
void afficher_ligne(char c, int n){
    for(int i=0; i<n; i++) printf("%c", c);
    printf("\n");
}

/* Read an integer with validation (min/max) */
int saisir_entier(const char *message, int min, int max){
    int valeur;
    int ok = 0;
    do {
        printf("%s", message);
        if(scanf("%d", &valeur) != 1){
            printf("  [!] Invalid input. Please enter an integer.\n");
            vider_buffer();
            ok = 0;
        } else if(valeur < min || valeur > max){
            printf("  [!] Value out of bounds (%d - %d).\n", min, max);
            vider_buffer();
            ok = 0;
        } else {
            vider_buffer();
            ok = 1;
        }
    } while(!ok);
    return valeur;
}

/* Read a non-empty string */
void saisir_chaine(const char *message, char *dest, int taille){
    int ok = 0;
    do {
        printf("%s", message);
        if(fgets(dest, taille, stdin) == NULL){
            dest[0] = '\0';
        }
        /* Remove trailing newline */
        int len = strlen(dest);
        if(len > 0 && dest[len-1] == '\n') dest[len-1] = '\0';
        /* Check if not empty */
        if(strlen(dest) == 0){
            printf("  [!] This field cannot be empty.\n");
            ok = 0;
        } else {
            ok = 1;
        }
    } while(!ok);
}

/* Validate HH:MM time format */
int valider_heure(const char *heure){
    if(strlen(heure) != 5) return 0;
    if(!isdigit(heure[0]) || !isdigit(heure[1])) return 0;
    if(heure[2] != ':') return 0;
    if(!isdigit(heure[3]) || !isdigit(heure[4])) return 0;
    int h = (heure[0]-'0')*10 + (heure[1]-'0');
    int m = (heure[3]-'0')*10 + (heure[4]-'0');
    return (h >= 0 && h <= 23 && m >= 0 && m <= 59);
}

/* Read a time in HH:MM format */
void saisir_heure(const char *message, char *dest){
    int ok = 0;
    do {
        printf("%s", message);
        if(fgets(dest, 10, stdin) == NULL) dest[0] = '\0';
        int len = strlen(dest);
        if(len > 0 && dest[len-1] == '\n') dest[len-1] = '\0';
        if(!valider_heure(dest)){
            printf("  [!] Invalid format. Use HH:MM (e.g., 08:30).\n");
            ok = 0;
        } else {
            ok = 1;
        }
    } while(!ok);
}

/* Validate YYYY-MM-DD date format with strict day/month check */
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
    /* Maximum days per month */
    int jours_max[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    /* Leap year check: divisible by 4, except centuries not divisible by 400 */
    int bissextile = (annee % 4 == 0 && (annee % 100 != 0 || annee % 400 == 0));
    if(mois == 2 && bissextile) jours_max[2] = 29;
    return (jour <= jours_max[mois]);
}

/* Read a date in YYYY-MM-DD format */
void saisir_date(const char *message, char *dest){
    int ok = 0;
    do {
        printf("%s", message);
        if(fgets(dest, 20, stdin) == NULL) dest[0] = '\0';
        int len = strlen(dest);
        if(len > 0 && dest[len-1] == '\n') dest[len-1] = '\0';
        if(!valider_date(dest)){
            printf("  [!] Invalid format. Use YYYY-MM-DD (e.g., 2025-06-15).\n");
            ok = 0;
        } else {
            ok = 1;
        }
    } while(!ok);
}

/* Wait for user to press Enter */
void attendre_entree(){
    printf("\n  Press Enter to continue...");
    vider_buffer();
}

/* ============================================================
   FLIGHT MANAGEMENT FUNCTIONS
   ============================================================ */
noeud_v* creer_vol(vol v){
    noeud_v*newE=(noeud_v*)malloc(sizeof(noeud_v));
    if(newE==NULL){ printf("Allocation failed!\n"); return newE; }
    newE->data=v;
    newE->next=NULL;
    return newE;
}

noeud_v* AjoutVol(noeud_v *tete,vol v){
    noeud_v *newE=creer_vol(v);
    if(newE==NULL){ printf("Allocation failed!\n"); return tete; }
    if(tete==NULL||strcmp(v.heure_depart,tete->data.heure_depart)<0){
        newE->next=tete; tete=newE;
        printf("  [OK] Flight added successfully!\n");
        return tete;
    }
    noeud_v*temp=tete;
    while(temp->next!=NULL && strcmp(temp->next->data.heure_depart,v.heure_depart)<0)
        temp=temp->next;
    newE->next=temp->next;
    temp->next=newE;
    printf("  [OK] Flight added successfully!\n");
    return tete;
}

void afficherVols(noeud_v *tete){
    if(tete==NULL){ printf("  No flights available.\n"); return; }
    noeud_v *temp=tete;
    printf("\n");
    afficher_ligne('-', 72);
    printf("  %-4s %-20s %-20s %-8s %-8s %-6s\n",
           "ID","Departure","Arrival","Dep.Time","Arr.Time","Seats");
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
    printf("\n  --- Modify Flight ---\n");
    saisir_chaine("  New departure city  : ", v->ville_depart, 100);
    saisir_chaine("  New arrival city    : ", v->ville_arrivee, 100);
    saisir_heure ("  New departure time  : ", v->heure_depart);
    saisir_heure ("  New arrival time    : ", v->heure_arrivee);
    v->nb_places = saisir_entier("  New number of seats : ", 1, 9999);
    printf("  [OK] Flight modified successfully!\n");
}

noeud_v* ModifierVol(noeud_v*tete,int idf){
    if(tete==NULL){ printf("  No flights to modify, list is empty.\n"); return tete; }
    noeud_v* vol_modif=chercherVol(tete,idf);
    if(vol_modif==NULL){ printf("  [!] No flight with ID %d.\n", idf); return tete; }
    saisie_modif(&vol_modif->data);
    return tete;
}

noeud_v* SupprimerVol(noeud_v*tete,noeud_r**listeR,int id){
    if(tete==NULL){ printf("  No flights to delete, list is empty.\n"); return tete; }
    noeud_v*tempV=tete;
    noeud_v*prevV=NULL;
    while(tempV!=NULL&&tempV->data.id!=id){ prevV=tempV; tempV=tempV->next; }
    if(tempV==NULL){ printf("  [!] No flight with ID %d.\n", id); return tete; }
    /* Delete all reservations linked to this flight */
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
    /* Delete the flight itself */
    if(prevV==NULL) tete=tete->next;
    else prevV->next=tempV->next;
    free(tempV);
    if(nb_res_supprimees > 0)
        printf("  [i] %d related reservation(s) deleted.\n", nb_res_supprimees);
    printf("  [OK] Flight and its reservations deleted!\n");
    return tete;
}

/* ============================================================
   PASSENGER MANAGEMENT FUNCTIONS
   ============================================================ */
noeud_p* creer_passager(passager p){
    noeud_p*newE=(noeud_p*)malloc(sizeof(noeud_p));
    if(newE==NULL){ printf("Allocation failed!\n"); return newE; }
    newE->data=p;
    newE->next=NULL;
    return newE;
}

noeud_p* AjoutPassager(noeud_p *tete,passager p){
    noeud_p *newE=creer_passager(p);
    if(newE==NULL){ printf("Allocation failed!\n"); return tete; }
    if(tete==NULL||p.id<tete->data.id){
        newE->next=tete; tete=newE;
        printf("  [OK] Passenger added successfully!\n");
        return tete;
    }
    noeud_p*temp=tete;
    while(temp->next!=NULL && temp->next->data.id<p.id)
        temp=temp->next;
    newE->next=temp->next;
    temp->next=newE;
    printf("  [OK] Passenger added successfully!\n");
    return tete;
}

void afficherPassagers(noeud_p *tete){
    if(tete==NULL){ printf("  No passengers available.\n"); return; }
    noeud_p *temp=tete;
    printf("\n");
    afficher_ligne('-', 78);
    printf("  %-4s %-15s %-15s %-4s %-12s %-15s\n",
           "ID","Last Name","First Name","Age","Passport","Nationality");
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
    printf("\n  --- Modify Passenger ---\n");
    p->age = saisir_entier("  New age            : ", 1, 120);
    saisir_chaine("  New last name      : ", p->nom, 50);
    saisir_chaine("  New first name     : ", p->prenom, 50);
    saisir_chaine("  New passport number: ", p->num_passeport, 50);
    saisir_chaine("  New nationality    : ", p->nationalite, 50);
    printf("  [OK] Passenger modified successfully!\n");
}

noeud_p* ModifierPassager(noeud_p*tete,int idf){
    if(tete==NULL){ printf("  No passengers to modify, list is empty.\n"); return tete; }
    noeud_p* psg_modif=chercherPassager(tete,idf);
    if(psg_modif==NULL){ printf("  [!] No passenger with ID %d.\n", idf); return tete; }
    saisie_modif_P(&psg_modif->data);
    return tete;
}

noeud_p* SupprimerPassager(noeud_p*tete,noeud_r**listeR,noeud_v*listeV,int id){
    if(tete==NULL){ printf("  No passengers to delete, list is empty.\n"); return tete; }
    noeud_p*tempP=tete;
    noeud_p*prevP=NULL;
    while(tempP!=NULL&&tempP->data.id!=id){ prevP=tempP; tempP=tempP->next; }
    if(tempP==NULL){ printf("  [!] No passenger with ID %d.\n", id); return tete; }
    noeud_r*tempR=*listeR;
    noeud_r*prevR=NULL;
    while(tempR!=NULL){
        if(tempR->data.id_passager==id){
            noeud_r*sup=tempR;
            /* FIX: Return reserved seats to the flight */
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
    printf("  [OK] Passenger and their reservations deleted!\n");
    return tete;
}

/* ============================================================
   RESERVATION MANAGEMENT FUNCTIONS
   ============================================================ */
noeud_r* creer_reservation(reservation r){
    noeud_r*newE=(noeud_r*)malloc(sizeof(noeud_r));
    if(newE==NULL){ printf("Allocation failed!\n"); return newE; }
    newE->data=r;
    newE->next=NULL;
    return newE;
}

noeud_r* AjoutReservationTriee(noeud_r *tete,reservation r){
    noeud_r *newE=creer_reservation(r);
    if(newE==NULL){ printf("Allocation failed!\n"); return tete; }
    if(tete==NULL||strcmp(r.date_reservation,tete->data.date_reservation)<0){
        newE->next=tete; tete=newE;
        printf("  [OK] Reservation completed successfully!\n");
        return tete;
    }
    noeud_r*temp=tete;
    while(temp->next!=NULL && strcmp(temp->next->data.date_reservation,r.date_reservation)<0)
        temp=temp->next;
    newE->next=temp->next;
    temp->next=newE;
    printf("  [OK] Reservation completed successfully!\n");
    return tete;
}

noeud_r* AjoutReservation(noeud_r*tete,noeud_v*listeV,noeud_p*listeP,reservation r){
    noeud_v*vol=chercherVol(listeV,r.id_vol);
    if(vol==NULL){ printf("  [!] Flight not found (ID %d), reservation failed!\n", r.id_vol); return tete; }
    noeud_p *passager=chercherPassager(listeP,r.id_passager);
    if(passager==NULL){ printf("  [!] Passenger not found (ID %d), reservation failed!\n", r.id_passager); return tete; }
    if(vol->data.nb_places < r.nb_places){
        printf("  [!] Insufficient seats (%d available, %d requested).\n", vol->data.nb_places, r.nb_places);
        return tete;
    }
    /* All validations passed: assign ID and create reservation */
    r.id = compteur_reservation++;
    printf("  [i] Auto-assigned Reservation ID: %d\n", r.id);
    vol->data.nb_places-=r.nb_places;
    tete=AjoutReservationTriee(tete,r);
    return tete;
}

void afficherReservations(noeud_r *tete){
    if(tete==NULL){ printf("  No reservations found.\n"); return; }
    noeud_r *temp=tete;
    printf("\n");
    afficher_ligne('-', 60);
    printf("  %-5s %-6s %-8s %-12s %-7s\n",
           "ID","Flight","Passenger","Date","Seats");
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
    if(tete==NULL){ printf("  No reservations to delete, list is empty.\n"); return tete; }
    noeud_r*temp=tete;
    noeud_r*prev=NULL;
    while(temp!=NULL&&temp->data.id!=id){ prev=temp; temp=temp->next; }
    if(temp==NULL){ printf("  [!] No reservation with ID %d.\n", id); return tete; }
    noeud_v*vol=chercherVol(listeV,temp->data.id_vol);
    if(vol==NULL) printf("  [!] Warning: Linked flight not found, seats not returned.\n");
    else vol->data.nb_places+=temp->data.nb_places;
    if(prev==NULL) tete=temp->next;
    else prev->next=temp->next;
    free(temp);
    printf("  [OK] Reservation deleted successfully!\n");
    return tete;
}

/* ============================================================
   ENTITY INPUT FUNCTIONS
   ============================================================ */
vol saisir_vol(){
    vol v;
    v.id = compteur_vol++;
    printf("\n  --- New Flight (Auto ID: %d) ---\n", v.id);
    saisir_chaine("  Departure city   : ", v.ville_depart, 100);
    saisir_chaine("  Arrival city     : ", v.ville_arrivee, 100);
    saisir_heure ("  Departure time   : ", v.heure_depart);
    saisir_heure ("  Arrival time     : ", v.heure_arrivee);
    v.nb_places = saisir_entier("  Number of seats  : ", 1, 9999);
    return v;
}

passager saisir_passager(){
    passager p;
    p.id = compteur_passager++;
    printf("\n  --- New Passenger (Auto ID: %d) ---\n", p.id);
    p.age = saisir_entier("  Age              : ", 1, 120);
    saisir_chaine("  Last name        : ", p.nom, 50);
    saisir_chaine("  First name       : ", p.prenom, 50);
    saisir_chaine("  Passport number  : ", p.num_passeport, 50);
    saisir_chaine("  Nationality      : ", p.nationalite, 50);
    return p;
}

reservation saisir_reservation(){
    reservation r;
    /* ID will be assigned after validation in AjoutReservation */
    r.id = 0;
    printf("\n  --- New Reservation ---\n");
    r.id_vol      = saisir_entier("  Flight ID        : ", 1, 99999);
    r.id_passager = saisir_entier("  Passenger ID     : ", 1, 99999);
    r.nb_places   = saisir_entier("  Number of seats  : ", 1, 9999);
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
        afficher_entete("FLIGHT MANAGEMENT");
        printf("  [1] Add a flight\n");
        printf("  [2] Display all flights\n");
        printf("  [3] Search for a flight\n");
        printf("  [4] Modify a flight\n");
        printf("  [5] Delete a flight\n");
        printf("  [0] Back to main menu\n");
        afficher_ligne('-', 60);
        choix = saisir_entier("  Your choice : ", 0, 5);

        switch(choix){
            case 1:{
                vol v = saisir_vol();
                *listeV = AjoutVol(*listeV, v);
                attendre_entree();
                break;
            }
            case 2:{
                afficher_entete("FLIGHT LIST");
                afficherVols(*listeV);
                attendre_entree();
                break;
            }
            case 3:{
                afficher_entete("FLIGHT SEARCH");
                afficherVols(*listeV);
                int id = saisir_entier("  Flight ID to search : ", 1, 99999);
                noeud_v* v = chercherVol(*listeV, id);
                if(v==NULL){
                    printf("  [!] No flight with ID %d.\n", id);
                } else {
                    afficher_ligne('-', 60);
                    printf("  ID         : %d\n", v->data.id);
                    printf("  Departure  : %s\n", v->data.ville_depart);
                    printf("  Arrival    : %s\n", v->data.ville_arrivee);
                    printf("  Dep. Time  : %s\n", v->data.heure_depart);
                    printf("  Arr. Time  : %s\n", v->data.heure_arrivee);
                    printf("  Seats      : %d\n", v->data.nb_places);
                    afficher_ligne('-', 60);
                }
                attendre_entree();
                break;
            }
            case 4:{
                afficher_entete("MODIFY FLIGHT");
                afficherVols(*listeV);
                int id = saisir_entier("  Flight ID to modify : ", 1, 99999);
                *listeV = ModifierVol(*listeV, id);
                attendre_entree();
                break;
            }
            case 5:{
                afficher_entete("DELETE FLIGHT");
                afficherVols(*listeV);
                int id = saisir_entier("  Flight ID to delete : ", 1, 99999);
                printf("  [?] Confirm deletion of flight %d and all its reservations? (1=Yes / 0=No) : ", id);
                int conf = saisir_entier("", 0, 1);
                if(conf == 1) *listeV = SupprimerVol(*listeV, listeR, id);
                else printf("  Deletion cancelled.\n");
                attendre_entree();
                break;
            }
        }
    } while(choix != 0);
}

void menu_passagers(noeud_p**listeP, noeud_r**listeR, noeud_v*listeV){
    int choix;
    do {
        afficher_entete("PASSENGER MANAGEMENT");
        printf("  [1] Add a passenger\n");
        printf("  [2] Display all passengers\n");
        printf("  [3] Search for a passenger\n");
        printf("  [4] Modify a passenger\n");
        printf("  [5] Delete a passenger\n");
        printf("  [0] Back to main menu\n");
        afficher_ligne('-', 60);
        choix = saisir_entier("  Your choice : ", 0, 5);

        switch(choix){
            case 1:{
                passager p = saisir_passager();
                *listeP = AjoutPassager(*listeP, p);
                attendre_entree();
                break;
            }
            case 2:{
                afficher_entete("PASSENGER LIST");
                afficherPassagers(*listeP);
                attendre_entree();
                break;
            }
            case 3:{
                afficher_entete("PASSENGER SEARCH");
                afficherPassagers(*listeP);
                int id = saisir_entier("  Passenger ID to search : ", 1, 99999);
                noeud_p* p = chercherPassager(*listeP, id);
                if(p==NULL){
                    printf("  [!] No passenger with ID %d.\n", id);
                } else {
                    afficher_ligne('-', 60);
                    printf("  ID          : %d\n",  p->data.id);
                    printf("  Last Name   : %s\n",  p->data.nom);
                    printf("  First Name  : %s\n",  p->data.prenom);
                    printf("  Age         : %d\n",  p->data.age);
                    printf("  Passport    : %s\n",  p->data.num_passeport);
                    printf("  Nationality : %s\n",  p->data.nationalite);
                    afficher_ligne('-', 60);
                }
                attendre_entree();
                break;
            }
            case 4:{
                afficher_entete("MODIFY PASSENGER");
                afficherPassagers(*listeP);
                int id = saisir_entier("  Passenger ID to modify : ", 1, 99999);
                *listeP = ModifierPassager(*listeP, id);
                attendre_entree();
                break;
            }
            case 5:{
                afficher_entete("DELETE PASSENGER");
                afficherPassagers(*listeP);
                int id = saisir_entier("  Passenger ID to delete : ", 1, 99999);
                printf("  [?] Confirm deletion of passenger %d and all their reservations? (1=Yes / 0=No) : ", id);
                int conf = saisir_entier("", 0, 1);
                if(conf == 1) *listeP = SupprimerPassager(*listeP, listeR, listeV, id);
                else printf("  Deletion cancelled.\n");
                attendre_entree();
                break;
            }
        }
    } while(choix != 0);
}

void menu_reservations(noeud_r**listeR, noeud_v*listeV, noeud_p*listeP){
    int choix;
    do {
        afficher_entete("RESERVATION MANAGEMENT");
        printf("  [1] Add a reservation\n");
        printf("  [2] Display all reservations\n");
        printf("  [3] Delete a reservation\n");
        printf("  [0] Back to main menu\n");
        afficher_ligne('-', 60);
        choix = saisir_entier("  Your choice : ", 0, 3);

        switch(choix){
            case 1:{
                afficher_entete("NEW RESERVATION");
                printf("  Available flights:\n");
                afficherVols(listeV);
                printf("  Registered passengers:\n");
                afficherPassagers(listeP);
                reservation r = saisir_reservation();
                *listeR = AjoutReservation(*listeR, listeV, listeP, r);
                attendre_entree();
                break;
            }
            case 2:{
                afficher_entete("RESERVATION LIST");
                afficherReservations(*listeR);
                attendre_entree();
                break;
            }
            case 3:{
                afficher_entete("DELETE RESERVATION");
                afficherReservations(*listeR);
                int id = saisir_entier("  Reservation ID to delete : ", 1, 99999);
                printf("  [?] Confirm deletion? (1=Yes / 0=No) : ");
                int conf = saisir_entier("", 0, 1);
                if(conf == 1) *listeR = SupprimerReservation(*listeR, listeV, id);
                else printf("  Deletion cancelled.\n");
                attendre_entree();
                break;
            }
        }
    } while(choix != 0);
}

/* ============================================================
   MEMORY CLEANUP
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
   MAIN FUNCTION
   ============================================================ */
int main(){
    noeud_v* listeV = NULL;
    noeud_p* listeP = NULL;
    noeud_r* listeR = NULL;

    int choix;
    do {
        afficher_entete("MAIN MENU");
        printf("  [1] Flight Management\n");
        printf("  [2] Passenger Management\n");
        printf("  [3] Reservation Management\n");
        afficher_ligne('-', 60);
        printf("  [0] Quit\n");
        afficher_ligne('=', 60);
        choix = saisir_entier("  Your choice : ", 0, 3);

        switch(choix){
            case 1: menu_vols(&listeV, &listeR);               break;
            case 2: menu_passagers(&listeP, &listeR, listeV);  break;
            case 3: menu_reservations(&listeR, listeV, listeP); break;
            case 0:
                afficher_ligne('=', 60);
                printf("  Goodbye!\n");
                afficher_ligne('=', 60);
                break;
        }
    } while(choix != 0);

    /* Free all dynamically allocated memory */
    liberer_reservations(listeR);
    liberer_passagers(listeP);
    liberer_vols(listeV);

    return 0;
}
