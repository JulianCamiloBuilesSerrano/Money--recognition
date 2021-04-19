#include <iostream>
#include <array>
#include <math.h>
#include <deque>
#include <opencv2/opencv.hpp>
using namespace cv;
typedef std::array<float, 256> hisImg;
typedef std::array<Mat, 10> imgCont;
typedef std::array<int,2> top;
hisImg histograma(Mat otiginal);
top variance(hisImg histogram, int size);
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

    // Read an images
    /*for (int i = 1; i < 11; i++)
    {
        savedImages = imread(argv[i], 1);
        
        if (!savedImages[i].data)
        {
            std::cerr << "Error: No image data" << std::endl;
            return (-1);
        }
    } */
    Mat image;
    image = imread(argv[1], 1);
    hisImg histogram = histograma(image);
    top threshoBest = variance(histogram,image.rows * image.cols);
    Mat dst =  Mat::zeros(image.rows, image.cols, image.type()); 
    Mat dst2 =  Mat::zeros(image.rows, image.cols, image.type()); 
    threshold(image,dst,threshoBest[1],255,THRESH_TOZERO );
    
    imwrite("results/salida.png",dst);
    threshold(image,dst,threshoBest[1],255,THRESH_TOZERO);
    imwrite("results/salida2.png",image);
    Mat element = getStructuringElement(  MORPH_RECT,
                       Size( 3+1, 3+1),
                       Point( 3, 3 ) );
    std::cout<<element<<"\n";
    
     erode( image, dst, element );
     subtract(dst2,dst,dst,noArray(),1);
     imwrite("results/salida3.png",dst);
}


top variance(hisImg histogram, int size)
{
    float var = 0;
    float max = -INFINITY;
    float max2 = -INFINITY;
    top values;
    int umbral = 0;
    int umbral2 = 0;
    float q1 = 0 ;
    float q2 = 0;
    float u1 = 0;
    float u2 = 0; 
    float sumb = 0 ;
    float sum = 0;
    for(int i = 0 ; i < 256; i++)
        sum += i *histogram[i];
    for (int i = 0; i < 256; i++)
    {
        q1 += histogram[i];
        if (q1 == 0)
            continue;
        else {
            q2 = size - q1;
            sumb += i*histogram[i];
            u1 = sumb/q1;
            u2 = (sum -sumb)/q2;
            var = q1 * q2 *(u1 -u2)*(u1 -u2);
            if(var > max){
                values[0] = i;
                max = var;
            }
            else if(var > max2 && max2 < max){
                values[1] = i;
                max2 =  var;
            }   
        } 
    }
    
    return values;
}

hisImg histograma(Mat original)
{
    hisImg hist = {0};
    for (int i = 0; i < original.rows; i++)
    {
        for (int j = 0; j < original.cols; j++)
        {
            hist[original.at<uchar>(i, j)]++;
        }
    }
/*
    for (int i = 0; i < 256; i++)
    {
        std::cout << i << ":" << hist[i] << "\n";
    }*/
    return hist;
}