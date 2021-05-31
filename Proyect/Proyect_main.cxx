#include <iostream>
#include <array>
#include <math.h>
#include <deque>
#include <list>
#include <opencv2/opencv.hpp>
using namespace cv;
struct Billete
{
    std::string denominacion;
    Mat cara1;
    Mat cara2;
    Vec3b canalesC1;
    Vec3b canalesC2;
};
struct Punto
{
    int i;
    int j;
    int color;
};

typedef std::list<Punto> Region;
typedef std::list<Region> Regiones;
typedef std::array<Billete, 5> BD;
struct RegionBillete
{
    Region region;
    Vec3b PromedioColor;
    std::string denominacion;
};
typedef std::list<RegionBillete> ReBillete;
bool aceptacion(Mat *image, Punto p);
std::queue<Punto> vecinos(Mat *image, Punto p);
Region crecimientoRegion(Mat *image, Mat *estados, int i, int j);
Regiones estiquetadoRegiones(Mat *image);
Vec3b promedio_imagen(Mat image);
BD lectura_BD();
ReBillete evualuacionRegiones(Mat *image, Regiones regiones, BD bd);

int main(int argc, char const *argv[])
{
    BD bd = lectura_BD();
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << std::endl;
        return (-1);
    } // fi

    // Review given command line arguments
    std::cout << "-------------------------" << std::endl;
    for (int a = 0; a < argc; a++)
        std::cout << argv[a] << std::endl;
    std::cout << "-------------------------" << std::endl;

    /*---------------------------------------------

                Lectura de la imagen

    ------------------------------------------------*/

    Mat imageColor = imread(argv[1], 1);
    /*---------------------------------------------

                Hacer una copia en escala
                        de gris 

    ------------------------------------------------*/

    Mat image = Mat::zeros(imageColor.rows, imageColor.cols, imageColor.type());
    cvtColor(imageColor, image, COLOR_BGR2GRAY, 0);
    //impresion de la imagen en escala de gris
    imwrite("results/grey.png", image);
    std::cout << image.type() << "\n";

    /*---------------------------------------------

                aplicar la umbralizacion
                    binarizada 

    ------------------------------------------------*/
    Mat binarizadaDilatada = Mat::zeros(image.rows, image.cols, image.type());
    threshold(image, binarizadaDilatada, 0, 255, THRESH_BINARY | THRESH_OTSU);
    imwrite("results/salida0_binarizada.png", binarizadaDilatada);
    Mat element = getStructuringElement(MORPH_RECT,
                                        Size(5, 5),
                                        Point(-1, -1));
    //dilatar imagen
    for (int i = 0; i < 5; i++)
        dilate(binarizadaDilatada, binarizadaDilatada, element);
    for (int i = 0; i < 5; i++)
        erode(binarizadaDilatada, binarizadaDilatada, element);
    imwrite("results/salida1_binarizadaDilatada.png", binarizadaDilatada);

    /*---------------------------------------------

                aplicar la umbralizacion
                    no binarizada 

    ------------------------------------------------*/
    Mat umbralizada = Mat::zeros(image.rows, image.cols, image.type());
    threshold(image, umbralizada, 0, 255, THRESH_TOZERO | THRESH_OTSU);
    imwrite("results/salida2_umbralizada.png", umbralizada);

    /*---------------------------------------------

                aplicar erosion y dilatación
                para disminuir el ruido 

    ------------------------------------------------*/
    element = getStructuringElement(MORPH_ELLIPSE,
                                    Size(5, 5),
                                    Point(-1, -1));
    for (int i = 0; i < 100; i++)
    {
        erode(umbralizada, umbralizada, element);
        dilate(umbralizada, umbralizada, element);
    }

    imwrite("results/salida3_umbralizadaSinRuido.png", umbralizada);
    //create the figure
    element = getStructuringElement(MORPH_RECT,
                                    Size(3, 3),
                                    Point(-1, -1));

    /*---------------------------------------------

                se hace una identificación
                    de bordes

    ------------------------------------------------*/
    //use erode
    Mat ero = Mat::zeros(image.rows, image.cols, image.type());
    erode(umbralizada, ero, element);
    //dilated
    Mat dil = Mat::zeros(image.rows, image.cols, image.type());
    dilate(umbralizada, dil, element);

    subtract(dil, ero, umbralizada, noArray(), umbralizada.type());

    imwrite("results/salida4_bordes.png", umbralizada);
    Mat kernel = Mat::ones(3, 3, CV_64F);
    /*---------------------------------------------

                se aplica un fitro pasa altos
                    para la los bordes

    ------------------------------------------------*/
    kernel.at<double>(0, 0) = -1;
    kernel.at<double>(0, 1) = -1;
    kernel.at<double>(0, 2) = -1;
    kernel.at<double>(1, 0) = -1;
    kernel.at<double>(1, 1) = 9;
    kernel.at<double>(1, 2) = -1;
    kernel.at<double>(2, 0) = -1;
    kernel.at<double>(2, 1) = -1;
    kernel.at<double>(2, 2) = -1;
    filter2D(umbralizada, umbralizada, umbralizada.depth(), kernel, Point(-1, -1), 0, BORDER_DEFAULT);
    imwrite("results/salida5_mejoraDebordes.png", umbralizada);

    /*-----------------------------------------------

            Caractruzacion de la imagen

    --------------------------------------------------*/
    Regiones regiones = estiquetadoRegiones(&binarizadaDilatada);
    std::cout << regiones.size() << std::endl;
    evualuacionRegiones(&imageColor, regiones, bd);
    imwrite("imagencolor.png", imageColor);
}

bool aceptacion(Mat *image, Punto p)
{
    if (p.i <= 0 || p.j <= 0)
        return false;
    if (p.i >= image->rows - 1 || p.j >= image->cols - 1)
        return false;

    return true;
}
std::queue<Punto> vecinos(Mat *image, Punto p)
{
    //std::cout<<p.i<<" "<<p.j<<std::endl;
    std::queue<Punto> neigh;
    Punto nuevo;
    //up
    nuevo.i = p.i - 1;
    nuevo.j = p.j;
    if (aceptacion(image, nuevo))
        neigh.push(nuevo);
    //left
    nuevo.i = p.i;
    nuevo.j = p.j - 1;
    if (aceptacion(image, nuevo))
        neigh.push(nuevo);
    //right
    nuevo.i = p.i;
    nuevo.j = p.j + 1;
    if (aceptacion(image, nuevo))
        neigh.push(nuevo);
    //down
    nuevo.i = p.i + 1;
    nuevo.j = p.j;
    if (aceptacion(image, nuevo))
        neigh.push(nuevo);

    //up-left
    nuevo.i = p.i - 1;
    nuevo.j = p.j - 1;
    if (aceptacion(image, nuevo))
        neigh.push(nuevo);
    //up-right
    nuevo.i = p.i - 1;
    nuevo.j = p.j + 1;
    if (aceptacion(image, nuevo))
        neigh.push(nuevo);
    //down-left
    nuevo.i = p.i + 1;
    nuevo.j = p.j - 1;
    if (aceptacion(image, nuevo))
        neigh.push(nuevo);
    //down-right
    nuevo.i = p.i + 1;
    nuevo.j = p.j + 1;
    if (aceptacion(image, nuevo))
        neigh.push(nuevo);

    return neigh;
}
Region crecimientoRegion(Mat *image, Mat *estados, int i, int j)
{
    Region reg;
    std::queue<Punto> neighborhood;
    Punto p;
    p.i = i;
    p.j = j;
    neighborhood.push(p);
    while (!neighborhood.empty())
    {
        p = neighborhood.front();
        neighborhood.pop();
        reg.push_back(p); //add the pixel to the region list

        std::queue<Punto> neigh = vecinos(image, p); //find the neighbord
        while (!neigh.empty())                       //until all the neighbors are evaluated
        {
            Punto n = neigh.front();                                                              //get the first neighbor
            neigh.pop();                                                                          //delete the first
            if ((int)image->at<uchar>(n.i, n.j) == 255 && (int)estados->at<uchar>(n.i, n.j) == 0) //pixel is white and no evaluated
            {
                neighborhood.push(n); //add to the queue of valid neighbors
            }
            estados->at<uchar>(n.i, n.j) = (unsigned char)1; //set pixel as evaluated
        }
    }
    //return the region find
    return reg;
}
Regiones estiquetadoRegiones(Mat *image)
{
    //container of regions
    Regiones regiones;
    //mat that helps to set pxels that were evalueted
    Mat estado = Mat::zeros(image->rows, image->cols, image->type());

    for (int i = 0; i < image->rows; i++)
    {
        for (int j = 0; j < image->cols; j++)
        {
            if ((int)estado.at<uchar>(i, j) == 0 && (int)image->at<uchar>(i, j) != 0) //if the pixel isn´t evalute and is white
            {
                Region r = crecimientoRegion(image, &estado, i, j);
                regiones.push_back(r);
            }
        }
    }

    return regiones;
}
BD lectura_BD()
{
    BD billetes;
    Billete b;
    // lectura del billete de 2000

    b.denominacion = "2000";
    std::cout << "denominacion" << b.denominacion << std::endl;
    b.cara1 = imread("../Base de datos/a.jpeg", 1);
    b.canalesC1 = promedio_imagen(b.cara1);
    std::cout << "promedio " << b.canalesC1 << std::endl;
    b.cara2 = imread("../Base de datos/b.jpeg", 1);
    b.canalesC2 = promedio_imagen(b.cara2);
    std::cout << "promedio " << b.canalesC2 << std::endl;
    billetes[0] = b;
    // lectura del billete de 5000
    b.denominacion = "5000";
    std::cout << "denominacion" << b.denominacion << std::endl;
    b.cara1 = imread("../Base de datos/c.jpeg", 1);
    b.canalesC1 = promedio_imagen(b.cara1);
    std::cout << "promedio " << b.canalesC1 << std::endl;
    b.cara2 = imread("../Base de datos/d.jpeg", 1);
    b.canalesC2 = promedio_imagen(b.cara2);
    std::cout << "promedio " << b.canalesC2 << std::endl;
    billetes[1] = b;
    // lectura del billete de 10000
    b.denominacion = "10000";
    std::cout << "denominacion" << b.denominacion << std::endl;
    b.cara1 = imread("../Base de datos/e.jpeg", 1);
    b.canalesC1 = promedio_imagen(b.cara1);
    std::cout << "promedio " << b.canalesC1 << std::endl;
    b.cara2 = imread("../Base de datos/f.jpeg", 1);
    b.canalesC2 = promedio_imagen(b.cara2);
    std::cout << "promedio " << b.canalesC2 << std::endl;
    billetes[2] = b;
    // lectura del billete de 20000
    b.denominacion = "20000";
    std::cout << "denominacion" << b.denominacion << std::endl;
    b.cara1 = imread("../Base de datos/g.jpeg", 1);
    b.canalesC1 = promedio_imagen(b.cara1);
    std::cout << "promedio " << b.canalesC1 << std::endl;
    b.cara2 = imread("../Base de datos/h.jpeg", 1);
    b.canalesC2 = promedio_imagen(b.cara2);
    std::cout << "promedio " << b.canalesC2 << std::endl;
    billetes[3] = b;
    // lectura del billete de 50000
    b.denominacion = "50000";
    std::cout << "denominacion" << b.denominacion << std::endl;
    b.cara1 = imread("../Base de datos/i.jpeg", 1);
    b.canalesC1 = promedio_imagen(b.cara1);
    std::cout << "promedio " << b.canalesC1 << std::endl;
    b.cara2 = imread("../Base de datos/j.jpeg", 1);
    b.canalesC2 = promedio_imagen(b.cara2);
    std::cout << "promedio " << b.canalesC2 << std::endl;
    billetes[4] = b;
    /*
    for (int i = 0; i < cara1.rows; i++)
    {
        for (int j = 0; j < cara1.cols; j++)
        {
            //std::cout<<cara1.at<Vec3b>(i,j)<<std::endl;
        }
        
    }*/

    return billetes;
}
Vec3b promedio_imagen(Mat image)
{
    Vec3b promedio;
    int a = 0;
    int b = 0;
    int c = 0;
    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            a += (int)image.at<Vec3b>(i, j)[0];
            b = b + (int)image.at<Vec3b>(i, j)[1];
            c += (int)image.at<Vec3b>(i, j)[2];
        }
    }
    a = a / (image.rows * image.cols);
    b = b / (image.rows * image.cols);
    c = c / (image.rows * image.cols);
    promedio[0] = a;
    promedio[1] = b;
    promedio[2] = c;

    return promedio;
}

ReBillete evualuacionRegiones(Mat *image, Regiones regiones, BD bd)
{
    ReBillete billetes;
    Regiones::iterator it = regiones.begin();
    for (; it != regiones.end(); it++)
    {
        Region::iterator itr = it->begin();

        int a = 0;
        int b = 0;
        int c = 0;
        //recorremos cada supuesto billete
        for (; itr != it->end(); itr++)
        {
            a += image->at<Vec3b>(itr->i, itr->j)[0];
            b += image->at<Vec3b>(itr->i, itr->j)[1];
            c += image->at<Vec3b>(itr->i, itr->j)[2];
        }
        itr = it->begin();
        //guardamos los valores del posible billete
        RegionBillete billete;
        billete.region = *it;
        Vec3b col;
        col[0] = a / it->size();
        col[1] = b / it->size();
        col[2] = c / it->size();
        billete.PromedioColor = col;
        std::cout<<billete.PromedioColor<<std::endl;
         for (; itr != it->end(); itr++)
        {
            image->at<Vec3b>(itr->i, itr->j) = billete.PromedioColor;
        }
    }
    return billetes;
}
