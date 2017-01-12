// Grab.cpp
/*
    Note: Before getting started, Basler recommends reading the Programmer's Guide topic
    in the pylon C++ API documentation that gets installed with pylon.
    If you are upgrading to a higher major version of pylon, Basler also
    strongly recommends reading the Migration topic in the pylon C++ API documentation.

    This sample illustrates how to grab and process images using the CInstantCamera class.
    The images are grabbed and processed asynchronously, i.e.,
    while the application is processing a buffer, the acquisition of the next buffer is done
    in parallel.

    The CInstantCamera class uses a pool of buffers to retrieve image data
    from the camera device. Once a buffer is filled and ready,
    the buffer can be retrieved from the camera object for processing. The buffer
    and additional image data are collected in a grab result. The grab result is
    held by a smart pointer after retrieval. The buffer is automatically reused
    when explicitly released or when the smart pointer object is destroyed.
*/

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#include <pylon/PylonBase.h>
#include <pylon/PylonImage.h>
#include <pylon/gige/BaslerGigEDeviceInfo.h>
#include <pylon/gige/BaslerGigEInstantCamera.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"

// Namespace for using pylon objects.
using namespace Pylon;

// Namespace for using cout.
using namespace std;

// Number of images to be grabbed.
static const uint32_t c_countOfImagesToGrab = 10000;

int main(int argc, char* argv[])
{
    // The exit code of the sample application.
    int exitCode = 0;

    int id = argc == 0 ? 0 : atoi(argv[1]);

    // Before using any pylon methods, the pylon runtime must be initialized.
    PylonInitialize();

    try
    {
    	DeviceInfoList devices;
        CTlFactory::GetInstance().EnumerateDevices(devices);
        CDeviceInfo dev = devices[id];

    	// Create an instant camera object with the camera device found first.
        CBaslerGigEInstantCamera camera( CTlFactory::GetInstance().CreateDevice(dev));
        camera.GevStreamChannelSelector.SetValue(Basler_GigECamera::GevStreamChannelSelector_StreamChannel0);
        camera.ExpertFeatureAccessSelector.SetValue(Basler_GigECamera::ExpertFeatureAccessSelector_ExpertFeature5);
        camera.ExpertFeatureEnable.SetValue(true);
        camera.GevSCPD.SetValue(450, false);

        // Print the model name of the camera.
        cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;


        Pylon::CBaslerGigEDeviceInfo info = static_cast<Pylon::CBaslerGigEDeviceInfo>(camera.GetDeviceInfo());
        cout << "IP:" << info.GetIpAddress() << " Port:" << info.GetPortNr() << endl;

        // The parameter MaxNumBuffer can be used to control the count of buffers
        // allocated for grabbing. The default value of this parameter is 10.
        camera.MaxNumBuffer = 5;

        // Start the grabbing of c_countOfImagesToGrab images.
        // The camera device is parameterized with a default configuration which
        // sets up free-running continuous acquisition.
        camera.StartGrabbing( c_countOfImagesToGrab);

        // This smart pointer will receive the grab result data.
        CGrabResultPtr ptrGrabResult;
        CPylonImage image;
        CImageFormatConverter fc;
        fc.OutputPixelFormat = Pylon::PixelType_RGB8packed;


        ////////// piping ///////////
        /*int pipe = mkfifo("/tmp/basler", S_IRWXU | S_IRWXG);
        if (pipe) {
        	cout << "Error: " << errno;
        	return errno;
        }*/
       // int pipe = open("/tmp/basler", O_WRONLY);
       // cout << "Pipe FD: " << pipe << endl;
        /////////////////////////////


        // Camera.StopGrabbing() is called automatically by the RetrieveResult() method
        // when c_countOfImagesToGrab images have been retrieved.
        while ( camera.IsGrabbing())
        {
            // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
            camera.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException);

            // Image grabbed successfully?
            if (ptrGrabResult->GrabSucceeded())
            {
                // Access the image data.
                //cout << "SizeX: " << ptrGrabResult->GetWidth() << endl;
                //cout << "SizeY: " << ptrGrabResult->GetHeight() << endl;
                fc.Convert(image, ptrGrabResult);
             //   int t;
             //   if (t=write(pipe, image.GetBuffer(), image.GetImageSize()) < 0) {
             //   	cout << "Error: " << errno << endl;
             //   }
             //   cout << "Wrote: " << t << endl;
                cv::Mat cv_img = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3,    (uint8_t*)image.GetBuffer());
                cv::imshow("grab",cv_img);
                cv::waitKey(1);
                const uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();
                //cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0] << endl << endl;

#ifdef PYLON_WIN_BUILD
                // Display the grabbed image.
                Pylon::DisplayImage(1, ptrGrabResult);
#endif
            }
            else
            {
                cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
            }
        }
    }
    catch (const GenericException &e)
    {
        // Error handling.
        cerr << "An exception occurred." << endl
        << e.GetDescription() << endl;
        exitCode = 1;
    }

    // Comment the following two lines to disable waiting on exit.
    cerr << endl << "Press Enter to exit." << endl;
    while( cin.get() != '\n');

    // Releases all pylon resources.
    PylonTerminate();

    return exitCode;
}
