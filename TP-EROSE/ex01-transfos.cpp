/*
    Exemples de transformations en OpenCV, avec zoom, seuil et affichage en
    couleurs. L'image de niveau est en CV_32SC1.

    $ make ex01-transfos
    $ ./ex01-transfos [-mag width height] [-thr seuil] image_in [image_out]

    CC BY-SA Edouard.Thiel@univ-amu.fr - 07/09/2020

                        --------------------------------

    Renommez ce fichier tp<n°>-<vos-noms>.cpp 
    Écrivez ci-dessous vos NOMS Prénoms et la date de la version :

    <NOM1 Prénom1> [et <NOM2 Prénom2>] - version du <date>
*/

#include <iostream>
#include <iomanip>
#include <cstring>
#include <opencv2/opencv.hpp>
#include "gd-util.hpp"

#define DILATATION 0
#define EROSION 1
// Hello
// Definition des structures
typedef struct{
    int x;
    int y;
    int color;
} Point;

typedef struct{
    Point origine;
    int * directions;
    int taille;
 } Mask;

typedef struct{
    Point * p_tab;
    int taille;
 } pointsOperation;

const int nx8[8] = {1, 1, 0 ,-1, -1, -1, 0, 1};
const int ny8[8] = {0, 1, 1, 1, 0, -1, -1, -1};
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
    enum Affi { A_ORIG, A_SEUIL, A_TRANS1, A_TRANS2, A_TRANS3, A_TRANS4, A_TRANS5, A_TRANS6, A_TRANS7, A_TRANS8 };
    Affi affi = A_ORIG;
};


//----------------------- T R A N S F O R M A T I O N S -----------------------

Point * get_point_from_dir(int d, int y, int x){
    Point * p = (Point *) malloc(sizeof(Point));
    p->y= y + ny8[d];
    p->x = x + nx8[d];
    return p;
}

// Placez ici vos fonctions de transformations à la place de ces exemples


void colorier_points_operation(cv::Mat img_niv, pointsOperation po){
    int y,x;
    int label;
    for(int i= 0; i<po.taille; i++){
        y = po.p_tab[i].y;
        x = po.p_tab[i].x;
        label = po.p_tab[i].color;
        img_niv.at<int>(y,x) = label;
    }
}

pointsOperation dilatation (cv::Mat img_niv, Mask m)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    pointsOperation po;
    po.taille = 0;
    Point *pts;
    po.p_tab = (Point*)malloc(img_niv.rows * img_niv.cols * sizeof(Point));

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x);
        if(g == 0)
        {  
            m.origine.x = x;
            m.origine.y = y;
            for(int i = 0 ; i < m.taille ; i++)
            {
                int d = m.directions[i];
                int color;
                pts = get_point_from_dir( d, y, x);
                if(pts->x > img_niv.cols - 1 || pts->x < 0 || pts->y > img_niv.rows - 1 || pts->y < 0)
                {
                    color = 0;
                }
                else
                    color =img_niv.at<int>(pts->y, pts->x);
                if(color == 255)
                {

                    m.origine.color = 255;
                    po.p_tab[po.taille++] = m.origine;
                    break;
                }
            }
        }
    }
    return po;
}


pointsOperation erosion (cv::Mat img_niv, Mask m)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    pointsOperation po;
    po.taille = 0;
    Point *pts;

    po.p_tab = (Point*)malloc(img_niv.rows * img_niv.cols * sizeof(Point));
    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x);
        if(g>0)
        {  
            m.origine.x = x;
            m.origine.y = y;
            for(int i = 0 ; i < m.taille ; i++)
            {
                int d = m.directions[i];
                int color;
                pts = get_point_from_dir( d, y, x);
                if(pts->x > img_niv.cols -1 || pts->x < 0 || pts->y > img_niv.rows -1 || pts->y < 0)
                    color = 255;
                else 
                    color = img_niv.at<int>(pts->y, pts->x);
                if(color == 0)
                {
                    m.origine.color = 0;
                    po.p_tab[po.taille++] = m.origine;
                    break;
                }
            }
        }
    }
    return po;
}

void ouverture(cv::Mat img_niv, Mask m){
    pointsOperation po;
    po = erosion(img_niv, m);
    colorier_points_operation(img_niv, po);
    po = dilatation(img_niv, m);
    colorier_points_operation(img_niv, po);
}

void fermeture(cv::Mat img_niv, Mask m){
    pointsOperation po;
    po = dilatation(img_niv, m);
    colorier_points_operation(img_niv, po);
    po = erosion(img_niv, m);
    colorier_points_operation(img_niv, po);
}

float f(int x, int choix)
{
    if(choix == 0)
        return x;//f(x) = x
    if(choix == 1)
        return x*std::sin(x)+50;
    else
        return (x*x-x)/50;
}

void dessine_fonction(cv::Mat img_niv, int choix)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        if(f(x, choix) <= y)
            img_niv.at<int>(y,x) = 0;
        else
            img_niv.at<int>(y,x) = 255;    
    }
}

pointsOperation erosion_fonctionnelle(cv::Mat img_niv, Mask m, int choix)
{
    dessine_fonction(img_niv, choix);
    return erosion(img_niv, m);
}

pointsOperation dilatation_fonctionnelle(cv::Mat img_niv, Mask m, int choix)
{
    dessine_fonction(img_niv, choix);
    return dilatation(img_niv, m);
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



// Appelez ici vos transformations selon affi
void effectuer_transformations (My::Affi affi, cv::Mat img_niv)
{
    int dirs[4] = {0, 2, 4, 6}; // element structurant -> croix
    Mask mask;
    mask.directions =(int *) malloc(4*sizeof(int));
    for(int i=0; i<4; i++){
        mask.directions[i] = dirs[i];
    }
    mask.taille = 4;
    printf("directions: %d, %d, %d, %d\n", mask.directions[0],mask.directions[1], mask.directions[2],mask.directions[3]);
    pointsOperation po;
    switch (affi) {
        case My::A_TRANS1 :
            po = dilatation(img_niv, mask);
            colorier_points_operation(img_niv, po);
            break;
        case My::A_TRANS2 :
            po = erosion(img_niv, mask);
            colorier_points_operation(img_niv, po);
            break;
        case My::A_TRANS3 :
            ouverture(img_niv, mask);
            break;
        case My::A_TRANS4 :
            fermeture(img_niv, mask);
            break;
        case My::A_TRANS5 :
            po = erosion_fonctionnelle(img_niv, mask,22);
            colorier_points_operation(img_niv, po);
            break;
        case My::A_TRANS6 :
            po = dilatation_fonctionnelle(img_niv, mask, 2);
            colorier_points_operation(img_niv, po);
            break;
        case My::A_TRANS7 :
            po = erosion_fonctionnelle(img_niv, mask,1);
            colorier_points_operation(img_niv, po);
            break;
        case My::A_TRANS8 :
            po = dilatation_fonctionnelle(img_niv, mask, 1);
            colorier_points_operation(img_niv, po);
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
        "   1    Dilatation ensembliste\n"
        "   2    Erosion ensembliste\n"
        "   3    ouverture ensembliste\n"
        "   4    fermeture ensembliste\n"
        "   5    Erosion fonctionnelle f(x) = y"
        "   6    Dilatation fonctionnelle f(x) = y"
        "   7    Erosion fonctionnelle (x/4)*sin(x)"
        "   8    Dilatation fonctionnelle (x/4)*sin(x)"
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
            std::cout << "Dilatation ensembliste" << std::endl;
            my->affi = My::A_TRANS1;
            my->set_recalc(My::R_SEUIL);
            break;
        case '2' :
            std::cout << "Erosion ensembliste" << std::endl;
            my->affi = My::A_TRANS2;
            my->set_recalc(My::R_SEUIL);
            break;
        case '3' :
            std::cout << "Ouverture ensembliste" << std::endl;
            my->affi = My::A_TRANS3;
            my->set_recalc(My::R_SEUIL);
            break;
        case '4' :
            std::cout << "Fermeture ensembliste" << std::endl;
            my->affi = My::A_TRANS4;
            my->set_recalc(My::R_SEUIL);
            break;
        case '5' :
            std::cout << "Erosion fonctionnelle f(x) = x" << std::endl;
            my->affi = My::A_TRANS5;
            my->set_recalc(My::R_SEUIL);
            break;
        case '6' :
            std::cout << "dilatation fonctionnelle f(x) = x" << std::endl;
            my->affi = My::A_TRANS6;
            my->set_recalc(My::R_SEUIL);
            break;
        case '7' :
            std::cout << "Erosion fonctionnelle f(x) = (x/4)*sin(x)" << std::endl;
            my->affi = My::A_TRANS7;
            my->set_recalc(My::R_SEUIL);
            break;
        case '8' :
            std::cout << "Dilatation fonctionnelle f(x) = (x/4)*sin(x)" << std::endl;
            my->affi = My::A_TRANS8;
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

