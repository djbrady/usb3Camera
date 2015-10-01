#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <thread>
#include "FlyCapture2.h"

using namespace FlyCapture2;
using namespace std;

void PrintBuildInfo()
{
    FC2Version fc2Version;
    Utilities::GetLibraryVersion( &fc2Version );

	ostringstream version;
	version << "FlyCapture2 library version: " << fc2Version.major << "." << fc2Version.minor << "." << fc2Version.type << "." << fc2Version.build;
	cout << version.str() << endl;

	ostringstream timeStamp;
    timeStamp <<"Application build date: " << __DATE__ << " " << __TIME__;
	cout << timeStamp.str() << endl << endl;
}

void PrintCameraInfo( CameraInfo* pCamInfo )
{
    cout << endl;
	cout << "*** CAMERA INFORMATION ***" << endl;
	cout << "Serial number -" << pCamInfo->serialNumber << endl;
    cout << "Camera model - " << pCamInfo->modelName << endl;
    cout << "Camera vendor - " << pCamInfo->vendorName << endl;
    cout << "Sensor - " << pCamInfo->sensorInfo << endl;
    cout << "Resolution - " << pCamInfo->sensorResolution << endl;
    cout << "Firmware version - " << pCamInfo->firmwareVersion << endl;
    cout << "Firmware build time - " << pCamInfo->firmwareBuildTime << endl << endl;

}

void PrintError( Error error )
{
    error.PrintErrorTrace();
}

int InitializeCamera( PGRGuid guid )
{
    Error error;
    Camera cam;

    // Connect to a camera
    error = cam.Connect(&guid);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    // Get the camera information
    CameraInfo camInfo;
    error = cam.GetCameraInfo(&camInfo);

    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    // Initialize camera

    const Mode k_fmt7Mode = MODE_0;
    const PixelFormat k_fmt7PixFmt = PIXEL_FORMAT_RAW8;

    // Query for available Format 7 modes
    Format7Info fmt7Info;
    bool supported;
    fmt7Info.mode = k_fmt7Mode;
    error = cam.GetFormat7Info( &fmt7Info, &supported );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    Format7ImageSettings fmt7ImageSettings;
    fmt7ImageSettings.mode = k_fmt7Mode;
    fmt7ImageSettings.offsetX = 0;
    fmt7ImageSettings.offsetY = 0;
    fmt7ImageSettings.width = 1920;
    fmt7ImageSettings.height = 1200;
    fmt7ImageSettings.pixelFormat = k_fmt7PixFmt;
    bool valid;
    Format7PacketInfo fmt7PacketInfo;

    // Validate the settings to make sure that they are valid
    error = cam.ValidateFormat7Settings(
        &fmt7ImageSettings,
        &valid,
        &fmt7PacketInfo );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    if ( !valid )
    {
        // Settings are not valid
		cout << "Format7 settings are not valid" << endl;
        return -1;
    }

    // Set the settings to the camera
    error = cam.SetFormat7Configuration(
        &fmt7ImageSettings,
        fmt7PacketInfo.recommendedBytesPerPacket );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    PrintCameraInfo(&camInfo);

    // Disconnect the camera
    error = cam.Disconnect();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    return 0;
}

int saveImageThread (Image inImage, int imCount)
{
    Error error;

        // Create a converted image
        Image convertedImage;
        // Convert the raw image
        error = inImage.Convert( PIXEL_FORMAT_RGB8, &convertedImage );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }

	// set jpeg quality
	JPEGOption(jpegQ);
	jpegQ.quality=75;

        // Create a unique filename

		ostringstream filename;
		filename << "imag" << "-" << imCount << ".jpg";

        // Save the image. If a file format is not passed in, then the file
        // extension is parsed to attempt to determine the file format.
        error = convertedImage.Save( filename.str().c_str(), &jpegQ );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }

        return 0;

}

int CaptureImageSequence( PGRGuid guid )
{

    int k_numImages =0;
    string input="";

  while (true) {
    cout << "how many frames do you want to capture?" <<endl;
    getline(cin, input);

   stringstream myStream(input);
   if (myStream >> k_numImages)
     break;
   cout << "Invalid number, please try again" << endl;
 
  }

    Error error;
    Camera cam;

    // Connect to a camera
    error = cam.Connect(&guid);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

 // set configuration
       FC2Config dbConfig;
    error = cam.GetConfiguration(&dbConfig);


    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    dbConfig.numBuffers=100;
    dbConfig.highPerformanceRetrieveBuffer=true;
    dbConfig.grabMode=BUFFER_FRAMES;
  //  dbConfig.grabMode=DROP_FRAMES;

    error = cam.SetConfiguration(&dbConfig);

    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }


    // Start capturing images
    error = cam.StartCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }
    Image rawImage;
    for ( int imageCnt=0; imageCnt < k_numImages; imageCnt++ )
    {
        // Retrieve an image
        error = cam.RetrieveBuffer( &rawImage );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            continue;
        }

        cout << "Grabbed image " << imageCnt << endl;
        thread (saveImageThread,rawImage, imageCnt).detach();

    }

    // Stop capturing images
        error = cam.StopCapture();

        if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }


    // Disconnect the camera
    error = cam.Disconnect();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    PrintBuildInfo();

    Error error;

    // Since this application saves images in the current folder
    // we must ensure that we have permission to write to this folder.
    // If we do not have permission, fail right away.
	FILE* tempFile = fopen("test.txt", "w+");
	if (tempFile == NULL)
	{
		cout << "Failed to create file in current folder.  Please check permissions." << endl;
		return -1;
	}
	fclose(tempFile);
	remove("test.txt");

    BusManager busMgr;
    unsigned int numCameras;
    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    cout << "Number of cameras detected: " << numCameras << endl;

    for (unsigned int i=0; i < numCameras; i++)
    {
        PGRGuid guid;
        error = busMgr.GetCameraFromIndex(i, &guid);
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }

        InitializeCamera( guid );
        CaptureImageSequence( guid);
    }

    cout << "Done! Press Enter to exit..." << endl;
    cin.ignore();

    return 0;
}
