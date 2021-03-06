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

bool isHex(string hexstr)
{
    for (auto it = hexstr.begin(); it != hexstr.end(); it++)
    {
        if (!isxdigit(*it))
        {
            return false;
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    read_args(argc, argv);
    string imgPath = "images/" + args["-i"] + ".ppm";
    //cout << "" << imgPath.c_str() << endl;
    string hexstring = args["-c"];
    string shapetocolor = args["-s"];
    if (!(shapetocolor == "rectangle" || shapetocolor == "triangle" || shapetocolor == "circle"))
    {
        cout << "invalid shape: program only accepts rectanlge triangle or circle" << endl;
        exit(1);
    }
    int redval = 0;
    int greenval = 0;
    int blueval = 0;
    cout << "Hex" << endl;
    //cout << hex.front() << endl;
    cout << hexstring << endl;
    cout << hexstring.substr(0, 2) << endl;
    CvScalar usecolor;
    if (hexstring.substr(0, 2) == "0x")
    {
        if (hexstring.length() != 8)
        {
            cout << "Hex should be 3 bytes, one byte each representing a color value" << endl;
            exit(1);
        }
        int hexval = 0;
        string newhexstring = hexstring.substr(2);
        string rhex = hexstring.substr(2, 2);
        string ghex = hexstring.substr(4, 2);
        string bhex = hexstring.substr(6);
        bool validhex = isHex(newhexstring);
        if (!validhex)
        {
            cout << "Ivalid Hex, Hex should only have values [0-9a-fA-F]" << endl;
            exit(1);
        }
        cout << "You have inputed a valid hex" << endl;
        stringstream strstream;
        strstream << rhex;
        strstream >> std::hex >> redval;
        stringstream().swap(strstream);
        strstream << ghex;
        strstream >> std::hex >> greenval;
        stringstream().swap(strstream);
        strstream << bhex;
        strstream >> std::hex >> blueval;

        //redval = ((0xb6feaa >> 16) & 255) / 255;
        //greenval = ((hexval >> 8) & 0xFF) / 255;
        //blueval = (hexval & 0xFF) / 255;
        //cout << redval << endl;
        //cout << greenval << endl;
        //cout << blueval << endl;
        //redval = 100;
        //greenval = 200;
        //blueval = 100;
        usecolor = cvScalar(blueval, greenval, redval);
    }
    else if (hexstring == "blue")
    {
        blueval = 255;
        usecolor = cvScalar(255, 0, 0);
    }
    else if (hexstring == "green")
    {
        greenval = 255;
        usecolor = cvScalar(0, 255, 0);
    }
    else if (hexstring == "red")
    {
        redval = 255;
        usecolor = cvScalar(0, 0, 255);
    }
    else if (hexstring == "black")
    {
        blueval = 255;
        greenval = 255;
        redval = 255;
        usecolor = cvScalar(255, 255, 255);
    }
    else
    {
        cout << "Invalid Input: Please use either red, green, blue, black, or a hex value such as 0xFFFFFF for the -c flag" << endl;
        exit(1);
    }

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
    if (shapetocolor == "triangle" || shapetocolor == "rectangle")
    {
        while (contours)
        {
            //obtain a sequence of points of contour, pointed by the variable 'contour'
            result = cvApproxPoly(contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP, cvContourPerimeter(contours) * 0.02, 0);

            //if there are 3  vertices  in the contour(It should be a triangle)
            if (shapetocolor == "triangle")
            {
                if (result->total == 3)
                {
                    //iterating through each point
                    CvPoint *pt[3];
                    for (int i = 0; i < 3; i++)
                    {
                        pt[i] = (CvPoint *)cvGetSeqElem(result, i);
                    }

                    //drawing lines around the triangle
                    cvLine(img, *pt[0], *pt[1], usecolor, 4);
                    cvLine(img, *pt[1], *pt[2], usecolor, 4);
                    cvLine(img, *pt[2], *pt[0], usecolor, 4);
                }
            }
            else if (shapetocolor == "rectangle")
            {
                //if there are 4 vertices in the contour(It should be a rectangle/square)

                if (result->total == 4)
                {
                    //iterating through each point
                    CvPoint *pt[4];
                    for (int i = 0; i < 4; i++)
                    {
                        pt[i] = (CvPoint *)cvGetSeqElem(result, i);
                    }

                    //drawing lines around the rectangle/square
                    cvLine(img, *pt[0], *pt[1], usecolor, 4);
                    cvLine(img, *pt[1], *pt[2], usecolor, 4);
                    cvLine(img, *pt[2], *pt[3], usecolor, 4);
                    cvLine(img, *pt[3], *pt[0], usecolor, 4);
                }
            }
            //obtain the next contour
            contours = contours->h_next;
        }
    }

    // apply hough circles to find circles and draw a cirlce around it
    if (shapetocolor == "circle")
    {
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
                     cvRound(p[2]), CV_RGB(redval, greenval, blueval), 3, 8, 0);
        }
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
