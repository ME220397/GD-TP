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
    enum Affi { A_ORIG, A_SEUIL, A_TRANS1, A_TRANS2, A_TRANS3 };
    Affi affi = A_ORIG;
};


//----------------------- T R A N S F O R M A T I O N S -----------------------

// Placez ici vos fonctions de transformations à la place de ces exemples

void transformer_bandes_horizontales (cv::Mat img_niv)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x);
        if (g > 0) {
            img_niv.at<int>(y,x) = y;
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
    switch (affi) {
        case My::A_TRANS1 :
            transformer_bandes_horizontales (img_niv);
            break;
        case My::A_TRANS2 :
            transformer_bandes_verticales (img_niv);
            break;
        case My::A_TRANS3 :
            transformer_bandes_diagonales (img_niv);
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
        "   1    affiche la transformation 1\n"
        "   2    affiche la transformation 2\n"
        "   3    affiche la transformation 3\n"
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

