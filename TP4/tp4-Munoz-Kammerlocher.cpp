/*
    Exemples de transformations en OpenCV, avec zoom, seuil et affichage en
    couleurs. L'image de niveau est en CV_32SC1.

    $ make ex01-transfos
    $ ./ex01-transfos [-mag width height] [-thr seuil] image_in [image_out]

    CC BY-SA Edouard.Thiel@univ-amu.fr - 07/09/2020

                        --------------------------------

    Renommez ce fichier tp<n°>-<vos-noms>.cpp 
    Écrivez ci-dessous vos NOMS Prénoms et la date de la version :

    KAMMERLOCHER Leo et MUNOZ Elias - version du 2/11/2020
*/

#include <iostream>
#include <iomanip>
#include <cstring>
#include <opencv2/opencv.hpp>
#include "gd-util.hpp"

#define HORAIRE 0
#define ANTI_HORAIRE 1
using namespace std;

float seuil_dist = 0;

typedef struct{
    int p_y, p_x;
    int taille;
    int* chaine_free;
    int dir_fond;
} ContourF8;

typedef struct{
    ContourF8 * f8;
    int taille;
} ContoursF8;

typedef struct{
    int * contoury, contourx;
    int * flag;
}ContourPol;

typedef struct{
    int id;
    int y, x;
    int flag;
} PointContour;

typedef struct{
    PointContour ** pc;
    int taille;
} PointContours;

typedef struct{
    PointContour p;
    float dist;
} PointFrontiere; 

//----------------------------------- M Y -------------------------------------

class My {
  public:
    cv::Mat img_src, img_res1, img_res2, img_niv, img_coul;
    Loupe loupe;
    int polyg = 200;
    int seuil = 127;
    int clic_x = 0;
    int clic_y = 0;
    int clic_n = 0;

    enum Recalc { R_RIEN, R_LOUPE, R_TRANSFOS, R_SEUIL, R_POLYG};
    Recalc recalc = R_SEUIL;
    Recalc recalc_polyg = R_POLYG;

    void reset_recalc ()             { recalc = R_RIEN; }
    void set_recalc   (Recalc level) { if (level > recalc) recalc = level; }
    int  need_recalc  (Recalc level) { return level <= recalc; }

    // Rajoutez ici des codes A_TRANSx pour le calcul et l'affichage
    enum Affi { A_ORIG, A_SEUIL, A_TRANS1, A_TRANS2, A_TRANS3, A_TRANS4, A_TRANS5 };
    Affi affi = A_ORIG;
};


//----------------------- T R A N S F O R M A T I O N S -----------------------

// Placez ici vos fonctions de transformations à la place de ces exemples

// Fonctions pouvant être utilisé pour le marquage des contour c8 et c4

bool x_hors_image(int x, int n_cols){
    if (x == -1 || x == n_cols)
        return true;
    return false;
}

bool y_hors_image(int y, int n_rows){
    if (y == -1 || y == n_rows)
        return true;
    return false;
}

bool point_hors_image(int y, int x, int n_rows, int n_cols)//Dit qu'un point est hors de l'image
{
    return (x_hors_image(x, n_cols) || y_hors_image(y, n_rows));
}

int *voisins_direct(int p_y, int p_x, cv::Mat img_niv) {
    int haut = p_y - 1;
    int bas = p_y + 1;
    int gauche = p_x - 1;
    int droite = p_x + 1;

    int * voisins = (int*) malloc(4*sizeof(int));
    //On initialises les voisins à zero
    for(int i=0; i<4; i++)
        voisins[i] = 0;
    // Si le voisin est contenus dans l'image,
    // On récupère ça valeur.
    if(!y_hors_image(haut, img_niv.rows))
        voisins[0] = img_niv.at<int>(haut,p_x);
    
    if(!y_hors_image(bas, img_niv.rows))
        voisins[1] = img_niv.at<int>(bas,p_x);

    if(!x_hors_image(gauche, img_niv.cols))
        voisins[2] = img_niv.at<int>(p_y,gauche);
    
    if(!x_hors_image(droite, img_niv.cols))
        voisins[3] = img_niv.at<int>(p_y,droite);
    return voisins;
}

int *voisins_indirect(int p_y, int p_x, cv::Mat img_niv) {
    int haut = p_y - 1;
    int bas = p_y + 1;
    int gauche = p_x - 1;
    int droite = p_x + 1;

    int * voisins = (int*) malloc(4*sizeof(int));

    for(int i=0; i<4; i++)
        voisins[i] = 0;

    if(!y_hors_image(haut, img_niv.rows)) {
        if(!x_hors_image(gauche, img_niv.cols))
            voisins[0] = img_niv.at<int>(haut,gauche);
        if(!x_hors_image(droite, img_niv.cols))
            voisins[1] = img_niv.at<int>(haut,droite);
    }
    if(!y_hors_image(bas, img_niv.rows)) {
        if(!x_hors_image(gauche, img_niv.cols))
            voisins[2] = img_niv.at<int>(bas,gauche);
        if(!x_hors_image(droite, img_niv.cols))
            voisins[3] = img_niv.at<int>(bas,droite);
    }

    return voisins;
}


bool est_point_contour(int p_y, int p_x, cv::Mat img_niv, int connexite_fond){
    int * voisins_direct_p = voisins_direct(p_y, p_x, img_niv);
    bool p_est_contour = false;

    if(connexite_fond == 4) {
        // On ne regarde que les voisins directs
        for(int i=0; i<4; i++){
            if(voisins_direct_p[i] == 0) {
                p_est_contour = true;
                break;
            }
        }
    }
    else if(connexite_fond == 8){
        int * voisins_indirect_p = voisins_indirect(p_y, p_x, img_niv);
        // On regarde les voisins directs et indirects.
        for(int i=0; i<4; i++){
            if(voisins_direct_p[i] == 0 || voisins_indirect_p[i] == 0) {
                p_est_contour = true;
                break;
            }
        }
        free(voisins_indirect_p);
    }
    free(voisins_direct_p);
    
    return p_est_contour;
}

bool voisin_est_labele(int v){
    if(v>0 && v!=255){
        return true;
    }
    return false;
}

// Retourne le nombre de voisins qui ont un 
int nb_label_voisin(int p_y, int p_x, cv::Mat img_niv, int connexite_objet) {
    int * voisins_direct_p = voisins_direct(p_y, p_x, img_niv);
    int nb_label = 0;
    if(connexite_objet == 4){
        for(int i=0; i<4; i++){
            if(voisin_est_labele(voisins_direct_p[i]))
                nb_label++;
        }
    }
    else if(connexite_objet == 8){
        int * voisins_indirect_p = voisins_indirect(p_y, p_x, img_niv);
        for(int i=0; i<4; i++){
            if(voisin_est_labele(voisins_direct_p[i]))
                nb_label++;
            if(voisin_est_labele(voisins_indirect_p[i]))
                nb_label++;
        }
        free(voisins_indirect_p);
    }
    free(voisins_direct_p);
    return nb_label;
}
// Retourne les voisins susceptible d'avoir des label
int *get_possible_label_voisins(int p_y, int p_x, cv::Mat img_niv, int connexite_objet){ 
    int * voisins_direct_p = voisins_direct(p_y, p_x, img_niv);
    int * label_voisins;
    if(connexite_objet == 4) {
        label_voisins = (int *) malloc(2*sizeof(int));
        label_voisins[0] = voisins_direct_p[0]; // Voisin du haut
        label_voisins[1] = voisins_direct_p[2]; // Voisin de gauche
    }
    if(connexite_objet == 8) {
        int * voisins_indirect_p = voisins_indirect(p_y, p_x, img_niv);
        label_voisins = (int *) malloc(4*sizeof(int));
        label_voisins[0] = voisins_direct_p[2]; //Voisin gauche
        label_voisins[1] = voisins_indirect_p[0]; //Voisin haut-Gauche 
        label_voisins[2] = voisins_direct_p[0]; // Voisin du haut
        label_voisins[3] = voisins_indirect_p[1]; //Voisin haut-Droit
        free(voisins_indirect_p);
    }
    free(voisins_direct_p);
    return label_voisins;
}
// renvoi le label du premier voisins trouver.
int get_premier_voisin_labele(int * voisin_labele, int connexite_objet){
    int n = 2;
    if(connexite_objet == 8)
        n = 4;
    for(int i=0; i<n; i++){
        if(voisin_est_labele(voisin_labele[i]))
            return voisin_labele[i];
    }
    return 0;
}
// Permute les labels dans la table d'équivalence.
void permute_labels(int t_equiv[], int label1, int label2){
    int tmp, depart, courant;
    bool est_dans_cycle = false;
    if(t_equiv[label1] == label1 || t_equiv[label2] == label2){
        tmp = t_equiv[label1];
        t_equiv[label1] = t_equiv[label2];
        t_equiv[label2] = tmp;
    }
    else{
        depart = label1;
        courant = label1;
        while(t_equiv[courant] != depart){
            if(t_equiv[courant] == label2){
                est_dans_cycle = true;
                break;
            }
            courant = t_equiv[courant];
        }
        if(!est_dans_cycle){
            tmp = t_equiv[label1];
            t_equiv[label1] = t_equiv[label2];
            t_equiv[label2] = tmp;
        }
    }
}
// On initialise la table d'equivalence
void init_tab_equiv(int t_equiv[], int n){
    for(int i=0; i<n; i++){
        t_equiv[i] = i;
    }
}
// On balaye la table d'equivalence pour affecter un label à chaque cycle
void uniformiser_cycle_equiv(int t_equiv[], int n){
    int label = 1;
    int tmp, courant, depart = 0;
    int labele[n];
    for(int i=0; i<n; i++)
        labele[i] = false;
    for(int i=1; i<n; i++){
        courant = i;
        if(labele[courant] == false){
            if(t_equiv[courant] == courant){
                t_equiv[courant] = label++;
                labele[courant] = true;
            }
            else{
                depart = courant;
                while(t_equiv[courant] != depart){
                    tmp = t_equiv[courant];
                    t_equiv[courant] = label;
                    labele[courant] = true;
                    courant = tmp;
                }
                    labele[courant] = true;
                    t_equiv[courant] = label;
                    label++;
            }
        }
    }
}

// Fonctions du TP

void marquer_contour_c8 (cv::Mat img_niv)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x);
        if (g > 0) {
            if(est_point_contour(y, x, img_niv, 4))
                img_niv.at<int>(y,x) = 1;
        }
    }
}

void marquer_contour_c4 (cv::Mat img_niv)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x);
        if (g > 0) {
            if(est_point_contour(y, x, img_niv, 8))
                img_niv.at<int>(y,x) = 1;
        }
    }
}


void transformer_bandes_verticales (cv::Mat img_niv)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)
    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x);
        if (g > 0) {
            img_niv.at<int>(y,x) = x;
        }
    }
}

void numeroter_contour_c8 (cv::Mat img_niv)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    int n = img_niv.rows * img_niv.cols;
    int t_equiv[n];
    init_tab_equiv(t_equiv, n);
    int label = 1;
    int nb_label_v;
    int c_objet = 8;
    int c_fond = 4;
    
    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x);
        if (g > 0 && est_point_contour(y,x, img_niv, c_fond)) {
            nb_label_v = nb_label_voisin(y, x, img_niv, c_objet);
            if(nb_label_v == 0)
                img_niv.at<int>(y,x) = label++;
            else{
                int * voisins_labele= get_possible_label_voisins(y, x, img_niv, c_objet);
                int p = get_premier_voisin_labele(voisins_labele, c_objet);
                img_niv.at<int>(y,x) = p;
                int courant;
                for(int i=0; i<4; i++){
                    courant = voisins_labele[i];
                    if(voisin_est_labele(courant) && courant != p){
                        permute_labels(t_equiv, p, courant);
                    }
                }
                
                free(voisins_labele);
            }
        }
    }
    uniformiser_cycle_equiv(t_equiv, n);
    //DEuxieme balayage
    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x);
        if (g > 0 && est_point_contour(y,x, img_niv, c_fond)) {
            img_niv.at<int>(y,x) = t_equiv[g];
        }
    }
}


void transformer_bandes_diagonales (cv::Mat img_niv)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x);
        if (g > 0) {
            img_niv.at<int>(y,x) = x+y;
        }
    }
}

void N8(int Q[], int ny8[], int nx8[], int y, int x, int d){
    Q[0] = y + ny8[d];
    Q[1] = x + nx8[d]; 
}


//---------------------------------------TP3-------------------------------------------
int get_taille_contourF8(ContourF8 *contour)
{
    int cpt =0;
    while(contour[cpt].chaine_free != NULL)
    {
        cpt++;
    }
    return cpt++;
}

void init_points_contours(PointContour pc[], int n){
    // Etant données une taille > 0 et un tableau de points du contours
    // Et que la taille correspond a celle du tableau 
    if( n > 0 )
        for(int i=0; i<n ;i++){
            // Lorsque je passe par un point
            // Alors je mets sa coordonné y à -1
            pc[i].y = -1;
            // Et je mets sa coordonnée x à -1
            pc[i].x = -1;
            // Et je mets son indice à -1;
            pc[i].id = -1;
            // Et je mets son flag à 1.
            pc[i].flag = 1;
        }
}

void memorise_coord_contours(PointContour pc[], int *chaine_freeman, int n){
    // Etant données une taille > 0, un PointContour et chaine de freeman non vide
    // Et que la taille correspond a celle de la chaine de freeman
    int voisin[2];
    int nx8[8] = {1, 1, 0, -1, -1, -1, 0, 1};
    int ny8[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    if(n > 0 && chaine_freeman != NULL){
        for(int i=0; i<n; i++){
            // Lorsque je passe par une direction d de la chaine de freeman d'indice i
            // Alors je récupère les coords du voisin du point d'indice i
            N8(voisin, ny8, nx8, pc[i].y, pc[i].x, chaine_freeman[i]);
            // Et je change la valeur de y du point d'indice i+1 a celle du voisin
            pc[i+1].y = voisin[0];
            // Et je change la valeur de x du point d'indice i+1 a celle du voisin
            pc[i+1].x = voisin[1];
            // Et je change son id avec i+1;
            pc[i+1].id = i+1;
        }
    } 

}

float calc_distance(PointContour p, PointContour q){
    float a = pow(q.y - p.y, 2);
    float b = pow(q.x - p.x, 2);

    return sqrt(a + b);
}

int get_size_pc(PointContour pc[]){
    int i = 0;
    int size = 0;
    while(pc[i].id == i){
        size++;
        i++;
    }

    return size;
}

PointContour point_max_distance_de(PointContour pc[], int n){
    PointContour *plus_loin;
    PointContour depart = pc[0];
    PointContour *courant;
    float dist_max = 0, dist_courante;
    // Etant donné une liste de points du contour non vide et un taille >0
    // Et que la taille correspond à celle de la liste
    if(n>0 && pc != NULL){
        for(int i = 0; i < n; i++){
            courant = &pc[i];
            // Etant donnés un point de depart et un point courant
            dist_courante = calc_distance(depart, *courant);
            // Lorsque la distance entre depart et courant et superieur à dist_max
            if(dist_courante > dist_max){
                // Alors je remplace dist_max par dist_courante
                dist_max = dist_courante;
                // Et je memorise le point courant
                plus_loin = courant;
            }
        }
        // Lorsque le dernier point du tableau à été visité
        // Alors je retourne le point le plus
        return *plus_loin;
    }
    else{
        printf("Error :\n\tFonction : point_max_distance_de(...)\n\tLigne 470\n");
        exit(1);
    }

}

float distance_point_segment(PointContour A, PointContour B, PointContour C){
    int BCx = C.x - B.x;
    int BCy = C.y - B.y;
    int BAx = A.x - B.x;
    int BAy = A.y - B.y; 

    // les points ayant comme coordonnées (y, x) dans ce tp
    int det = abs(BCx*BAy - BCy*BAx);
    float normBC = sqrt(pow(BCx, 2) + pow(BCy, 2));

    return det/normBC; 
}

PointFrontiere point_max_distance_de_segment(PointContour pc[], int n, int id){
    PointContour B;
    PointContour C;
    PointContour * courant;
    PointContour * plus_loin;
    float dist_max = 0;
    float dist_courante;

    // On recupere les points B et C du segment.
    B.y = pc[id].y;
    B.x = pc[id].x;
    C.y = pc[n].y;
    C.x = pc[n].x;
    
    // Etant donnée une liste de points, un segment, et une taille > id
    if(n > id){
        // On memorise la distance max par la distance du premier point trouvé avec le segment.
        courant = &pc[id+1];
        dist_courante = distance_point_segment(*courant, B, C);
        // Losque la distance entre le point courant et le segment est supérieur a dist_max
        // Alors on remplace dist_max par dist_courante
        dist_max = dist_courante;
        // Et on memorise le point
        plus_loin = courant;
        for(int i=id+2; i< n; i++){
            courant = &pc[i];
            dist_courante = distance_point_segment(*courant, B, C);
            
            // Losque la distance entre le point courant et le segment est supérieur a dist_max
            if(dist_courante > dist_max){
                // Alors on remplace dist_max par dist_courante
                dist_max = dist_courante;
                // Et on memorise le point
                plus_loin = courant;
            }
        }
        // Lorsque le dernier point à été visité
        // Alors on renvoir le point
        PointFrontiere pf;
        pf.p = *plus_loin;
        pf.dist = dist_max;
        return pf;
    }
    else {
        printf("Error :\n\tFonction : point_max_distance_de(...)\n\tLigne ..\n");
        exit(1);
    }

}

void afficher_pc_stats(PointContour pc[], int n){
    int nb_flag_0 = 0;
    int nb_flag_1 = 0;

    for(int i= 0; i<n; i++){
        printf(" %d", pc[i].id);
        if(pc[i].flag == 1)
            nb_flag_1++;
        if(pc[i].flag == 0)
            nb_flag_0++;
    }

    printf("%d points avec flag = 0\n", nb_flag_0);
    printf("%d points avec flag = 1\n", nb_flag_1);
}

void approximer_fragment_c8(PointContour pc[], int id, int n, float seuil){

    //Etant donné une liste de points du contour, un indice de départ et une taille >2
    float max_dist = 0;
    if( n - id > 2 ){
       //On cherche dans l'intervalle ]P, Q[ le point p le plus eloigné de PQ
       PointFrontiere pf = point_max_distance_de_segment(pc, n, id);
       PointContour p= pf.p;
       max_dist = pf.dist;
       // Si la distance entre le point le plus distant du segment et inferieur au seuil
       if(max_dist < seuil){
           // Alors on met a 0 le flag des points du contour en excluant le premier et le dernier
           for(int i = id+1; i < n-1; i++){
               pc[i].flag = 0;
           }
       }
       else{
            approximer_fragment_c8(pc, id, p.id, seuil);
            approximer_fragment_c8(pc, p.id, n, seuil);
        }
    }
}

PointContour * approximer_contour_c8(int * chaine_freeman, int n, int y_depart, int x_depart, float seuil)
{
    if(n > 0){
        PointContour * points_contour =(PointContour *) malloc((n+1)*sizeof(PointContour));
        PointContour point_max;
        /****** Initialisation ******/
        init_points_contours(points_contour, n+1);

        // Je rajoute le point de départ dans ma liste
        points_contour[0].y = y_depart;
        points_contour[0].x = x_depart; 
        points_contour[0].id = 0;
        // On récupère les points du contours
        memorise_coord_contours(points_contour, chaine_freeman, n);
        //Etape 1
        // On recupère le point le plus éloigné du point de départ
        point_max = point_max_distance_de(points_contour, n);

        // On recupere la taille des deux trançons
        int n_fragment1 = point_max.id;
        int n_fragment2 = n+1;
        // Approximation du fragment 1
        approximer_fragment_c8(points_contour, 0 ,n_fragment1, seuil);
        approximer_fragment_c8(points_contour, n_fragment1, n_fragment2, seuil);

        return points_contour;
    }
    else
        return NULL;
}//Fin approximer_contour_c8

void colorier_morceau(PointContour pc[], int n, cv::Mat img_niv){
    int num_label = 1;
    int nb_flag_1 = 0;

    for(int i = 0; i < n; i++){
        if(nb_flag_1 == 2){
            nb_flag_1 = 0;
            num_label++;
            if(num_label == 255)
                num_label++;
        }
        if(pc[i].flag == 1){
            nb_flag_1++;
        }
        img_niv.at<int>(pc[i].y,pc[i].x) = num_label;

    }
}

PointContours approximer_et_colorier_contours_c8(ContoursF8 tab_contours_F8, float seuil, cv::Mat img_niv){
    int taille_F8 = tab_contours_F8.taille;
    ContourF8 * contours_F8 = tab_contours_F8.f8;
    PointContours pc_tab;
    pc_tab.taille = taille_F8;
    pc_tab.pc = (PointContour **) malloc(taille_F8*sizeof(PointContour*));
    for(int i=0; i<taille_F8; i++){
        //PointContour pc[contours_F8[i].taille +1 ];
        pc_tab.pc[i] = approximer_contour_c8(contours_F8[i].chaine_free, contours_F8[i].taille, contours_F8[i].p_y, contours_F8[i].p_x, seuil);
        if(pc_tab.pc[i] != NULL){
            //afficher_pc_stats(pc_tab.pc[i], contours_F8[i].taille +1);
            colorier_morceau(pc_tab.pc[i], contours_F8[i].taille +1, img_niv);
        }
    }
    return pc_tab;
}

//----------------------------------------------TP2-----------------------------------------------------

int get_direction(int k, int i, int sens){
    if(sens == HORAIRE)
        return (k+i)%8;
    return (k+8-i)%8;
}

int get_dir_oppose(int d){
    return (d+4)%8;
}

void suivre_un_contour_c8(cv::Mat img_niv, int yA, int xA, int ny8[], int nx8[], int dirA, int num_contour, ContourF8 *contour_F8)
{
    int d=0;
    int Q[2];
    int dir_finale=0;
    for(int i=0; i<8; i++)
    {  
        d = get_direction(dirA, i, HORAIRE);
        N8(Q, ny8, nx8, yA, xA, d);//Q[0] = y && Q[1] = x
        if(!point_hors_image(Q[0], Q[1], img_niv.rows, img_niv.cols) && img_niv.at<int>(Q[0],Q[1]) > 0)
        {
            //On obtient la direction finale du voisin precedent que cloturera la boucle
            dir_finale = get_dir_oppose(d);
            break;
        }
    }
    int x = xA;
    int y = yA;
    int dir = dir_finale;
    bool est_isole = true;
    do
    {
        img_niv.at<int>(y, x) = num_contour;
        dir = (dir + 3)%8;
        for(int i = 0; i<8; i++)
        {
            d = get_direction(dir, i, ANTI_HORAIRE);
            N8(Q, ny8, nx8, y, x, d);
            if(!point_hors_image(Q[0], Q[1], img_niv.rows, img_niv.cols) && img_niv.at<int>(Q[0],Q[1]) > 0)
            {
                y=Q[0];
                x=Q[1];
                dir=d;
                est_isole = false;
                contour_F8->chaine_free[contour_F8->taille++]=dir;
                break;
            }
            
        }
        if(est_isole)//point isolé
        {
            break;
        }

    }
    while(!(x == xA && y == yA && dir == dir_finale));
    //contour_F8->chaine_free[contour_F8->taille++]=dir;
    
}

ContoursF8 effectuer_suivi_contours_c8(cv::Mat img_niv)//Permet d'editer le contour de l'image
{
    int num_contour =1; //initialisation de la couleur du contour
    int nx8[8] = {1, 1, 0, -1, -1, -1, 0, 1};
    int ny8[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    ContourF8 * contours_F8 = (ContourF8 *) malloc(img_niv.rows*img_niv.cols*sizeof(ContourF8));
    int taille_F8 = 0;
    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        if (img_niv.at<int>(y,x) == 255) //Verifie si on est dans l'objet
        {
            int dir = -1;
            if(x == 0 || img_niv.at<int>(y,x-1) == 0)//bord gauche
            {
                dir = 4;
                            }
            else if(y == 0 || img_niv.at<int>(y-1,x) == 0)//bord haut
            {
                dir = 6;
            }
            else if(x == img_niv.cols -1 || img_niv.at<int>(y,x+1) == 0)//bord droit
            {
                dir = 0;
            }
            else if (y == img_niv.rows -1 || img_niv.at<int>(y+1,x) == 0)//bord bas
            {
                dir = 2;
            }
            
            
            if(dir >= 0) //trouve un contour
            {
                contours_F8[taille_F8].dir_fond=dir;
                contours_F8[taille_F8].p_y=y;
                contours_F8[taille_F8].p_x = x;
                contours_F8[taille_F8].taille = 0;
                contours_F8[taille_F8].chaine_free = (int *) malloc(img_niv.rows*img_niv.cols*sizeof(int));
                suivre_un_contour_c8(img_niv, y, x, ny8, nx8, dir, num_contour++,&contours_F8[taille_F8]);
                taille_F8++;
                //printf("contour");
                //break;
                if(num_contour == 255)
                {
                    num_contour++;
                }
            }
        }
    }
    ContoursF8 csF8;
    csF8.f8 = contours_F8;
    csF8.taille = taille_F8;
    return csF8;

}// Fin effectuer_suivi_contours_c8

//----------------------------------------------TP2---------------------------------------------------//

void remplir_polyg(PointContour pc[], cv::Mat img_niv, cv::Scalar col, int n){
    int n_flags = 0; 
    for (int i=0; i<n; i++){
       if(pc[i].flag == 1){
            n_flags++;
       }
    }
    cv::Point points[0][n];
    int cpt = 0;
    for (int i=0; i<n; i++){
        points[0][cpt++] = cv::Point(pc[i].y, pc[i].x);
    }
    printf("OK\n");
    const cv::Point* ppt[1] = {points[0]};
    int npt[] = {n};
    cv::fillPoly(img_niv, ppt, npt, 1, col, 8);
}

// Appelez ici vos transformations selon affi
void effectuer_transformations (My::Affi affi, cv::Mat img_niv)
{
    ContoursF8 contours_f8;
    PointContour * pc;
    switch (affi) {
        case My::A_TRANS1 :
            //marquer_contour_c8 (img_niv);
            contours_f8 = effectuer_suivi_contours_c8(img_niv);
            pc = approximer_contour_c8(contours_f8.f8[0].chaine_free, contours_f8.f8[0].taille, contours_f8.f8[0].p_y, contours_f8.f8[0].p_x, seuil_dist);
            remplir_polyg(pc, img_niv, cv::Scalar(0, 0, 0), contours_f8.f8[0].taille+1);
            break;
        case My::A_TRANS2 :
            marquer_contour_c4 (img_niv);
            break;
        case My::A_TRANS3 :
            numeroter_contour_c8 (img_niv);
            break;
        case My::A_TRANS4 :
            effectuer_suivi_contours_c8(img_niv);
            break;
        case My::A_TRANS5 : 
            contours_f8 = effectuer_suivi_contours_c8(img_niv);
            approximer_et_colorier_contours_c8(contours_f8, seuil_dist, img_niv);
            break;
        default : ;
    }
}

//---------------------------- C A L L B A C K S ------------------------------

// Callback des sliders
void onZoomSlide (int pos, void *data)
{
    My *my = (My*) data;
    my->loupe.reborner (my->img_res1, my->img_res2);
    my->set_recalc(My::R_LOUPE);
}

void onSeuilSlide (int pos, void *data)
{
    My *my = (My*) data;
    my->set_recalc(My::R_SEUIL);
}

void onPolygSlide (int pos, void *data)
{
    My *my = (My*) data;
    my->set_recalc(My::R_POLYG);
}


// Callback pour la souris
void onMouseEventSrc (int event, int x, int y, int flags, void *data)
{
    My *my = (My*) data;

    switch (event) {
        case cv::EVENT_LBUTTONDOWN :
            my->clic_x = x;
            my->clic_y = y;
            my->clic_n = 1;
            break;
        case cv::EVENT_MOUSEMOVE :
            // std::cout << "mouse move " << x << "," << y << std::endl;
            if (my->clic_n == 1) {
                my->loupe.deplacer (my->img_res1, my->img_res2, 
                    x - my->clic_x, y - my->clic_y);
                my->clic_x = x;
                my->clic_y = y;
                my->set_recalc(My::R_LOUPE);
            }
            break;
        case cv::EVENT_LBUTTONUP :
            my->clic_n = 0;
            break;
    }
}


void onMouseEventLoupe (int event, int x, int y, int flags, void *data)
{
    My *my = (My*) data;

    switch (event) {
        case cv::EVENT_LBUTTONDOWN :
            my->loupe.afficher_tableau_valeurs (my->img_niv, x, y, 5, 4);
            break;
    }
}


void afficher_aide() {
    // Indiquez les transformations ici
    std::cout <<
        "Touches du clavier:\n"
        "   a    affiche cette aide\n"
        " hHlL   change la taille de la loupe\n"
        "   i    inverse les couleurs de src\n"
        "   o    affiche l'image src originale\n"
        "   s    affiche l'image src seuillée\n"
        "   1    marquer contour C8\n"
        "   2    marquer contour C4\n"
        "   3    numeroter contour c8\n"
        "   4    suivi de contour c8\n"
        "   5    Approximer et colorier contour\n"
        "  esc   quitte\n"
    << std::endl;
}

// Callback "maison" pour le clavier
int onKeyPressEvent (int key, void *data)
{
    My *my = (My*) data;

    if (key < 0) return 0;        // aucune touche pressée
    key &= 255;                   // pour comparer avec un char
    if (key == 27) return -1;     // ESC pour quitter

    switch (key) {
        case 'a' :
            afficher_aide();
            break;
        case 'h' :
        case 'H' :
        case 'l' :
        case 'L' : {
            std::cout << "Taille loupe" << std::endl;
            int h = my->img_res2.rows, w = my->img_res2.cols; 
            if      (key == 'h') h = h >=  200+100 ? h-100 :  200;
            else if (key == 'H') h = h <= 2000-100 ? h+100 : 2000;
            else if (key == 'l') w = w >=  200+100 ? w-100 :  200;
            else if (key == 'L') w = w <= 2000-100 ? w+100 : 2000;
            my->img_res2 = cv::Mat(h, w, CV_8UC3);
            my->loupe.reborner(my->img_res1, my->img_res2);
            my->set_recalc(My::R_LOUPE);
          } break;
        case 'i' :
            std::cout << "Couleurs inversées" << std::endl;
            inverser_couleurs(my->img_src);
            my->set_recalc(My::R_SEUIL);
            break;
        case 'o' :
            std::cout << "Image originale" << std::endl;
            my->affi = My::A_ORIG;
            my->set_recalc(My::R_TRANSFOS);
            break;
        case 's' :
            std::cout << "Image seuillée" << std::endl;
            my->affi = My::A_SEUIL;
            my->set_recalc(My::R_SEUIL);
            break;

        // Rajoutez ici des touches pour les transformations.
        // Dans my->set_recalc, passez :
        //   My::R_SEUIL pour faire le calcul à partir de l'image originale seuillée
        //   My::R_TRANSFOS pour faire le calcul à partir de l'image actuelle
        case '1' :
            std::cout << "marquer contour C8" << std::endl;
            my->affi = My::A_TRANS1;
            my->set_recalc(My::R_SEUIL);
            break;
        case '2' :
            std::cout << "marquer contour C4" << std::endl;
            my->affi = My::A_TRANS2;
            my->set_recalc(My::R_SEUIL);
            break;
        case '3' :
            std::cout << "numeroter contour c8" << std::endl;
            my->affi = My::A_TRANS3;
            my->set_recalc(My::R_SEUIL);
            break;

        case '4' :
            std::cout << "suivi de contour c8" << std::endl;
            my->affi = My::A_TRANS4;
            my->set_recalc(My::R_SEUIL);
            break;

        case '5' :
            std::cout << "Approximer et colorier contour" << std::endl;
            my->affi = My::A_TRANS5;
            my->set_recalc(My::R_SEUIL);
            break;

        default :
            //std::cout << "Touche '" << char(key) << "'" << std::endl;
            break;
    }
    return 1;
}


//---------------------------------- M A I N ----------------------------------

void afficher_usage (char *nom_prog) {
    std::cout << "Usage: " << nom_prog
              << "[-mag width height] [-thr seuil] in1 [out2]" 
              << std::endl;
}

int main (int argc, char**argv)
{
    My my;
    char *nom_in1, *nom_out2, *nom_prog = argv[0];
    int zoom_w = 600, zoom_h = 500;
    seuil_dist = (float)my.polyg;
    while (argc-1 > 0) {
        if (!strcmp(argv[1], "-mag")) {
            if (argc-1 < 3) { afficher_usage(nom_prog); return 1; }
            zoom_w = atoi(argv[2]);
            zoom_h = atoi(argv[3]);
            argc -= 3; argv += 3;
        } else if (!strcmp(argv[1], "-thr")) {
            if (argc-1 < 2) { afficher_usage(nom_prog); return 1; }
            my.seuil = atoi(argv[2]);
            argc -= 2; argv += 2;
        } else break;
    }
    if (argc-1 < 1 or argc-1 > 2) { afficher_usage(nom_prog); return 1; }
    nom_in1  = argv[1];
    nom_out2 = (argc-1 == 2) ? argv[2] : NULL;

    // Lecture image
    my.img_src = cv::imread (nom_in1, cv::IMREAD_COLOR);  // produit du 8UC3
    if (my.img_src.empty()) {
        std::cout << "Erreur de lecture" << std::endl;
        return 1;
    }

    // Création résultats
    my.img_res1 = cv::Mat(my.img_src.rows, my.img_src.cols, CV_8UC3);
    my.img_res2 = cv::Mat(zoom_h, zoom_w, CV_8UC3);
    my.img_niv  = cv::Mat(my.img_src.rows, my.img_src.cols, CV_32SC1);
    my.img_coul = cv::Mat(my.img_src.rows, my.img_src.cols, CV_8UC3);
    my.loupe.reborner(my.img_res1, my.img_res2);

    // Création fenêtre
    cv::namedWindow ("ImageSrc", cv::WINDOW_AUTOSIZE);
    cv::createTrackbar ("Zoom", "ImageSrc", &my.loupe.zoom, my.loupe.zoom_max, 
        onZoomSlide, &my);
    cv::createTrackbar ("Seuil", "ImageSrc", &my.seuil, 255, 
        onSeuilSlide, &my);
    cv::createTrackbar ("Polyg", "ImageSrc", &my.polyg, 1000, 
        onPolygSlide, &my);
    cv::setMouseCallback ("ImageSrc", onMouseEventSrc, &my);

    cv::namedWindow ("Loupe", cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback ("Loupe", onMouseEventLoupe, &my);

    afficher_aide();

    // Boucle d'événements
    for (;;) {

        if (my.need_recalc(My::R_SEUIL)) 
        {
            // std::cout << "Calcul seuil" << std::endl;
            cv::Mat img_gry;
            cv::cvtColor (my.img_src, img_gry, cv::COLOR_BGR2GRAY);
            cv::threshold (img_gry, img_gry, my.seuil, 255, cv::THRESH_BINARY);
            img_gry.convertTo (my.img_niv, CV_32SC1,1., 0.);
        }

        if(my.need_recalc(My::R_POLYG)){
            std::cout << my.polyg << std::endl;
            if (my.affi != My::A_ORIG) {
                seuil_dist = float(my.polyg)/100.f;
                effectuer_transformations (my.affi, my.img_niv);
                representer_en_couleurs_vga (my.img_niv, my.img_coul);
            } else my.img_coul = my.img_src.clone();
        }

        if (my.need_recalc(My::R_TRANSFOS))
        {
            // std::cout << "Calcul transfos" << std::endl;
            if (my.affi != My::A_ORIG) {
                effectuer_transformations (my.affi, my.img_niv);
                representer_en_couleurs_vga (my.img_niv, my.img_coul);
            } else my.img_coul = my.img_src.clone();
        }

        if (my.need_recalc(My::R_LOUPE)) {
            // std::cout << "Calcul loupe puis affichage" << std::endl;
            my.loupe.dessiner_rect    (my.img_coul, my.img_res1);
            my.loupe.dessiner_portion (my.img_coul, my.img_res2);
            cv::imshow ("ImageSrc", my.img_res1);
            cv::imshow ("Loupe"   , my.img_res2);
        }
        my.reset_recalc();

        // Attente du prochain événement sur toutes les fenêtres, avec un
        // timeout de 15ms pour détecter les changements de flags
        int key = cv::waitKey (15);

        // Gestion des événements clavier avec une callback "maison" que l'on
        // appelle nous-même. Les Callbacks souris et slider sont directement
        // appelées par waitKey lors de l'attente.
        if (onKeyPressEvent (key, &my) < 0) break;
    }

    // Enregistrement résultat
    if (nom_out2) {
        if (! cv::imwrite (nom_out2, my.img_coul))
             std::cout << "Erreur d'enregistrement" << std::endl;
        else std::cout << "Enregistrement effectué" << std::endl;
     }
    return 0;
}

