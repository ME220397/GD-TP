#include <iostream>
#include "DGtal/base/Common.h"
#include "DGtal/io/readers/GenericReader.h"
#include "DGtal/images/ImageHelper.h"
#include "DGtal/images/Image.h"
#include "ConfigExamples.h"

#include "DGtal/helpers/StdDefs.h"

#include "DGtal/io/viewers/Viewer3D.h"
#include "DGtal/io/DrawWithDisplay3DModifier.h"
#include "DGtal/io/colormaps/HueShadeColorMap.h"
#include "DGtal/io/Color.h"

#include "DGtal/kernel/SpaceND.h"
#include "DGtal/kernel/domains/HyperRectDomain.h"
#include "DGtal/images/ImageSelector.h"

#include "DGtal/geometry/volumes/distance/DistanceTransformation.h"
#include "DGtal/images/SimpleThresholdForegroundPredicate.h"
#include "DGtal/helpers/StdDefs.h"

///////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace DGtal;



struct hueFct{
 inline
 unsigned int operator() (unsigned int aVal) const
  {
    HueShadeColorMap<unsigned int>  hueShade(0,255);
    Color col = hueShade((unsigned int)aVal);
    return  (((unsigned int) col.red()) <<  16)| (((unsigned int) col.green()) << 8)|((unsigned int) col.blue());
  }
};

// Définition du type de viewer à utiliser. 
typedef Viewer3D<> ViewerType ;
// Définition du type de conteneur à utiliser pour l'image du premier exercice. 
typedef ImageContainerBySTLVector<Z3i::Domain,  float> Image3D;
// Définition du type de conteneur à utiliser pour l'image de la transformée en distance. 
typedef ImageSelector<Z3i::Domain, unsigned char>::Type Image;

// Parcours et traitement d'un volume. 
void imageSandbox(ViewerType& viewer, std::string filename);
// Tranformée en distance.
void transformeeEnDistance(ViewerType& viewer, std::string filename);

// Méthode pour générer des voxels de manière aléatoire. 
template<typename Image>
void randomSeeds(Image &image, const unsigned int nb, const int value);

///////////////////////////////////////////////////////////////////////////////

int main( int argc, char** argv )
{
  QApplication application(argc,argv);
  ViewerType viewer;
  
  // Appel aux méthodes des exercices.
  if (argc > 2)
  {
    std::stringstream ssAlgo;
    ssAlgo << std::string(argv[1]);

    std::stringstream ssFile;
    ssFile << std::string(argv[2]);
    if (ssAlgo.str() == "Sandbox")
      imageSandbox(viewer, ssFile.str());
    else if (ssAlgo.str() == "DT")
      transformeeEnDistance(viewer, ssFile.str());
  }
  else
  {
    std::cout << "Les paramètres n'a pas été fourni." << std::endl;
    std::cout << "startCode <paramètre = {Sandbox | DT}> <nom du fichier>" << std::endl;
    return 0;
  }
  
  return application.exec();
}


/**
 * Cette fonction vous permettra de commencer à pratiquer avec le chargement d'objets 
 * volumiques. Les parcourir, retrouver les valeurs affectées à chaque voxel et les 
 * les modifier. 
 * \param Visualisateur à utiliser.
 * \param Nom du fichier.
 *
 */
void imageSandbox(ViewerType& viewer, std::string filename)
{
  // Lance le visusalisateur. 
  viewer.show();
  
  //Chargement d'une image dans une structure de données ImageContainerBySTLVector.
  std::string inputFilename = examplesPath + "/" + filename;
  Image3D image = GenericReader<Image3D>::import(inputFilename);

  // Obtention du domaine (taille) de l'image chargée. 
  Z3i::Domain initialDomain = image.domain();
  Z3i::Point pointInitial (220, 50, 10);
  Z3i::Point pointFinal (260, 100, 20);
  Z3i::Domain sousDomain(pointInitial, pointFinal);

  Image3D sousImage(sousDomain);

  // Définition du gradient des couleurs. 
  GradientColorMap<long> gradient( 0,30);
  gradient.addColor(Color::Red);
  gradient.addColor(Color::Yellow);
  gradient.addColor(Color::Green);
  gradient.addColor(Color::Cyan);
  gradient.addColor(Color::Blue);
  gradient.addColor(Color::Magenta);
  gradient.addColor(Color::Red);

  float min = 0.0;
  float max = 0.0;

  // Calcul du centre du domain
  float n_voxels = 0;
  Z3i::Point pt(0.0, 0.0, 0.0);
  for(Z3i::Domain::ConstIterator it= image.domain().begin(),
	itend = image.domain().end(); it != itend; ++it)
  {
    pt += *it;
    n_voxels += 1;
  }

  float x = pt[0]/n_voxels;
  float y = pt[1]/n_voxels;
  float z = pt[2]/n_voxels;

  Z3i::Point centre(x, y, z); // centre du volume

  for(Z3i::Domain::ConstIterator it= image.domain().begin(),
	itend = image.domain().end(); it != itend; ++it)
  {

    //calcul de la distance
    Z3i::Point p = *it;
    p = p - centre;
    double dist = p[0]*p[0] + p[1]*p[1] + p[2]*p[2];

    Color c= gradient(dist);
    viewer << CustomColors3D(Color((float)(c.red()),
          (float)(c.green()),
          (float)(c.blue(),205)),
          Color((float)(c.red()),
          (float)(c.green()),
          (float)(c.blue()),205));

    if (image(*it) >0){
      viewer << *it;
    }

    /*Z3i::Point pt = *it;
    if(pt[0]>=pointInitial[0] && pt[1] >= pointInitial[1] && pt[2] >= pointInitial[2] &&
       pt[0]<=pointFinal[0] && pt[1] <= pointFinal[1] && pt[2] <= pointFinal[2]){
       sousImage.setValue(*it, image(*it));
       viewer << *it;
    }*/

    /*if ((*it)[0] > 10 ){
      viewer << *it;
    }*/
  }

  viewer << SetMode3D(image.className(), "BoundingBox");
  viewer << ViewerType::updateDisplay;
  
}

/**
 * Fonction de la transformée en distance à partir de quelques points germes.
 * La distance est calculée à partir de chaque point. Donc, la distance dans un 
 * voxel est la distance minimale à tous les points germes. 
 * \param le visualisateur à utiliser. 
 * \param Nom du fichier.
 *
 */
void transformeeEnDistance(ViewerType& viewer, std::string filename)
{
  // Affichage de la visualisation. 
  viewer.show();
  // Nombre du fichier à charger. 
  std::string inputFilename = examplesPath + "/" + filename;

  // Création du type d'image. 
  //Default image selector = STLVector
  typedef ImageSelector<Z3i::Domain, unsigned char>::Type Image;

  //Chargement du fichier image dans la structure. 
  Image image = VolReader<Image>::importVol( inputFilename );
  // Obtention du domaine (taille) de l'image. 
  Z3i::Domain domain = image.domain();

   Image imageSeeds ( domain);
   for ( Image::Iterator it = imageSeeds.begin(), itend = imageSeeds.end();it != itend; ++it)
     (*it)=1;
   //imageSeeds.setValue(p0, 0 );
   randomSeeds(imageSeeds, 70, 0);
    

  typedef functors::SimpleThresholdForegroundPredicate<Image> Predicate;
  Predicate aPredicate(imageSeeds,0);
  
  // Création de type et de l'objet pour appliquer la transformée. 
  typedef  DistanceTransformation<Z3i::Space,Predicate, Z3i::L2Metric> DTL2;
  DTL2 dtL2(&domain, &aPredicate, &Z3i::l2Metric);

  // Detection des distances minimales et maximales. 
  unsigned int min = 0;
  unsigned int max = 0;
  for(DTL2::ConstRange::ConstIterator it = dtL2.constRange().begin(),
	itend=dtL2.constRange().end();
      it!=itend;
      ++it)
  {
    if(  (*it) < min )
      min=(*it);
    if( (*it) > max )
      max=(*it);
  }
  
  //Spécification des gradients de couleur pour la visualisation.
  GradientColorMap<long> gradient( 0,30);
  gradient.addColor(Color::Red);
  gradient.addColor(Color::Yellow);
  gradient.addColor(Color::Green);
  gradient.addColor(Color::Cyan);
  gradient.addColor(Color::Blue);
  gradient.addColor(Color::Magenta);
  gradient.addColor(Color::Red);

  // Affectation du mode de visualisation 3D. 
  viewer << SetMode3D( (*(domain.begin())).className(), "Paving" );

  // Parcours de tous les voxels de l'image avec un iterateur sur le domaine.  
  for(Z3i::Domain::ConstIterator it = domain.begin(), itend=domain.end();
      it!=itend;
      ++it)
  {

    // Calcul de la transformée en distance pour le voxel courant. 
    double valDist= dtL2( (*it) );
    // Calcul du gradient de couleur pour cette distance.
    Color c= gradient(valDist);
    viewer << CustomColors3D(Color((float)(c.red()),
          (float)(c.green()),
          (float)(c.blue(),205)),
          Color((float)(c.red()),
          (float)(c.green()),
          (float)(c.blue()),205));
    // Le viewer reçoit le prochain voxel pour visualisation.
    if (image(*it) > 0)
      viewer << *it ;
  }
  
  //viewer << ClippingPlane(0,1,0, -40) << Viewer3D<>::updateDisplay;
  // Mise à jour du visualisateur après le parcours de tous le voxels. 
  viewer<< Viewer3D<>::updateDisplay;
}


/**
 * Cette fonction genère un ensemble de points afin de les placer 
 * dans le volume comme les germes de la transformée en distance. 
 * \param image.
 * \param nombre de germes.
 * \param value à affecter comme seuil. 
 *
 */ 
template<typename Image>
 void randomSeeds(Image &image, const unsigned int nb, const int value)
 {
   typename Image::Point p, low = image.domain().lowerBound();
   typename Image::Vector ext;
   srand ( time(NULL) );
 
   ext = image.extent();
 
   for (unsigned int k = 0 ; k < nb; k++)
     {
       for (unsigned int dim = 0; dim < Image::dimension; dim++)
         p[dim] = rand() % (ext[dim]) +  low[dim];
 
       image.setValue(p, value);
     }
 }
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
