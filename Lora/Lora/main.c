#include <stdio.h>
#include <time.h>   
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

// renvoie une V.A. continue dans ]0..1[
double unifC() {
	double x = 0;
	while(x == 0){
		x = (double)rand()/RAND_MAX;
	}
	return x;
}

// renvoie une V.A. de loi exp(lambda)
double Expo_Duree(double lambda) {
	return -log(unifC())/lambda;
}

#define e ((double)10.0) // pour le calcul du temps d'emission
#define w ((double)0.25) // pour le calcul du temps d'attente apres collisiom
#define I ((double)0.1)  // pour le calcul du temps d'attente avant emission au premir coup
//#define K 10 // le nombre de capteurs
#define NMAX 1e3 // condition d'arret
#define R 50 // le nombre de simulation a faire pour estimer l'intervalle de confaince


// les types d'evenements de notre simulation
enum type_event { Demission, Femission };

// la structure d'un evenement
struct event{
	int numero_capteur; // le numero du capteur qui execute l'evenement.
    float date_arrive; // Date d'arrive de l'evenement
    enum type_event type; // Type de l'evenement
} ;

// l'echeancier sous forme d'une liste lineaire chaine
struct noeud{
   struct event event;
   struct noeud *suivant;
};


struct noeud *tete = NULL; // la tete de l'echeancier
struct noeud *queue = NULL; // l'evenement dernier de l'echeancier.
struct noeud *current = NULL; // l'evenement actuel de l'echeancier.



int canal; // le canal de transmission ( libre = 1 ou occupe = 0 )
int num_capteur_en_emission; // le numero du capteur entrain d'emmetre un packet.
int K;// le nombre de capteurs
float Time; // temps de la simulation
float moyenne;
float last_time;
float temps_debut_emission;
float temps_debut_emission_e2;
float dernier_temps;
float temps_experience;
float temps_emission_e1;
float temps_emission_e1_last;
float temps_emission_e1_total;
float temps_emission_e2;
float temps_emission_e2_last;
float temps_emission_e2_total;
int emission_e1; // variable booleene qui servira a calculer le temps d'emission valideé de e1.
int emission_e2;
int nb_events;
int Nb_packets; // nombre de packets envoyes totale
int nb_packets_e1;
int nb_packets_e2;
int nb_collisions[7]; // nombre de collisions de chaque etat;
int nb_tentatives[7];//nombre de tentative d'envoie de chaque etat
float nb_tentatives_total;
float nb_messages_envoye;
float nb_collisions_total;
enum etat_capteur { i, e1, e2, e3, e4, e5, e6, e7,
				 w1, w2, w3, w4, w5, w6};			
enum etat_capteur capteurs[1000]; // k capteurs stocke dans un tableau.
int nb_packets_par_capteurs[1000];



//------------------------------------------------------ Les fonctions -------------------------------------------------------------

// Allouer de la memoire pour un evenement for tout en l'initialisant 
struct event *initEvent() {
    struct event *evn = (struct event*)malloc(sizeof(struct event));
    evn->date_arrive = 0;
    evn->numero_capteur = 0;
    return evn;
}
// Ajouter un evenement a l'echeancier en dernier
void ajouter_event(struct event evn) {	
   struct noeud *link = (struct noeud*) malloc(sizeof(struct noeud));
   link->event = evn;
   link->suivant = NULL;
   //pointer le dernier noeud au nouveau.
   if(tete == NULL){
   	tete = link;
   	tete->suivant = NULL;
   	queue = tete;
   }else{
   	queue->suivant = link;
   	queue = link;
   }
	nb_events = nb_events + 1;	
}
void parcours_echeancier(){
	current = tete;
	if(current != NULL){
		while(current != NULL){
			printf("%.2lf | %d | ",current->event.date_arrive, current->event.numero_capteur);
			current = current->suivant;
		}
	}
	printf(" \n");
}
// extraire et supprimer de l'echenacier l'evenement avec la date la plus petite
struct event extraire_event()
{
	float mintime = tete->event.date_arrive;
      //On va l'utiliser pour liberer la memoire
      struct noeud *before_noeud;
      struct noeud *noeud = tete;
      current  = tete;
      
	  while(current->suivant != NULL)
          {
          	if(mintime > current->suivant->event.date_arrive){
          		mintime = current->suivant->event.date_arrive;
          		before_noeud = current;
          		noeud = current->suivant;
			  }else{
			  	if(mintime == current->suivant->event.date_arrive){
			  		current->suivant->event.date_arrive = current->suivant->event.date_arrive + 0.001;//on ajoute un petit bias si les evenement arrivent exactement au meme moment
				  }
			  }
            current = current->suivant;
          }
          if(mintime == tete->event.date_arrive){
          	tete = tete->suivant; // on mets a jour la tete.
		  }else{
		  	//on libere le noeud de la liste.        
          	before_noeud->suivant = noeud->suivant;
          	if(noeud->suivant == NULL){
          		queue = before_noeud ; // on mets a jour la queue
		  	}
		  }
		  nb_events = nb_events -1;	
    return noeud->event;
}
//Supprime un evenement programme dans l'echeancier
void supprimer_event(int num_capteur, enum type_event event_type){

	struct noeud *before_noeud;
    struct noeud *noeud = tete;
    current  = tete;
    int found = 0;
    // si l'evenement est dans la tete de l'echeancier.
    if(tete->event.numero_capteur == num_capteur && tete->event.type == event_type){
    	found = 1;
    	if(tete->suivant != NULL){
    		tete = tete->suivant;
		}
		else{
			tete = NULL;
		}
	}
    // si l'evenement a supprimer n'est pas dans la tete
    while(!found){
    	before_noeud = current;
    	noeud = current->suivant;
    	if(current->suivant->event.numero_capteur == num_capteur && current->suivant->event.type == event_type){
    		found = 1;
    		if(current->suivant->suivant != NULL){
    			// on supprime le noeud de l'echeancier
    			before_noeud->suivant = current->suivant->suivant;
			}
			else{
				before_noeud->suivant = NULL;
          		queue = before_noeud ; // on mets a jour la queue
			}
		}
        current =  current->suivant;
	}
	nb_events = nb_events - 1;	
}


// le changement de l'etat apres une collision
enum etat_capteur nouveau_etat_post_collision(enum etat_capteur etat){

	switch(etat){
		case e1:
			return w1;
			break;
		case e2:
			return w2;
			break;
		case e3:
			return w3;
			break;
		case e4:
			return w4;
			break;
		case e5:
			return w5;
			break;
		case e6:
			return w6;
			break;	
		case e7:
			return i;
			break;				
	}
}
// le nouveau etat d'un capteur au debit d'emission
enum etat_capteur nouveau_etat_emission(enum etat_capteur etat){

	switch(etat){
		case i:
			return e1;
			break;
		case w1:
			return e2;
			break;
		case w2:
			return e3;
			break;
		case w3:
			return e4;
			break;
		case w4:
			return e5;
			break;
		case w5:
			return e6;
			break;	
		case w6:
			return e7;
			break;				
	}
}

char* convert_etat_capteur(enum etat_capteur etat){
	switch(etat){
		case i:
			return "i";
			break;
		case w1:
			return "w1";
			break;
		case w2:
			return "w2";
			break;
		case w3:
			return "w3";
			break;
		case w4:
			return "w4";
			break;
		case w5:
			return "w5";
			break;	
		case w6:
			return "w6";
			break;	
		case e1:
			return "e1";
			break;
		case e2:
			return "e2";
			break;
		case e3:
			return "e3";
			break;
		case e4:
			return "e4";
			break;
		case e5:
			return "e5";
			break;
		case e6:
			return "e6";
			break;	
		case e7:
			return "e7";
			break;			
	}
}


int convert_etat_emission_to_int(enum etat_capteur etat){
		switch(etat){
		case e1:
			return 0;
			break;
		case e2:
			return 1;
			break;
		case e3:
			return 2;
			break;
		case e4:
			return 3;
			break;
		case e5:
			return 4;
			break;
		case e6:
			return 5;
			break;	
		case e7:
			return 6;
			break;	
	}
}


// traite une collision
void Traitement_Collision(int numero_victime, int numero_provocateur, float current_time){
	
	// on incremente le nombre de collisions de chaque etat.
	nb_collisions[convert_etat_emission_to_int(capteurs[numero_victime])] = nb_collisions[convert_etat_emission_to_int(capteurs[numero_victime])] + 1.0f;
	nb_collisions_total = nb_collisions_total + 1;// on inceremente le nombre de collisions totale
	nb_collisions[convert_etat_emission_to_int(capteurs[numero_provocateur])] = nb_collisions[convert_etat_emission_to_int(capteurs[numero_provocateur])] + 1.0f;
	nb_collisions_total = nb_collisions_total + 1;// on inceremente le nombre de collisions totale
	// on libere le canal.
	canal = 1; 
	// stoker le temps d'emission de la victime---------------------------------------------
	if(capteurs[numero_victime] == e1){ // on rajoute le temps d'emission de la victime
	    emission_e1 = 1;
		//temps_emission_e1 = temps_emission_e1_last + (current_time - temps_debut_emission);
		temps_emission_e1 = (current_time - temps_debut_emission);
		temps_emission_e1_last = temps_emission_e1;
	}
	if(capteurs[numero_victime] == e2){ // on rajoute le temps d'emission de la victime
	    emission_e2 = 1;
		//temps_emission_e1 = temps_emission_e1_last + (current_time - temps_debut_emission);
		temps_emission_e2 = (current_time - temps_debut_emission_e2);
		temps_emission_e2_last = temps_emission_e2;
	}
	//changer l'etat du capteur victime.
	capteurs[numero_victime] = nouveau_etat_post_collision(capteurs[numero_victime]);
	//supprimer l'evenement de fin d'emission de l'echeancier.
	supprimer_event(numero_victime, Femission);
	// on creer un nouvel evenement de debut d'emission
	struct event evn = *initEvent(); 
	if(capteurs[numero_victime] == i){
		evn.date_arrive = current_time + Expo_Duree(I);
	}else{
		evn.date_arrive = current_time + Expo_Duree(w);
	}
	evn.numero_capteur = numero_victime;
	evn.type = Demission; // on lance un debut d'emission a T+exp(I) ou T+exp(w) selon l'etat du capteur.
	ajouter_event(evn);// ajouter l'evenement a l'echeancier.
	
	// on fait le meme traitement pour le capteur provocateur.
	capteurs[numero_provocateur] = nouveau_etat_post_collision(capteurs[numero_provocateur]);//changer l'etat du capteur provocateur.
	// on creer un nouvel evenement
	struct event evn2 = *initEvent(); 
	if(capteurs[numero_provocateur] == i){
		evn2.date_arrive = current_time + Expo_Duree(I);
	}else{
		evn2.date_arrive = current_time + Expo_Duree(w);
	}
	evn2.numero_capteur = numero_provocateur;
	evn2.type = Demission; // on lance un debut d'emission a T+exp(i) ou T+exp(w) selon l'etat du capteur.
	ajouter_event(evn2);// ajouter l'evenement a l'echeancier.*/
}

// lance un evenement de debut d'emission par un capteur.
void debut_emission(struct event evenement){
	capteurs[evenement.numero_capteur] = nouveau_etat_emission(capteurs[evenement.numero_capteur]); // on mets le capteur a sa nouvel valeur
	nb_tentatives[convert_etat_emission_to_int(capteurs[evenement.numero_capteur])] = nb_tentatives[convert_etat_emission_to_int(capteurs[evenement.numero_capteur])] + 1.0f; // on incremente le nombre de tentatives d'envoie
	nb_tentatives_total = nb_tentatives_total + 1;// on incremente le nombre de tentatives totale
	Time = evenement.date_arrive; // on mets le temps actuel a la date d'arrive de l'evenement.
	if(canal == 0){
				// si le canal est occupe !! Collision !!
				Traitement_Collision(num_capteur_en_emission,evenement.numero_capteur,Time);//Traiter la collision
		}
	else{
			num_capteur_en_emission = evenement.numero_capteur; 
			canal = 0; // on occupe le canal
			struct event evn = *initEvent(); // on creer un nouvel evenement
			evn.date_arrive = Time + Expo_Duree(e);
			if(capteurs[evenement.numero_capteur] == e1){
					//temps_emission_e1 = temps_emission_e1_last + evn.date_arrive - Time; // on ajoute le temps d'emission
					//temps_debut_emission = Time;
					temps_emission_e1 = evn.date_arrive - Time;
					temps_debut_emission = Time;
				}
				if(capteurs[evenement.numero_capteur] == e2){
					//temps_emission_e1 = temps_emission_e1_last + evn.date_arrive - Time; // on ajoute le temps d'emission
					//temps_debut_emission = Time;
					temps_emission_e2 = evn.date_arrive - Time;
					temps_debut_emission_e2 = Time;
				}
			evn.numero_capteur = evenement.numero_capteur;
			evn.type = Femission; // on lance une fin d'emission a t+exp(e)
			ajouter_event(evn); 
		}
}
// lance un evenement de fin d'emission par un capteur
void fin_emission(struct event evenement){
			if(capteurs[evenement.numero_capteur] == e1){
					emission_e1 = 1;
					temps_emission_e1_last = temps_emission_e1; // on valide le temps d'emission
					nb_packets_e1 = nb_packets_e1 + 1;
			}
			if(capteurs[evenement.numero_capteur] == e2){
					emission_e2 = 1;
					temps_emission_e2_last = temps_emission_e2; // on valide le temps d'emission
					nb_packets_e2 = nb_packets_e2 + 1;
			}
			nb_messages_envoye = nb_messages_envoye + 1;
			Time = evenement.date_arrive; // on actualise le temps.
			struct event evn = *initEvent(); // on creer un nouvel evenement
			evn.date_arrive = Time + Expo_Duree(I);
			evn.numero_capteur = evenement.numero_capteur;
			evn.type = Demission; // on lance un debut d'emission a T+exp(i)
			ajouter_event(evn); // ajouter l'evenement a l'echeancier.
			canal = 1; // on libere le canal.
			num_capteur_en_emission = -1;
			Nb_packets = Nb_packets + 1; // on incremente le nombre de packets emis totale.
			capteurs[evenement.numero_capteur] = i; // on mets le capteur a i
			nb_packets_par_capteurs[evenement.numero_capteur] = nb_packets_par_capteurs[evenement.numero_capteur] + 1 ;
}
// traite un evenement
void Traitement_Event(struct event evenement){
	switch(evenement.type){
		case Demission:
				debut_emission(evenement);// on traite le debut de l'emission.
				break;
				
		case Femission:
				fin_emission(evenement);// on traite la fin de l'emission.
				break;
	}
}

void initialisation(){
	nb_tentatives_total = 0; // le nombre de tentatives d'envoie totales
	nb_collisions_total = 0; // le nombre de collisions totale entre tout les etats
	nb_messages_envoye = 0;
	nb_packets_e1 = 0;
	nb_packets_e2 = 0;
	num_capteur_en_emission = -1; // le numero du capteur entrain d'emmetre un packet.
	temps_emission_e1 = 0; // le temps total d'emission à l'etat e1
	temps_emission_e1_last = 0; // une variable qui va aider pour le calcul
    temps_emission_e1_total = 0; 
	temps_emission_e2 = 0; // le temps total d'emission à l'etat e1
    temps_emission_e2_last = 0;
    temps_emission_e2_total = 0;
    emission_e1 = 0;
    emission_e2 = 0;
    moyenne = 0.0f;
	Time = 0.0f; // temps de la simulation
	temps_debut_emission = 0.0f;
	temps_debut_emission_e2 = 0.0f;
	temps_experience = 0.0f;
	canal = 1; // le canal est libre au debut
	struct event evn;
	int l;
	for (l = 0; l < K; l++){
		evn = *initEvent();
		evn.date_arrive = Time + Expo_Duree(I);
		evn.numero_capteur = l;
		evn.type = Demission; // on lance un debut d'emission a T+exp(i)
		ajouter_event (evn);// on ajoute l'evenement a l'echeancier
		capteurs[l] = i; // en attente d'emission
		nb_packets_par_capteurs[l] = 0; // on initialise le nombre de packets envoye par capteurs a 0;
	}
	for(l=0;l<7;l++){
		nb_tentatives[l] = 0; // le nombre de tentatice de chaque etat a 0.
		nb_collisions[l] = 0; //  le nombre de collisions de chaque etat a 0.
	}
}
// la conditon d'arret de la simulation
int condition_arret( int nb_messages){
		int condition = 1;
		int f =0;
		while(f < K && condition == 1){
			if(nb_packets_par_capteurs[f] <= nb_messages)
			{
				condition = 0;
			}
			f = f+1;
		}
		return condition;
	}
float moyenne_distribution(float Tab[R]){
	int i=0;
	// Calcul de la moyenne de la distribution.
	float moyenne = 0;
	while(i<R){
		moyenne = moyenne + Tab[i];
		i= i +1;
	}
	moyenne = moyenne / (float) R;
	return moyenne;
}	
float ecart_type(float Tab[R], float moyenne){
	
	// Calcul de l'ecart type d'une distribution de R echantillon et une moyenne 
	float ecart_type = 0;
	int i = 0;
	while(i<R){
		ecart_type = ecart_type + (Tab[i] - moyenne) * (Tab[i] - moyenne);
		i= i+1;
	}
	ecart_type = ecart_type / (float) R;
	ecart_type = sqrt(ecart_type);
	return ecart_type;
}
FILE *F3;
// la fonction aui va calculer la probabilte de collision par etats sur un fichier.
void Ecrire_en_fichier(){
	float proba_collision[7] = {0,0,0,0,0,0,0};
	int l;
	for(l=0;l<7;l++){
			if(nb_tentatives[l] !=0) proba_collision[l] =(float)nb_collisions[l]/(float)nb_tentatives[l];//pour eviter la division par ZERO !!
	}
	fprintf(F3,"%.2lf %.2lf %.2lf %.2lf %.2lf %.2lf %.2lf %.2lf\n",Time,proba_collision[0]*100,proba_collision[1]*100,proba_collision[2]*100,proba_collision[3]*100,
	proba_collision[4]*100,proba_collision[5]*100,proba_collision[6]*100);
}
FILE *F4;
void Proba_collision_e2(){
	float proba_collision_e2 = 0.0f;
	float time_between ;
	int l = 1 ;
	if(nb_tentatives[l] != 0) {
		proba_collision_e2 =(float)nb_collisions[l]/(float)nb_tentatives[l];//pour eviter la division par ZERO !!
		time_between = Time - last_time;
		temps_experience = temps_experience + time_between;
		moyenne = moyenne + proba_collision_e2 * time_between;// pour calculer la moyenne, on multiplie la proba * son temps d'apparition.
		}
}
float moyenne_proba_totale(){
	float proba_collision_totale = 0.0f;
	float time_between ;
	
	if(nb_tentatives_total != 0){
		proba_collision_totale = (float)nb_collisions_total/(float)nb_tentatives_total ;//pour eviter la division par ZERO !!
		time_between = Time - last_time;
		temps_experience = temps_experience + time_between;
		moyenne = moyenne + proba_collision_totale * time_between;
	}
}

// Lance la simulation
void simulation(){
	float Tab[R];
	struct event next_event;
	float moyenne_proba_e2;
	F3 = fopen("Lora.data","w");
	if (!F3) exit(1);
	F4 = fopen("collision moyenne e2 par simulation.data","w");
	if (!F4) exit(1);
	FILE *F9;
	F9 = fopen("temps d emission e1.data","w");
	if (!F9) exit(1);
	FILE *F10;
	F10 = fopen("temps d emission e2.data","w");
	if (!F10) exit(1);
		FILE *F7;
	F7 = fopen("Collisions moyenne en fct du nb capteurs.data","w");
	if (!F7) exit(1);
		FILE *F8;
	F8 = fopen("nombre de capteurs max pour 2etats.data","w");
	if (!F8) exit(1);
	int i = 0;
	K = 5;// nombre de cqpteurs
	while(i < R)// nombre de simulations
	{
		tete = NULL; // la tete de l'echeancier
		queue = NULL; // l'evenement dernier de l'echeancier.
		current = NULL;
		initialisation();
		while(condition_arret(5000) == 0){ // tant que on a pas encore envoye 1000 messages par capteur.
			last_time = Time; // on actualise le temps
			next_event = extraire_event(); // on extrait le prochain evenement de l'echeancier
			Traitement_Event(next_event); // on traite l'evenement
			Proba_collision_e2(); // va calculer la moyenne de collision a l'etat e2 sur les 50 simulations
			if(i == 0) Ecrire_en_fichier(F3);// genere le fichier Lora.data pour une seul simulation
			if(i == 0){ 
			/* ------------------------------------- Genere les fichiers contenant le temps d'emission de e1 et e2 ------------*/
				if(emission_e1 == 1){// au moment de l'emission d'un packet depuis e1 ou a une collision a e1
				emission_e1 = 0;
				fprintf(F9,"%.2lf %.3lf\n",Time,temps_emission_e1_last);
				temps_emission_e1_total = temps_emission_e1_total + temps_emission_e1_last;
				temps_emission_e1_last = 0;
				temps_emission_e1= 0;
				temps_debut_emission = 0;
				 
			}
			if(emission_e2 == 1){// au moment de l'emission d'un packet depuis e1 ou a une collision a e1
				emission_e2 = 0;
				fprintf(F10,"%.2lf %.3lf\n",Time,temps_emission_e2_last);
				temps_emission_e2_total = temps_emission_e2_total + temps_emission_e2_last;
				temps_emission_e2_last = 0;
				temps_emission_e2= 0;
				temps_debut_emission_e2 = 0;
				 
			}
			}
			
		}
	    
		
		moyenne = moyenne / temps_experience; // la moyenne de collision
		Tab[i] = moyenne*100;
		i = i+1;
		fprintf(F4,"%d %.2lf\n",i,Tab[i-1]); // on ajoute la moyenne de collision e2 de la simulation i
	
		
	}
	
	float moyenne_dist = moyenne_distribution(Tab);
	float ecartType = ecart_type(Tab,moyenne_dist);
	printf("La moyenne de la distribution des 50 sumulations est : %.2lf | et l'ecart type est : %.2lf\n",moyenne_dist, ecartType);
	i = 0;
	K = 1;
	while(i < R)// nombre de simulations
	{
		tete = NULL; // la tete de l'echeancier
		queue = NULL; // l'evenement dernier de l'echeancier.
		current = NULL;
		initialisation();
		while(condition_arret(1000) == 0 && nb_collisions[1] == 0){ 
			last_time = Time; // on actualise le temps
			next_event = extraire_event(); // on extrait le prochain evenement de l'echeancier
			Traitement_Event(next_event); // on traite l'evenement
			}
	    
		if(nb_collisions[1] == 0) {
			K = K + 1;// on incremente le nombre de capteurs et refais l'experience
		}else{
			i = i+1;
			fprintf(F8,"%d %.d\n",i,K);
			K = 1;
		}
	}
	
	K = 1;
	while(K<= 150){
		tete = NULL; // la tete de l'echeancier
		queue = NULL; // l'evenement dernier de l'echeancier.
		current = NULL;
		initialisation(); // in refait l'initialisation
		while(condition_arret(1000) == 0){ // tant que on a pas encore envoye 1000 messages par capteur.
			last_time = Time; // on actualise le temps
			next_event = extraire_event(); // on extrait le prochain evenement de l'echeancier
			Traitement_Event(next_event); // on traite l'evenement
			moyenne_proba_totale();
		}
		moyenne = moyenne / temps_experience; 
		fprintf(F7,"%d %.2lf\n",K,moyenne*100);
		K = K +1;
	}

	fclose(F3);
	fclose(F4);
	fclose(F7); 
	fclose(F8);
	fclose(F9);
	fclose(F10);
}

/* run this program using the console pauser or add your own getch, system("pause") or input loop */
int main(int argc, char *argv[]) {
	srand( time( NULL ) );
	simulation();
	return 0;
}
