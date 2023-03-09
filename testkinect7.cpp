//#include <windows.h>
//#include <iostream> 
//#include <NuiApi.h>
//#include<NuiSensor.h>
//#include <opencv2/opencv.hpp>
//#include <opencv2/highgui/highgui_c.h>
//
//using namespace std;
//using namespace cv;
//
////通过传入关节点的位置，把骨骼画出来
////https://blog.csdn.net/zouxy09/article/details/8161617
//
////void getSkeletonImage(HANDLE& skeletonEvent, Mat& skeletonImage, Mat& colorImage, Mat& depthImage);
//void drawSkeleton(Mat& image, CvPoint pointSet[], int whichone);
//
//
//int main(int argc, char* argv[])
//{
//    Mat skeletonImage1;
//    skeletonImage1.create(240, 320, CV_8UC3);
//    CvPoint skeletonPoint1[NUI_SKELETON_COUNT][NUI_SKELETON_POSITION_COUNT] = { cvPoint(0,0) };
//    bool tracked1[NUI_SKELETON_COUNT] = { FALSE };
//
//    INuiSensor* sensor1;            // The kinect sensor
//    int sensorIndex1 = 0;
//
//    // Get a working kinect sensor
//    int numSensors;
//    if (NuiGetSensorCount(&numSensors) < 0 || numSensors < 1) cout << "no device" << endl;
//    cout << "numSensor:" << numSensors << endl;
//
//    if (NuiCreateSensorByIndex(sensorIndex1, &sensor1) < 0) cout << "Initialize failed" << endl;
//
//    //1、初始化NUI，注意这里是USES_SKELETON
//    HRESULT hr = sensor1->NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON);
//    if (FAILED(hr))
//    {
//        cout << "NuiInitialize failed" << endl;
//        return hr;
//    }
//
//    //2、定义骨骼信号事件句柄 
//    HANDLE skeletonEvent1 = CreateEvent(NULL, TRUE, FALSE, NULL);
//
//    //3、打开骨骼跟踪事件
//    hr = sensor1->NuiSkeletonTrackingEnable(skeletonEvent1, 0);
//    if (FAILED(hr))
//    {
//        cout << "Could not open color image stream video" << endl;
//        NuiShutdown();
//        return hr;
//    }
//
//    namedWindow("skeletonImage1", CV_WINDOW_AUTOSIZE);
//
//    //第二个sensoer的配置*************************************************
//    Mat skeletonImage2; 
//    skeletonImage2.create(240, 320, CV_8UC3);
//
//    CvPoint skeletonPoint2[NUI_SKELETON_COUNT][NUI_SKELETON_POSITION_COUNT] = { cvPoint(0,0) };
//    bool tracked2[NUI_SKELETON_COUNT] = { FALSE };
//
//    INuiSensor* sensor2;
//    int sensorIndex2 = 1;
//  
//    if (NuiCreateSensorByIndex(sensorIndex2, &sensor2) < 0) cout << "Initialize failed" << endl;
//
//    //1、初始化NUI，注意这里是USES_SKELETON
//     hr = sensor2->NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON);
//    if (FAILED(hr))
//    {
//        cout << "NuiInitialize failed" << endl;
//        return hr;
//    }
//
//    //2、定义骨骼信号事件句柄 
//    HANDLE skeletonEvent2 = CreateEvent(NULL, TRUE, FALSE, NULL);
//
//    //3、打开骨骼跟踪事件
//    hr = sensor2->NuiSkeletonTrackingEnable(skeletonEvent2, 0);
//    if (FAILED(hr))
//    {
//        cout << "Could not open color image stream video" << endl;
//        NuiShutdown();
//        return hr;
//    }
// 
//    namedWindow("skeletonImage2", CV_WINDOW_AUTOSIZE);
//
//    //4、开始读取骨骼跟踪数据 
//    while (1)
//    {
//        NUI_SKELETON_FRAME skeletonFrame1 = { 0 };  //骨骼帧的定义 
//        bool bFoundSkeleton1 = false;
//        NUI_SKELETON_FRAME skeletonFrame2 = { 0 };  //骨骼帧的定义 
//        bool bFoundSkeleton2 = false;
//
//      /*  4.1、无限等待新的数据，等到后返回*/
//      
//        if (WaitForSingleObject(skeletonEvent1, INFINITE) == 0)
//        {
//            
//            //4.2、从刚才打开数据流的流句柄中得到该帧数据，读取到的数据地址存于skeletonFrame
//            hr = NuiSkeletonGetNextFrame(0, &skeletonFrame1);
//            if (SUCCEEDED(hr))
//            {
//                //NUI_SKELETON_COUNT是检测到的骨骼数（即，跟踪到的人数）
//                for (int i = 0; i < NUI_SKELETON_COUNT; i++)
//                {
//                    NUI_SKELETON_TRACKING_STATE trackingState = skeletonFrame1.SkeletonData[i].eTrackingState;
//                    //4.3、Kinect最多检测六个人，但只能跟踪两个人的骨骼，再检查每个“人”（有可能是空，不是人）
//                    //是否跟踪到了 
//                    if (trackingState == NUI_SKELETON_TRACKED)
//                    {
//                        bFoundSkeleton1 = true;
//                    }
//                }
//            }
//            if (!bFoundSkeleton1)
//            {
//                continue;
//            }
//            //4.4、平滑骨骼帧，消除抖动
//            NuiTransformSmooth(&skeletonFrame1, NULL);
//            skeletonImage1.setTo(0);
//            for (int i = 0; i < NUI_SKELETON_COUNT; i++)
//            {
//                // Show skeleton only if it is tracked, and the center-shoulder joint is at least inferred. 
//                //断定是否是一个正确骨骼的条件：骨骼被跟踪到并且肩部中心（颈部位置）必须跟踪到。 
//                if (skeletonFrame1.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED &&
//                    skeletonFrame1.SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER] != NUI_SKELETON_POSITION_NOT_TRACKED)
//                {
//                    float fx, fy;
//                    //拿到所有跟踪到的关节点的坐标，并转换为我们的深度空间的坐标，因为我们是在深度图像中
//                    //把这些关节点标记出来的
//                    //NUI_SKELETON_POSITION_COUNT为跟踪到的一个骨骼的关节点的数目，为20
//                    for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
//                    {
//                        NuiTransformSkeletonToDepthImage(skeletonFrame1.SkeletonData[i].SkeletonPositions[j], &fx, &fy);
//                        skeletonPoint1[i][j].x = (int)fx;
//                        skeletonPoint1[i][j].y = (int)fy;
//                    }
//                    for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
//                    {
//                        if (skeletonFrame1.SkeletonData[i].eSkeletonPositionTrackingState[j] != NUI_SKELETON_POSITION_NOT_TRACKED)//跟踪点一用有三种状态：1没有被跟踪到，2跟踪到，3根据跟踪到的估计到   
//                        {
//                            circle(skeletonImage1, skeletonPoint1[i][j], 3, cvScalar(0, 255, 255), 1, 8, 0);
//                            tracked1[i] = TRUE;
//                        }
//                    }
//                    drawSkeleton(skeletonImage1, skeletonPoint1[i], i);
//                }
//            }
//            imshow("skeletonImage1", skeletonImage1); //显示图像 
//        }
//        else
//        {
//            cout << "Buffer length of received texture is bogus\r\n" << endl;
//        }
//
//
//
//        
//        if (WaitForSingleObject(skeletonEvent2, INFINITE) == 0)
//        {
//            
//            //4.2、从刚才打开数据流的流句柄中得到该帧数据，读取到的数据地址存于skeletonFrame
//            hr = NuiSkeletonGetNextFrame(0, &skeletonFrame2);
//            if (SUCCEEDED(hr))
//            {
//                //NUI_SKELETON_COUNT是检测到的骨骼数（即，跟踪到的人数）
//                for (int i = 0; i < NUI_SKELETON_COUNT; i++)
//                {
//                    NUI_SKELETON_TRACKING_STATE trackingState = skeletonFrame2.SkeletonData[i].eTrackingState;
//                    //4.3、Kinect最多检测六个人，但只能跟踪两个人的骨骼，再检查每个“人”（有可能是空，不是人）
//                    //是否跟踪到了 
//                    if (trackingState == NUI_SKELETON_TRACKED)
//                    {
//                        bFoundSkeleton2 = true;
//                    }
//                }
//            }
//            if (!bFoundSkeleton2)
//            {
//                continue;
//            }
//            //4.4、平滑骨骼帧，消除抖动
//            NuiTransformSmooth(&skeletonFrame2, NULL);
//            skeletonImage2.setTo(0);
//            for (int i = 0; i < NUI_SKELETON_COUNT; i++)
//            {
//                // Show skeleton only if it is tracked, and the center-shoulder joint is at least inferred. 
//                //断定是否是一个正确骨骼的条件：骨骼被跟踪到并且肩部中心（颈部位置）必须跟踪到。 
//                if (skeletonFrame2.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED &&
//                    skeletonFrame2.SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER] != NUI_SKELETON_POSITION_NOT_TRACKED)
//                {
//                    float fx, fy;
//                    //拿到所有跟踪到的关节点的坐标，并转换为我们的深度空间的坐标，因为我们是在深度图像中
//                    //把这些关节点标记出来的
//                    //NUI_SKELETON_POSITION_COUNT为跟踪到的一个骨骼的关节点的数目，为20
//                    for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
//                    {
//                        NuiTransformSkeletonToDepthImage(skeletonFrame2.SkeletonData[i].SkeletonPositions[j], &fx, &fy);
//                        skeletonPoint2[i][j].x = (int)fx;
//                        skeletonPoint2[i][j].y = (int)fy;
//                    }
//                    for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
//                    {
//                        if (skeletonFrame2.SkeletonData[i].eSkeletonPositionTrackingState[j] != NUI_SKELETON_POSITION_NOT_TRACKED)//跟踪点一用有三种状态：1没有被跟踪到，2跟踪到，3根据跟踪到的估计到   
//                        {
//                            circle(skeletonImage2, skeletonPoint2[i][j], 3, cvScalar(0, 255, 255), 1, 8, 0);
//                            tracked2[i] = TRUE;
//                        }
//                    }
//                    drawSkeleton(skeletonImage2, skeletonPoint2[i], i);
//                }
//            }
//            imshow("skeletonImage2", skeletonImage2); //显示图像 
//        }
//        else
//        {
//            cout << "Buffer length of received texture is bogus\r\n" << endl;
//        }
//
//        if (cvWaitKey(20) == 27)
//            break;
//    }
//    //5、关闭NUI链接 
//    NuiShutdown();
//    return 0;
//}
//
////void getSkeletonImage(HANDLE& skeletonEvent, Mat& skeletonImage, Mat& colorImage, Mat& depthImage)
////{
////    NUI_SKELETON_FRAME skeletonFrame = { 0 };
////    bool bFoundSkeleton = false;
////
////    if (NuiSkeletonGetNextFrame(0, &skeletonFrame) == S_OK)
////    {
////        for (int i = 0; i < NUI_SKELETON_COUNT; i++)
////        {
////            if (skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED)
////            {
////                bFoundSkeleton = true;
////                break;
////            }
////        }
////    }
////    else
////    {
////        cout << "没有找到合适的骨骼帧" << endl;
////        return;
////    }
////
////    if (!bFoundSkeleton)
////    {
////        return;
////    }
////
////    NuiTransformSmooth(&skeletonFrame, NULL);//平滑骨骼帧,消除抖动   
////    skeletonImage.setTo(0);
////
////    for (int i = 0; i < NUI_SKELETON_COUNT; i++)
////    {
////        if (skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED &&
////            skeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER] != NUI_SKELETON_POSITION_NOT_TRACKED)
////        {
////            float fx, fy;
////
////            for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)//所有的坐标转化为深度图的坐标   
////            {
////                NuiTransformSkeletonToDepthImage(skeletonFrame.SkeletonData[i].SkeletonPositions[j], &fx, &fy);
////                skeletonPoint[i][j].x = (int)fx;
////                skeletonPoint[i][j].y = (int)fy;
////            }
////
////            for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
////            {
////                if (skeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState[j] != NUI_SKELETON_POSITION_NOT_TRACKED)//跟踪点一用有三种状态：1没有被跟踪到，2跟踪到，3根据跟踪到的估计到   
////                {
////                    LONG colorx, colory;
////                    NuiImageGetColorPixelCoordinatesFromDepthPixel(NUI_IMAGE_RESOLUTION_640x480, 0,
////                        skeletonPoint[i][j].x, skeletonPoint[i][j].y, 0, &colorx, &colory);
////                    colorPoint[i][j].x = int(colorx);
////                    colorPoint[i][j].y = int(colory); //存储坐标点 
////                    circle(colorImage, colorPoint[i][j], 4, cvScalar(0, 255, 255), 1, 8, 0);
////                    circle(skeletonImage, skeletonPoint[i][j], 3, cvScalar(0, 255, 255), 1, 8, 0);
////
////                    tracked[i] = TRUE;
////                }
////            }
////
////            drawSkeleton(colorImage, colorPoint[i], i);
////            drawSkeleton(skeletonImage, skeletonPoint[i], i);
////        }
////    }
////}
//
//
//
//// 通过传入关节点的位置，把骨骼画出来
//void drawSkeleton(Mat& image, CvPoint pointSet[], int whichone)
//{
//    CvScalar color;
//    switch (whichone) //跟踪不同的人显示不同的颜色 
//    {
//    case 0:
//        color = cvScalar(255);
//        break;
//    case 1:
//        color = cvScalar(0, 255);
//        break;
//    case 2:
//        color = cvScalar(0, 0, 255);
//        break;
//    case 3:
//        color = cvScalar(255, 255, 0);
//        break;
//    case 4:
//        color = cvScalar(255, 0, 255);
//        break;
//    case 5:
//        color = cvScalar(0, 255, 255);
//        break;
//    }
//
//    if ((pointSet[NUI_SKELETON_POSITION_HEAD].x != 0 || pointSet[NUI_SKELETON_POSITION_HEAD].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_HEAD], pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], color, 2);
//    if ((pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_SPINE].x != 0 || pointSet[NUI_SKELETON_POSITION_SPINE].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], pointSet[NUI_SKELETON_POSITION_SPINE], color, 2);
//    if ((pointSet[NUI_SKELETON_POSITION_SPINE].x != 0 || pointSet[NUI_SKELETON_POSITION_SPINE].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_HIP_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_CENTER].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_SPINE], pointSet[NUI_SKELETON_POSITION_HIP_CENTER], color, 2);
//
//    //左上肢 
//    if ((pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT], color, 2);
//    if ((pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT], pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT], color, 2);
//    if ((pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT], pointSet[NUI_SKELETON_POSITION_WRIST_LEFT], color, 2);
//    if ((pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_HAND_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_HAND_LEFT].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_WRIST_LEFT], pointSet[NUI_SKELETON_POSITION_HAND_LEFT], color, 2);
//
//    //右上肢 
//    if ((pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT], color, 2);
//    if ((pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT], pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT], color, 2);
//    if ((pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT], pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT], color, 2);
//    if ((pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_HAND_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_HAND_RIGHT].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT], pointSet[NUI_SKELETON_POSITION_HAND_RIGHT], color, 2);
//
//    //左下肢 
//    if ((pointSet[NUI_SKELETON_POSITION_HIP_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_CENTER].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_HIP_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_LEFT].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_HIP_CENTER], pointSet[NUI_SKELETON_POSITION_HIP_LEFT], color, 2);
//    if ((pointSet[NUI_SKELETON_POSITION_HIP_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_LEFT].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_HIP_LEFT], pointSet[NUI_SKELETON_POSITION_KNEE_LEFT], color, 2);
//    if ((pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_KNEE_LEFT], pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT], color, 2);
//    if ((pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_FOOT_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_FOOT_LEFT].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT], pointSet[NUI_SKELETON_POSITION_FOOT_LEFT], color, 2);
//
//    //右下肢 
//    if ((pointSet[NUI_SKELETON_POSITION_HIP_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_CENTER].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_HIP_CENTER], pointSet[NUI_SKELETON_POSITION_HIP_RIGHT], color, 2);
//    if ((pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_HIP_RIGHT], pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT], color, 2);
//    if ((pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT], pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT], color, 2);
//    if ((pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].y != 0) &&
//        (pointSet[NUI_SKELETON_POSITION_FOOT_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_FOOT_RIGHT].y != 0))
//        line(image, pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT], pointSet[NUI_SKELETON_POSITION_FOOT_RIGHT], color, 2);
//}