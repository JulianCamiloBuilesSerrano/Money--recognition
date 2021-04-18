#include <iostream>
#include <array>
#include <math.h>
#include <opencv2/opencv.hpp>
using namespace cv;
typedef std::array<float, 256> hisImg;

int main(int argc, char const *argv[])
{
    hisImg histo;
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

    // Read an image
    std::cout<<argc<<"\n";
    Mat image;
    image = imread(argv[1], 1);

    if (!image.data)
    {
        std::cerr << "Error: No image data" << std::endl;
        return (-1);
    }
    
}