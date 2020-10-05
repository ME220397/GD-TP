/*
    Module utilitaire pour les TPs en géométrie discrète avec OpenCV

    CC BY-SA Edouard.Thiel@univ-amu.fr - 07/09/2020
*/


// Cette macro permet de vérifier si une image a le type attendu

#define CHECK_MAT_TYPE(mat, format_type) \
    if (mat.type() != int(format_type)) \
        throw std::runtime_error(std::string(__func__) +\
            ": format non géré '" + std::to_string(mat.type()) +\
            "' pour la matrice '" # mat "'");


// Classe Loupe pour afficher une partie de l'image avec un zoom

class Loupe {
  public:
    int zoom = 5;
    int zoom_max = 20;
    int zoom_x0 = 0;
    int zoom_y0 = 0;
    int zoom_x1 = 100;
    int zoom_y1 = 100;

    void reborner (cv::Mat &res1, cv::Mat &res2);
    void deplacer (cv::Mat &res1, cv::Mat &res2, int dx, int dy);
    void dessiner_rect (cv::Mat &src, cv::Mat &dest);
    void dessiner_portion (cv::Mat &src, cv::Mat &dest);
    void afficher_tableau_valeurs (cv::Mat &src, int ex, int ey, int rx, int ry);
};


// Couleurs

void representer_en_couleurs_vga (cv::Mat img_niv, cv::Mat img_coul);
void inverser_couleurs (cv::Mat img);

