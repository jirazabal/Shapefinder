#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

map<string, string> args;
void read_args(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    read_args(argc, argv);

    string imgPath = "images/" + args["-i"] + ".ppm";
    //cout << "" << imgPath.c_str() << endl;

    IplImage *img = cvLoadImage(imgPath.c_str());

    //show the original image
    cvNamedWindow("Original Image");
    cvShowImage("Original Image", img);

    //converting the original image into grayscale
    IplImage *imgGrayScale = cvCreateImage(cvGetSize(img), 8, 1);
    IplImage *imgCanny = cvCreateImage(cvGetSize(img), 8, 1);
    cvCvtColor(img, imgGrayScale, CV_BGR2GRAY);

    //thresholding the grayscale image to get better results
    cvThreshold(imgGrayScale, imgGrayScale, 128, 255, CV_THRESH_BINARY);

    CvSeq *contours;                               //hold the pointer to a contour in the memory block
    CvSeq *result;                                 //hold sequence of points of a contour
    CvMemStorage *storage = cvCreateMemStorage(0); //storage area for all contours

    //finding all contours in the image
    cvFindContours(imgGrayScale, storage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

    //iterating through each contour
    while (contours)
    {
        //obtain a sequence of points of contour, pointed by the variable 'contour'
        result = cvApproxPoly(contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP, cvContourPerimeter(contours) * 0.02, 0);

        //if there are 3  vertices  in the contour(It should be a triangle)
        if (result->total == 3)
        {
            //iterating through each point
            CvPoint *pt[3];
            for (int i = 0; i < 3; i++)
            {
                pt[i] = (CvPoint *)cvGetSeqElem(result, i);
            }

            //drawing lines around the triangle
            cvLine(img, *pt[0], *pt[1], cvScalar(255, 0, 0), 4);
            cvLine(img, *pt[1], *pt[2], cvScalar(255, 0, 0), 4);
            cvLine(img, *pt[2], *pt[0], cvScalar(255, 0, 0), 4);
        }

        //if there are 4 vertices in the contour(It should be a rectangle/square)
        else if (result->total == 4)
        {
            //iterating through each point
            CvPoint *pt[4];
            for (int i = 0; i < 4; i++)
            {
                pt[i] = (CvPoint *)cvGetSeqElem(result, i);
            }

            //drawing lines around the rectangle/square
            cvLine(img, *pt[0], *pt[1], cvScalar(0, 255, 0), 4);
            cvLine(img, *pt[1], *pt[2], cvScalar(0, 255, 0), 4);
            cvLine(img, *pt[2], *pt[3], cvScalar(0, 255, 0), 4);
            cvLine(img, *pt[3], *pt[0], cvScalar(0, 255, 0), 4);
        }

        //obtain the next contour
        contours = contours->h_next;
    }
    // apply hough circles to find circles and draw a cirlce around it
    cvCanny(imgGrayScale, imgCanny, 0, 0, 3);
    CvSeq *circles = cvHoughCircles(imgCanny,
                                    storage,
                                    CV_HOUGH_GRADIENT,
                                    2,
                                    imgCanny->height / 2,
                                    200,
                                    100);

    for (int i = 0; i < circles->total; i++)
    {
        float *p = (float *)cvGetSeqElem(circles, i);
        cvCircle(img, cvPoint(cvRound(p[0]), cvRound(p[1])),
                 3, CV_RGB(0, 255, 0), -1, 8, 0);
        cvCircle(img, cvPoint(cvRound(p[0]), cvRound(p[1])),
                 cvRound(p[2]), CV_RGB(0, 255, 255), 3, 8, 0);
    }
    
    //saves image
    if (args.find("-o") == args.end())
    {
        cout << "No output file specified!" << endl;
    }
    else 
    {
        string outputName = args["-o"] + ".png";
        Mat src;
        src = cvarrToMat(img);
        imshow("src", src);
        waitKey(0);
        cvReleaseImage(&img);
        imwrite(outputName, src);
    }


    //cleaning up
    cvDestroyAllWindows();
    cvReleaseMemStorage(&storage);
    cvReleaseImage(&img);
    cvReleaseImage(&imgGrayScale);

    return 0;
}

void read_args(int argc, char *argv[])
{
    int cnt = (argc - 1) / 2;

    cout << "num_arguments: " << cnt << endl;

    for (int i = 0; i < cnt; i++)
    {
        args[argv[(2 * i) + 1]] = argv[(2 * i) + 2];
    }

    for (auto it = args.cbegin(); it != args.cend(); it++)
    {
        cout << "args[\"" << it->first << "\"] = \"" << it->second << "\"\n";
    }
}
