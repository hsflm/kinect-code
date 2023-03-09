//#include <windows.h>
//#include <iostream> 
//#include <NuiApi.h>
//#include <NuiSensor.h>
//#include <opencv2/opencv.hpp>
//#include <opencv2/highgui/highgui_c.h>
//using namespace std;
//using namespace cv;
//void getColorImage(HANDLE& colorEvent, HANDLE& colorStreamHandle, Mat& colorImage);
//
////提取颜色数据并用OpenCV显示
//// 
////https://blog.csdn.net/zouxy09/article/details/8146266
//
////改进后：可以同时驱动两台kinect，获取颜色数据并用Opencv显示
//
//int main(int argc, char* argv[])
//{
//    Mat image1;
//    Mat image2;
//
//    image1.create(480, 640, CV_8UC3);
//    image2.create(480, 640, CV_8UC3);
//
//    INuiSensor* sensor1;            // The kinect sensor
//    INuiSensor* sensor2;
//
//    int sensorIndex1 = 0;
//    int sensorIndex2 = 1;
//
//
//    // Get a working kinect sensor
//    int numSensors;
//    if (NuiGetSensorCount(&numSensors) < 0 || numSensors < 1) cout << "no device" << endl;
//    cout << "numSensor:" << numSensors << endl;
//
//    if (NuiCreateSensorByIndex(sensorIndex1, &sensor1) < 0) cout << "Initialize failed" << endl;
//    if (NuiCreateSensorByIndex(sensorIndex2, &sensor2) < 0) cout << "Initialize failed" << endl;
//
//
//
//    //1、初始化NUI 
//    HRESULT hr = sensor1-> NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR);
//    if (FAILED(hr))
//    {
//        cout << "NuiInitialize failed" << endl;
//        return hr;
//    }
//    hr = sensor2->NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR);
//    if (FAILED(hr))
//    {
//        cout << "NuiInitialize failed" << endl;
//        return hr;
//    }
//
//    //2、定义事件句柄 
//    //创建读取下一帧的信号事件句柄，控制KINECT是否可以开始读取下一帧数据
//    HANDLE nextColorFrameEvent1 = CreateEvent(NULL, TRUE, FALSE, NULL);
//    HANDLE colorStreamHandle1 = NULL; //保存图像数据流的句柄，用以提取数据 
//
//    HANDLE nextColorFrameEvent2 = CreateEvent(NULL, TRUE, FALSE, NULL);
//    HANDLE colorStreamHandle2 = NULL; //保存图像数据流的句柄，用以提取数据 
//
//    //3、打开KINECT设备的彩色图信息通道，并用colorStreamHandle保存该流的句柄，以便于以后读取
//    hr = sensor1->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480,
//        0, 2, nextColorFrameEvent1, &colorStreamHandle1);
//    if (FAILED(hr))//判断是否提取正确 
//    {
//        cout << "Could not open color image stream video" << endl;
//        NuiShutdown();
//        return hr;
//    }
//  
//    hr = sensor2->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480,
//        0, 2, nextColorFrameEvent2, &colorStreamHandle2);
//    if (FAILED(hr))//判断是否提取正确 
//    {
//        cout << "Could not open color image stream video" << endl;
//        NuiShutdown();
//        return hr;
//    }
//    
//    namedWindow("colorImage1", CV_WINDOW_AUTOSIZE);
//    namedWindow("colorImage2", CV_WINDOW_AUTOSIZE);
//
//    //4、开始读取彩色图数据 
//    while (1)
//    {
//        const NUI_IMAGE_FRAME* pImageFrame = NULL;
//
//        //4.1、无限等待新的数据，等到后返回
//        if (WaitForSingleObject(nextColorFrameEvent1, INFINITE) == 0)
//            getColorImage(nextColorFrameEvent1, colorStreamHandle1, image1);
//        if (WaitForSingleObject(nextColorFrameEvent2, INFINITE) == 0)
//            getColorImage(nextColorFrameEvent2, colorStreamHandle2, image2);
//
//        imshow("colorImage1", image1);
//        imshow("colorImage2", image2);
//
//   
//        if (cvWaitKey(20) == 27)
//            break;
//
//    }
//    //7、关闭NUI链接 
//    NuiShutdown();
//    return 0;
//}
//
//
//void getColorImage(HANDLE& colorEvent, HANDLE& colorStreamHandle, Mat& colorImage)
//{
//    // colorFrame，彩色图片结构体指针
//    const NUI_IMAGE_FRAME* colorFrame = NULL;
//
//    // 获取下一帧图片保存在colorFrame之中
//    NuiImageStreamGetNextFrame(colorStreamHandle, 0, &colorFrame);
//
//    // 这是一个类
//    INuiFrameTexture* pTexture = colorFrame->pFrameTexture;
//
//    NUI_LOCKED_RECT LockedRect;
//    pTexture->LockRect(0, &LockedRect, NULL, 0);
//
//    if (LockedRect.Pitch != 0)
//    {
//        for (int i = 0; i < colorImage.rows; i++)
//        {
//            uchar* ptr = colorImage.ptr<uchar>(i);  //第i行的指针					
//            //每个字节代表一个颜色信息，直接使用uchar
//            uchar* pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
//            for (int j = 0; j < colorImage.cols; j++)
//            {
//                ptr[3 * j] = pBuffer[4 * j];  //内部数据是4个字节，0-1-2是BGR，第4个现在未使用 
//                ptr[3 * j + 1] = pBuffer[4 * j + 1];
//                ptr[3 * j + 2] = pBuffer[4 * j + 2];
//            }
//        }
//    }
//    else
//    {
//        cout << "捕捉色彩图像出现错误" << endl;
//    }
//
//    pTexture->UnlockRect(0);
//    NuiImageStreamReleaseFrame(colorStreamHandle, colorFrame);
//}