/*
    Module utilitaire pour les TPs en géométrie discrète avec OpenCV

    CC BY-SA Edouard.Thiel@univ-amu.fr - 07/09/2020
*/

#include <iostream>
#include <iomanip>
#include <cstring>
#include <opencv2/opencv.hpp>
#include "gd-util.hpp"


//--------------------------------- L O U P E ---------------------------------

void Loupe::reborner (cv::Mat &res1, cv::Mat &res2)
{
    int bon_zoom = zoom >= 1 ? zoom : 1;

    int h = res2.rows / bon_zoom;
    int w = res2.cols / bon_zoom;

    if (zoom_x0 < 0) zoom_x0 = 0;
    zoom_x1 = zoom_x0 + w;
    if (zoom_x1 > res1.cols) {
        zoom_x1 = res1.cols;
        zoom_x0 = zoom_x1 - w;
        if (zoom_x0 < 0) zoom_x0 = 0;
    }

    if (zoom_y0 < 0) zoom_y0 = 0;
    zoom_y1 = zoom_y0 + h;
    if (zoom_y1 > res1.rows) {
        zoom_y1 = res1.rows;
        zoom_y0 = zoom_y1 - h;
        if (zoom_y0 < 0) zoom_y0 = 0;
    }
}


void Loupe::deplacer (cv::Mat &res1, cv::Mat &res2, int dx, int dy)
{
    zoom_x0 += dx; zoom_y0 += dy; 
    zoom_x1 += dx; zoom_y1 += dy; 
    reborner (res1, res2);
}


void Loupe::dessiner_rect (cv::Mat &src, cv::Mat &dest)
{
    dest = src.clone();
    if (zoom == 0) return;
    cv::Point p0 = cv::Point(zoom_x0, zoom_y0),
              p1 = cv::Point(zoom_x1, zoom_y1);
    cv::rectangle(dest, p0, p1, cv::Scalar (255, 255, 255), 3, 4);
    cv::rectangle(dest, p0, p1, cv::Scalar (  0,   0, 255), 1, 4);
}


void Loupe::dessiner_portion (cv::Mat &src, cv::Mat &dest)
{
    CHECK_MAT_TYPE(src, CV_8UC3)

    int bon_zoom = zoom >= 1 ? zoom : 1;

    for (int y = 0; y < dest.rows; y++)
    for (int x = 0; x < dest.cols; x++)
    {
        int x0 = zoom_x0 + x / bon_zoom;
        int y0 = zoom_y0 + y / bon_zoom;

        if (x0 < 0 || x0 >= src.cols || y0 < 0 || y0 >= src.rows) {
            dest.at<cv::Vec3b>(y,x)[0] = 64;
            dest.at<cv::Vec3b>(y,x)[1] = 64;
            dest.at<cv::Vec3b>(y,x)[2] = 64;
            continue;
        }
        dest.at<cv::Vec3b>(y,x)[0] = src.at<cv::Vec3b>(y0,x0)[0];
        dest.at<cv::Vec3b>(y,x)[1] = src.at<cv::Vec3b>(y0,x0)[1];
        dest.at<cv::Vec3b>(y,x)[2] = src.at<cv::Vec3b>(y0,x0)[2];
    }
}


void Loupe::afficher_tableau_valeurs (cv::Mat &src, int ex, int ey, int rx, int ry)
{
    CHECK_MAT_TYPE(src, CV_32SC1)

    int bon_zoom = zoom >= 1 ? zoom : 1;
    int cx = zoom_x0 + ex / bon_zoom;
    int cy = zoom_y0 + ey / bon_zoom;
    int x1 = cx-rx, x2 = cx+rx, y1 = cy-ry, y2 = cy+ry;

    int vmin = 0, vmax = 0;
    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            if (x < 0 || x >= src.cols || y < 0 || y >= src.rows)
                continue;
            int v = src.at<int>(y,x);
            if (v < vmin) vmin = v;
            else if (v > vmax) vmax = v;
        }
    }
    int n1 = ceil(log10(abs(vmin)+1)), n2 = ceil(log10(abs(vmax)+1));
    if (vmin < 0) n1++; // signe 
    if (vmax < 0) n2++;
    int n = std::max(n1,n2); // nb de chiffres

    std::cout << "Valeurs autour de " << cx << "," << cy << " :" << std::endl;
    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            if (x < 0 || x >= src.cols || y < 0 || y >= src.rows)
                 std::cout << std::setw(n+1) << "* " ;  // hors de l'image
            else std::cout << std::setw(n) << src.at<int>(y,x) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}


//---------------------------- C O U L E U R S --------------------------------

void representer_en_couleurs_vga (cv::Mat img_niv, cv::Mat img_coul)
{
    CHECK_MAT_TYPE(img_niv, CV_32SC1)
    CHECK_MAT_TYPE(img_coul, CV_8UC3)

    unsigned char couls[4][3] = {  // R, G, B
        {   0,   0,   0 },   //  0  black           ->  0 uniquement
        { 200, 200, 200 },   //  7  light gray      ->  1, 3, 5, ...
        { 110, 110, 140 },   //  8  dark gray       ->  2, 4, 6, ...
        { 252, 252, 252 },   // 15  white           -> 255 uniquement
    };

    for (int y = 0; y < img_niv.rows; y++)
    for (int x = 0; x < img_niv.cols; x++)
    {
        int g = img_niv.at<int>(y,x), c = 0;
        if (g == 255) c = 15;                      // seul 255 est blanc
        else if (g != 0) c = 1 + abs(g-1) % 14;    // seul 0 est noir
        // Attention img_coul est en B, G, R -> inverser les canaux
        img_coul.at<cv::Vec3b>(y,x)[0] = couls[c][2];
        img_coul.at<cv::Vec3b>(y,x)[1] = couls[c][1];
        img_coul.at<cv::Vec3b>(y,x)[2] = couls[c][0];
    }
}


void inverser_couleurs (cv::Mat img)
{
    CHECK_MAT_TYPE(img, CV_8UC3)

    for (int y = 0; y < img.rows; y++)
    for (int x = 0; x < img.cols; x++)
    {
        img.at<cv::Vec3b>(y,x)[0] = 255 - img.at<cv::Vec3b>(y,x)[0];
        img.at<cv::Vec3b>(y,x)[1] = 255 - img.at<cv::Vec3b>(y,x)[1];
        img.at<cv::Vec3b>(y,x)[2] = 255 - img.at<cv::Vec3b>(y,x)[2];
    }
}

