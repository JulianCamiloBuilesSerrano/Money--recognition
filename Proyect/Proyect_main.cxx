#include <iostream>
#include <array>
#include <math.h>
#include <deque>
#include <opencv2/opencv.hpp>
using namespace cv;
typedef std::array<float, 256> hisImg;
typedef std::array<Mat, 10> imgCont;
typedef std::array<int, 2> top;

int main(int argc, char const *argv[])
{
    hisImg histo;
    imgCont savedImages;
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

    //Read image
    Mat imageColor = imread(argv[1], 1);

    Mat image = Mat::zeros(imageColor.rows, imageColor.cols, imageColor.type());
    cvtColor(imageColor, image, COLOR_BGR2GRAY,0 );
    imwrite("results/grey.png", image);
    std::cout<<image.type()<<"\n";
    //final image
    Mat dst = Mat::zeros(image.rows, image.cols, image.type());
    //aplay the threshold color
    Mat m1 = Mat::zeros(image.rows, image.cols, image.type());
    threshold(image, m1, 0, 255, THRESH_BINARY | THRESH_OTSU);
    imwrite("results/salida0_binarizada.png", m1);
    Mat element = getStructuringElement(MORPH_RECT,
                                        Size(5, 5),
                                        Point(-1, -1));
    for (int i = 0; i < 5; i++)
        dilate(m1, m1, element);
    imwrite("results/salida1_binarizadaDilatada.png", m1);

    threshold(image, dst, 0, 255, THRESH_TOZERO | THRESH_OTSU);
    //save the image
    imwrite("results/salida2_umbralizada.png", dst);

    element = getStructuringElement(MORPH_ELLIPSE,
                                    Size(5, 5),
                                    Point(-1, -1));
    for (int i = 0; i < 100; i++)
    {
        erode(dst, dst, element);
        dilate(dst, dst, element);
    }

    imwrite("results/salida3_umbralizadaSinRuido.png", dst);
    //create the figure
    element = getStructuringElement(MORPH_RECT,
                                    Size(3, 3),
                                    Point(-1, -1));

    //use erode
    Mat ero = Mat::zeros(image.rows, image.cols, image.type());
    erode(dst, ero, element);
    //dilated
    Mat dil = Mat::zeros(image.rows, image.cols, image.type());
    dilate(dst, dil, element);

    subtract(dil, ero, dst, noArray(), dst.type());

    imwrite("results/salida4_bordes.png", dst);
    Mat kernel = Mat::ones(3, 3, CV_64F);
    //set values of the kernel
    kernel.at<double>(0, 0) = -1;
    kernel.at<double>(0, 1) = -1;
    kernel.at<double>(0, 2) = -1;
    kernel.at<double>(1, 0) = -1;
    kernel.at<double>(1, 1) = 9;
    kernel.at<double>(1, 2) = -1;
    kernel.at<double>(2, 0) = -1;
    kernel.at<double>(2, 1) = -1;
    kernel.at<double>(2, 2) = -1;
    filter2D(dst, dst, dst.depth(), kernel, Point(-1, -1), 0, BORDER_DEFAULT);
    imwrite("results/salida5_mejoraDebordes.png", dst);
}