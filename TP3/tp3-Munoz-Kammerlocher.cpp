/*
    Exemples de transformations en OpenCV, avec zoom, seuil et affichage en
    couleurs. L'image de niveau est en CV_32SC1.

    $ make ex01-transfos
    $ ./ex01-transfos [-mag width height] [-thr seuil] image_in [image_out]

    CC BY-SA Edouard.Thiel@univ-amu.fr - 07/09/2020

                        --------------------------------

    Renommez ce fichier tp<n°>-<vos-noms>.cpp 
    Écrivez ci-dessous vos NOMS Prénoms et la date de la version :

    Kammerlocher Leo et Elias Munoz - version du 22/09/2020
*/

#include <iostream>
#include <iomanip>
#include <cstring>
#include <opencv2/opencv.hpp>
#include "gd-util.hpp"

#define HORAIRE 0
#define ANTI_HORAIRE 1

typedef struct{
    int p_y, p_x;
    int taille;
    int* chaine_free;
} ContourF8;

typedef struct{
    int contoury[], contourx[];
    int flag[];
}ContourPol;

//----------------------------------- M Y -------------------------------------

class My {
  public:
    cv::Mat img_src, img_res1, img_res2, img_niv, img_coul;
    Loupe loupe;
    int seuil = 127;
    int clic_x = 0;
    int clic_y = 0;
    int clic_n = 0;

    enum Recalc { R_RIEN, R_LOUPE, R_TRANSFOS, R_SEUIL };
    Recalc recalc = R_SEUIL;

    void reset_recalc ()             { recalc = R_RIEN; }
    void set_recalc   (Recalc level) { if (level > recalc) recalc = level; }
    int  need_recalc  (Recalc level) { return level <= recalc; }

    // Rajoutez ici des codes A_TRANSx pour le calcul et l'affichage
    enum Affi { A_ORIG, A_SEUIL, A_TRANS1, A_TRANS2, A_TRANS3, A_TRANS4 };
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


//----------------------------------------------TP2-----------------------------------------------------

void N8(int Q[], int ny8[], int nx8[], int y, int x, int d){
    Q[0] = y + ny8[d];
    Q[1] = x + nx8[d]; 
}



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
    printf("%d", dir);
    contour_F8->chaine_free[contour_F8->taille++]=dir;
    
}

ContourF8 * effectuer_suivi_contours_c8(cv::Mat img_niv)//Permet d'editer le contour de l'image
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
    for(int i=0; i<taille_F8; i++){
        printf("Contour %d : p_x = %d, p_y = %d\n", i, contours_F8[i].p_x, contours_F8[i].p_y);

        printf("s = {");
        for(int j=0; j<contours_F8[i].taille-1; j++)
         printf("%d, ", contours_F8[i].chaine_free[j]);
        printf("%d", contours_F8[i].chaine_free[contours_F8[i].taille-1]);
        printf("}\n");
    }
    return contours_F8;

}// Fin effectuer_suivi_contours_c8


//---------------------------------------TP3-------------------------------------------
int get_taille_contourF8(ContourF8 *contour)
{
    int cpt =0;
    while(contour[cpt].chaine_free != NULL)
    {
        cpt++;
    }
    return cpt;
}



ContourPol *approximer_contour_c8(ContourF8 *contour, float seuil)
{
    int nx8[8] = {1, 1, 0, -1, -1, -1, 0, 1};
    int ny8[8] = {0, 1, 1, 1, 0, -1, -1, -1};

    int taille_csF8 = get_taille_contourF8(contour);
    ContourPol *contourpol = (ContourPol *) malloc(taille_csF8 * sizeof(ContourPol));
    int taille=0;
    int Q[2];
    int p_x=0, p_y=0;
    for(int i = 0; i < taille_csF8; i++)
    {
        //Initialisation
        taille = contour[i].taille;
        int contourx[taille], contoury[taille], const_f[taille];
        const_f[0] = 1;
        contourx[0] = p_x;//On met le point de départ dans contourx et contoury
        contoury[0] = p_y;
        for (int j = 1; j< taille; j++)
        {
            N8(Q[], ny8[], nx8[], p_x, p_y, contour[i].chaine_free[j-1]);
            p_x = Q[0];
            p_y = Q[1];
            contourx[j] = p_x;
            contoury[j] = p_y;
            const_f[i] = 1;
        }
        //Etape 1
        
    }

}//Fin approximer_contour_c8






// Appelez ici vos transformations selon affi
void effectuer_transformations (My::Affi affi, cv::Mat img_niv)
{
    switch (affi) {
        case My::A_TRANS1 :
            marquer_contour_c8 (img_niv);
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
        default : ;
    }
}




//----------------------------------------------TP3-----------------------------------------------------







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
        "   1    affiche la transformation 1\n"
        "   2    affiche la transformation 2\n"
        "   3    affiche la transformation 3\n"
        "   4    affiche la transformation 4\n"
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
            std::cout << "Transformation 1" << std::endl;
            my->affi = My::A_TRANS1;
            my->set_recalc(My::R_SEUIL);
            break;
        case '2' :
            std::cout << "Transformation 2" << std::endl;
            my->affi = My::A_TRANS2;
            my->set_recalc(My::R_SEUIL);
            break;
        case '3' :
            std::cout << "Transformation 3" << std::endl;
            my->affi = My::A_TRANS3;
            my->set_recalc(My::R_SEUIL);
            break;

        case '4' :
            std::cout << "Transformation 4" << std::endl;
            my->affi = My::A_TRANS4;
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

