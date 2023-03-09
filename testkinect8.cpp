#include <windows.h>
#include <iostream> 
#include <NuiApi.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>

using namespace std;
using namespace cv;

//彩色、深度、骨骼和用户抠图结合https://blog.csdn.net/zouxy09/article/details/8163265

bool tracked[NUI_SKELETON_COUNT] = { FALSE };   // 骨骼追踪人数
// [人数][骨骼点个数]
CvPoint skeletonPoint[NUI_SKELETON_COUNT][NUI_SKELETON_POSITION_COUNT] = { cvPoint(0,0) };
CvPoint colorPoint[NUI_SKELETON_COUNT][NUI_SKELETON_POSITION_COUNT] = { cvPoint(0,0) };

void getColorImage(HANDLE& colorEvent, HANDLE& colorStreamHandle, Mat& colorImage);
void getDepthImage(HANDLE& depthEvent, HANDLE& depthStreamHandle, Mat& depthImage);
void getSkeletonImage(HANDLE& skeletonEvent, Mat& skeletonImage, Mat& colorImage, Mat& depthImage);
void drawSkeleton(Mat& image, CvPoint pointSet[], int witchone);
void getTheContour(Mat& image, int whichone, Mat& mask);

int main(int argc, char* argv[])
{
    Mat colorImage;
    colorImage.create(480, 640, CV_8UC3);
    Mat depthImage;
    depthImage.create(240, 320, CV_8UC3);
    Mat skeletonImage;
    skeletonImage.create(240, 320, CV_8UC3);
    Mat mask;
    mask.create(240, 320, CV_8UC3);

    //typedef void* HANDLE;
    //是用HANDLE来代表void*
    //void类型的指针表示可以指向任意类型的数据，但是void类型指针不能直接使用，使用前必须先转换成某种确定的类型
    //1.确定返回的句柄是否可被子进程继承.如果lpEventAttributes是NULL，此句柄不能被继承。
    //2.指定将事件对象创建成手动复原还是自动复原。如果是TRUE，那么必须用ResetEvent函数来手工将事件的状态复原到无信号状态。如果设置为FALSE，当一个等待线程被释放以后，系统将会自动将事件状态复原为无信号状态。
    //3.指定事件对象的初始状态。如果为TRUE，初始状态为有信号状态；否则为无信号状态。
    //4.指定事件的对象的名称，
       
    HANDLE colorEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    HANDLE depthEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    HANDLE skeletonEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    // 这个句柄有什么用？应该可以写一个函数在每次获取数据的时候调用？
    HANDLE colorStreamHandle = NULL;
    HANDLE depthStreamHandle = NULL;

    //NUI Common Initialization Declarations  获取颜色， 带ID的深度信息， 人体骨架、、、、、
    HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON);
    if (hr != S_OK)
    {
        cout << "NuiInitialize failed" << endl;
        return hr;
    }
    // 这里应该是控制kinect打开相关的传感器
    // 打开获取图片的数据流,彩色，分辨率，帧标志，帧限制，下一帧时间，当前数据流句柄
    hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, NULL, 4, colorEvent, &colorStreamHandle);
    if (hr != S_OK)
    {
        cout << "Open the color Stream failed" << endl;
        NuiShutdown();
        return hr;
    }
    // 打开获取深度图片的数据流
    hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, NUI_IMAGE_RESOLUTION_320x240, NULL, 2, depthEvent, &depthStreamHandle);
    if (hr != S_OK)
    {
        cout << "Open the depth Stream failed" << endl;
        NuiShutdown();
        return hr;
    }
    // 打开骨骼跟踪事件
    hr = NuiSkeletonTrackingEnable(skeletonEvent, 0);//打开骨骼跟踪事件   
    if (hr != S_OK)
    {
        cout << "NuiSkeletonTrackingEnable failed" << endl;
        NuiShutdown();
        return hr;
    }

    //Opencv函数，打开一个显示窗口
    namedWindow("mask", CV_WINDOW_AUTOSIZE);
    namedWindow("colorImage", CV_WINDOW_AUTOSIZE);
    namedWindow("depthImage", CV_WINDOW_AUTOSIZE);
    namedWindow("skeletonImage", CV_WINDOW_AUTOSIZE);


    int numSensors;
    cout << "NuiGetSensorCount(&numSensors):" << NuiGetSensorCount(&numSensors) << endl;
    cout << "numSensors:" << numSensors << endl;
   

    while (1)
    {
        // DWORD WaitForSingleObject(
       /*       [in] HANDLE hHandle,
                [in] DWORD  dwMilliseconds
            );*/
       /* [in] hHandle
            对象的句柄。 有关可以指定句柄的对象类型的列表，请参阅以下“备注”部分。
            如果等待仍在等待时关闭此句柄，则函数的行为未定义。
            句柄必须具有 SYNCHRONIZE 访问权限。 有关详细信息，请参阅 标准访问权限。
        [in] dwMilliseconds
            超时间隔（以毫秒为单位）。 如果指定了非零值，该函数将等待对象发出信号或间隔。 如果 dwMilliseconds 为零，则如果对象未发出信号，则函数不会输入等待状态; 
            它始终会立即返回。 如果 dwMilliseconds 为 INFINITE，则仅当发出对象信号时，该函数才会返回。*/

        if (WaitForSingleObject(colorEvent, 0) == 0)
            getColorImage(colorEvent, colorStreamHandle, colorImage);
        if (WaitForSingleObject(depthEvent, 0) == 0)
            getDepthImage(depthEvent, depthStreamHandle, depthImage);
        //这里使用INFINITE是为了避免没有激活skeletonEvent而跳过此代码出现colorimage频闪的现象 
        if (WaitForSingleObject(skeletonEvent, INFINITE) == 0)
            getSkeletonImage(skeletonEvent, skeletonImage, colorImage, depthImage);

        for (int i = 0; i < 6; i++)
        {
            if (tracked[i] == TRUE)
            {
                mask.setTo(0);
                getTheContour(depthImage, i, mask);
                tracked[i] = FALSE;
                break;
            }
        }

        // 显示图片
        imshow("mask", mask);
        imshow("colorImage", colorImage);
        imshow("depthImage", depthImage);
        imshow("skeletonImage", skeletonImage);

        if (cvWaitKey(1) == 27)
            break;
    }

    NuiShutdown();
    return 0;
}


void getColorImage(HANDLE& colorEvent, HANDLE& colorStreamHandle, Mat& colorImage)
{
    // colorFrame，彩色图片结构体指针
    const NUI_IMAGE_FRAME* colorFrame = NULL;

    //typedef struct _NUI_IMAGE_FRAME
    //{
    //    LARGE_INTEGER liTimeStamp;  // 时间戳
    //    DWORD dwFrameNumber;          //帧数
    //    NUI_IMAGE_TYPE eImageType;    //图像类型
    //    NUI_IMAGE_RESOLUTION eResolution; //分辨率
    //    INuiFrameTexture* pFrameTexture; //帧纹理？特征？
    //    DWORD dwFrameFlags;       //帧标志
    //    NUI_IMAGE_VIEW_AREA ViewArea;
    //} 	NUI_IMAGE_FRAME;

    // 获取下一帧图片保存在colorFrame之中
    NuiImageStreamGetNextFrame(colorStreamHandle, 0, &colorFrame);

    // 这是一个类
    INuiFrameTexture* pTexture = colorFrame->pFrameTexture;

    NUI_LOCKED_RECT LockedRect;
    pTexture->LockRect(0, &LockedRect, NULL, 0);

    if (LockedRect.Pitch != 0)
    {
        for (int i = 0; i < colorImage.rows; i++)
        {
            uchar* ptr = colorImage.ptr<uchar>(i);  //第i行的指针					
            //每个字节代表一个颜色信息，直接使用uchar
            uchar* pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
            for (int j = 0; j < colorImage.cols; j++)
            {
                ptr[3 * j] = pBuffer[4 * j];  //内部数据是4个字节，0-1-2是BGR，第4个现在未使用 
                ptr[3 * j + 1] = pBuffer[4 * j + 1];
                ptr[3 * j + 2] = pBuffer[4 * j + 2];
            }
        }
    }
    else
    {
        cout << "捕捉色彩图像出现错误" << endl;
    }

    pTexture->UnlockRect(0);
    NuiImageStreamReleaseFrame(colorStreamHandle, colorFrame);
}

void getDepthImage(HANDLE& depthEvent, HANDLE& depthStreamHandle, Mat& depthImage)
{
    const NUI_IMAGE_FRAME* depthFrame = NULL;

    NuiImageStreamGetNextFrame(depthStreamHandle, 0, &depthFrame);
    INuiFrameTexture* pTexture = depthFrame->pFrameTexture;

    NUI_LOCKED_RECT LockedRect;
    pTexture->LockRect(0, &LockedRect, NULL, 0);

    RGBQUAD q;

    if (LockedRect.Pitch != 0)
    {
        for (int i = 0; i < depthImage.rows; i++)
        {
            uchar* ptr = depthImage.ptr<uchar>(i);
            uchar* pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
            USHORT* pBufferRun = (USHORT*)pBuffer;

            for (int j = 0; j < depthImage.cols; j++)
            {
                int player = pBufferRun[j] & 7;
                int data = (pBufferRun[j] & 0xfff8) >> 3;

                uchar imageData = 255 - (uchar)(256 * data / 0x0fff);
                q.rgbBlue = q.rgbGreen = q.rgbRed = 0;

                switch (player)
                {
                case 0:
                    q.rgbRed = imageData / 2;
                    q.rgbBlue = imageData / 2;
                    q.rgbGreen = imageData / 2;
                    break;
                case 1:
                    q.rgbRed = imageData;
                    break;
                case 2:
                    q.rgbGreen = imageData;
                    break;
                case 3:
                    q.rgbRed = imageData / 4;
                    q.rgbGreen = q.rgbRed * 4;  //这里利用乘的方法，而不用原来的方法可以避免不整除的情况 
                    q.rgbBlue = q.rgbRed * 4;  //可以在后面的getTheContour()中配合使用，避免遗漏一些情况 
                    break;
                case 4:
                    q.rgbBlue = imageData / 4;
                    q.rgbRed = q.rgbBlue * 4;
                    q.rgbGreen = q.rgbBlue * 4;
                    break;
                case 5:
                    q.rgbGreen = imageData / 4;
                    q.rgbRed = q.rgbGreen * 4;
                    q.rgbBlue = q.rgbGreen * 4;
                    break;
                case 6:
                    q.rgbRed = imageData / 2;
                    q.rgbGreen = imageData / 2;
                    q.rgbBlue = q.rgbGreen * 2;
                    break;
                case 7:
                    q.rgbRed = 255 - (imageData / 2);
                    q.rgbGreen = 255 - (imageData / 2);
                    q.rgbBlue = 255 - (imageData / 2);
                }
                ptr[3 * j] = q.rgbBlue;
                ptr[3 * j + 1] = q.rgbGreen;
                ptr[3 * j + 2] = q.rgbRed;
            }
        }
    }
    else
    {
        cout << "捕捉深度图像出现错误" << endl;
    }

    pTexture->UnlockRect(0);
    NuiImageStreamReleaseFrame(depthStreamHandle, depthFrame);
}

void getSkeletonImage(HANDLE& skeletonEvent, Mat& skeletonImage, Mat& colorImage, Mat& depthImage)
{
    NUI_SKELETON_FRAME skeletonFrame = { 0 };
    bool bFoundSkeleton = false;

    if (NuiSkeletonGetNextFrame(0, &skeletonFrame) == S_OK)
    {
        for (int i = 0; i < NUI_SKELETON_COUNT; i++)
        {
            if (skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED)
            {
                bFoundSkeleton = true;
                break;
            }
        }
    }
    else
    {
        cout << "没有找到合适的骨骼帧" << endl;
        return;
    }

    if (!bFoundSkeleton)
    {
        return;
    }

    NuiTransformSmooth(&skeletonFrame, NULL);//平滑骨骼帧,消除抖动   
    skeletonImage.setTo(0);

    for (int i = 0; i < NUI_SKELETON_COUNT; i++)
    {
        if (skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED &&
            skeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER] != NUI_SKELETON_POSITION_NOT_TRACKED)
        {
            float fx, fy;

            for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)//所有的坐标转化为深度图的坐标   
            {
                NuiTransformSkeletonToDepthImage(skeletonFrame.SkeletonData[i].SkeletonPositions[j], &fx, &fy);
                skeletonPoint[i][j].x = (int)fx;
                skeletonPoint[i][j].y = (int)fy;
            }

            for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
            {
                if (skeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState[j] != NUI_SKELETON_POSITION_NOT_TRACKED)//跟踪点一用有三种状态：1没有被跟踪到，2跟踪到，3根据跟踪到的估计到   
                {
                    LONG colorx, colory;
                    NuiImageGetColorPixelCoordinatesFromDepthPixel(NUI_IMAGE_RESOLUTION_640x480, 0,
                        skeletonPoint[i][j].x, skeletonPoint[i][j].y, 0, &colorx, &colory);
                    colorPoint[i][j].x = int(colorx);
                    colorPoint[i][j].y = int(colory); //存储坐标点 
                    circle(colorImage, colorPoint[i][j], 4, cvScalar(0, 255, 255), 1, 8, 0);
                    circle(skeletonImage, skeletonPoint[i][j], 3, cvScalar(0, 255, 255), 1, 8, 0);

                    tracked[i] = TRUE;
                }
            }

            drawSkeleton(colorImage, colorPoint[i], i);
            drawSkeleton(skeletonImage, skeletonPoint[i], i);
        }
    }
}

//绘制人体骨骼图片，参数：图片，点，人ID
void drawSkeleton(Mat& image, CvPoint pointSet[], int whichone)
{
    CvScalar color;
    switch (whichone) //跟踪不同的人显示不同的颜色 
    {
    case 0:
        color = cvScalar(255);
        break;
    case 1:
        color = cvScalar(0, 255);
        break;
    case 2:
        color = cvScalar(0, 0, 255);
        break;
    case 3:
        color = cvScalar(255, 255, 0);
        break;
    case 4:
        color = cvScalar(255, 0, 255);
        break;
    case 5:
        color = cvScalar(0, 255, 255);
        break;
    }

    if ((pointSet[NUI_SKELETON_POSITION_HEAD].x != 0 || pointSet[NUI_SKELETON_POSITION_HEAD].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_HEAD], pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], color, 2);
    if ((pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_SPINE].x != 0 || pointSet[NUI_SKELETON_POSITION_SPINE].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], pointSet[NUI_SKELETON_POSITION_SPINE], color, 2);
    if ((pointSet[NUI_SKELETON_POSITION_SPINE].x != 0 || pointSet[NUI_SKELETON_POSITION_SPINE].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_HIP_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_CENTER].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_SPINE], pointSet[NUI_SKELETON_POSITION_HIP_CENTER], color, 2);

    //左上肢 
    if ((pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT], color, 2);
    if ((pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT], pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT], color, 2);
    if ((pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT], pointSet[NUI_SKELETON_POSITION_WRIST_LEFT], color, 2);
    if ((pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_HAND_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_HAND_LEFT].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_WRIST_LEFT], pointSet[NUI_SKELETON_POSITION_HAND_LEFT], color, 2);

    //右上肢 
    if ((pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT], color, 2);
    if ((pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT], pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT], color, 2);
    if ((pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT], pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT], color, 2);
    if ((pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_HAND_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_HAND_RIGHT].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT], pointSet[NUI_SKELETON_POSITION_HAND_RIGHT], color, 2);

    //左下肢 
    if ((pointSet[NUI_SKELETON_POSITION_HIP_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_CENTER].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_HIP_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_LEFT].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_HIP_CENTER], pointSet[NUI_SKELETON_POSITION_HIP_LEFT], color, 2);
    if ((pointSet[NUI_SKELETON_POSITION_HIP_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_LEFT].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_HIP_LEFT], pointSet[NUI_SKELETON_POSITION_KNEE_LEFT], color, 2);
    if ((pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_KNEE_LEFT], pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT], color, 2);
    if ((pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_FOOT_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_FOOT_LEFT].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT], pointSet[NUI_SKELETON_POSITION_FOOT_LEFT], color, 2);

    //右下肢 
    if ((pointSet[NUI_SKELETON_POSITION_HIP_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_CENTER].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_HIP_CENTER], pointSet[NUI_SKELETON_POSITION_HIP_RIGHT], color, 2);
    if ((pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_HIP_RIGHT], pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT], color, 2);
    if ((pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT], pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT], color, 2);
    if ((pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].y != 0) &&
        (pointSet[NUI_SKELETON_POSITION_FOOT_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_FOOT_RIGHT].y != 0))
        line(image, pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT], pointSet[NUI_SKELETON_POSITION_FOOT_RIGHT], color, 2);
}

//根据给定的深度数据的关系（在getDepthImage()中的）确定不同的跟踪目标 
void getTheContour(Mat& image, int whichone, Mat& mask)
{
    for (int i = 0; i < image.rows; i++)
    {
        uchar* ptr = image.ptr<uchar>(i);
        uchar* ptrmask = mask.ptr<uchar>(i);
        for (int j = 0; j < image.cols; j++)
        {
            if (ptr[3 * j] == 0 && ptr[3 * j + 1] == 0 && ptr[3 * j + 2] == 0) //都为0的时候予以忽略 
            {
                ptrmask[3 * j] = ptrmask[3 * j + 1] = ptrmask[3 * j + 2] = 0;
            }
            else if (ptr[3 * j] == 0 && ptr[3 * j + 1] == 0 && ptr[3 * j + 2] != 0)//ID为1的时候，显示绿色 
            {
                ptrmask[3 * j] = 0;
                ptrmask[3 * j + 1] = 255;
                ptrmask[3 * j + 2] = 0;
            }
            else if (ptr[3 * j] == 0 && ptr[3 * j + 1] != 0 && ptr[3 * j + 2] == 0)//ID为2的时候，显示红色 
            {
                ptrmask[3 * j] = 0;
                ptrmask[3 * j + 1] = 0;
                ptrmask[3 * j + 2] = 255;
            }
            else if (ptr[3 * j] == ptr[3 * j + 1] && ptr[3 * j] == 4 * ptr[3 * j + 2])//ID为3的时候 
            {
                ptrmask[3 * j] = 255;
                ptrmask[3 * j + 1] = 255;
                ptrmask[3 * j + 2] = 0;
            }
            else if (4 * ptr[3 * j] == ptr[3 * j + 1] && ptr[3 * j + 1] == ptr[3 * j + 2])//ID为4的时候 
            {
                ptrmask[3 * j] = 255;
                ptrmask[3 * j + 1] = 0;
                ptrmask[3 * j + 2] = 255;
            }
            else if (ptr[3 * j] == 4 * ptr[3 * j + 1] && ptr[3 * j] == ptr[3 * j + 2])//ID为5的时候 
            {
                ptrmask[3 * j] = 0;
                ptrmask[3 * j + 1] = 255;
                ptrmask[3 * j + 2] = 255;
            }
            else if (ptr[3 * j] == 2 * ptr[3 * j + 1] && ptr[3 * j + 1] == ptr[3 * j + 2])//ID为6的时候 
            {
                ptrmask[3 * j] = 255;
                ptrmask[3 * j + 1] = 255;
                ptrmask[3 * j + 2] = 255;
            }
            else if (ptr[3 * j] == ptr[3 * j + 1] && ptr[3 * j] == ptr[3 * j + 2])//ID为7的时候或者ID为0的时候，显示蓝色 
            {
                ptrmask[3 * j] = 255;
                ptrmask[3 * j + 1] = 0;
                ptrmask[3 * j + 2] = 0;
            }
            else
            {
                cout << "如果输出这段代码，说明有遗漏的情况，请查询getTheContour函数" << endl;
            }
        }
    }
}