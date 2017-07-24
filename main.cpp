#include <iostream>

#include "gdal_priv.h"
#include <opencv2/opencv.hpp>
#include<gtest/gtest.h>

using namespace cv;

cv::Mat org,sample,dst,img,tmp;

int count = 0;
float xscale = 1.0f;
float yscale = 1.0f;
char dir[128] = "/home/wuwei/Picture/Cut/";

bool CVLibraryTest()
{
    Mat img = imread("/home/wuwei/Picture/bridge_ly.jpg");
    if(img.empty())
        return false;
    else
        return true;
}
bool GDALLibraryTest()
{
    GDALAllRegister();
    GDALDatasetH m_dataset = GDALOpen("/home/wuwei/Picture/bridge_ly.jpg",GA_ReadOnly);
    if(m_dataset!=NULL)
        return true;
    else
        return false;
}

TEST(TestLibrary, ALLLibTest)
{
    EXPECT_EQ(true,CVLibraryTest());
    EXPECT_EQ(true,GDALLibraryTest());
}

/**
 * 鼠标点击世间
 * @param event
 * @param x
 * @param y
 * @param flags
 * @param ustc
 */
void on_mouse(int event,int x,int y,int flags,void *ustc)//event鼠标事件代号，x,y鼠标坐标，flags拖拽和键盘操作的代号
{
    static Point pre_pt = Point(-1,-1);//初始坐标
    static Point cur_pt = Point(-1,-1);//实时坐标
    char temp[16];
    if (event == CV_EVENT_LBUTTONDOWN)//左键按下，读取初始坐标，并在图像上该点处划圆
    {
        sample.copyTo(img);//将原始图片复制到img中
        sprintf(temp,"(%d,%d)",x,y);
        pre_pt = Point(x,y);
        putText(img,temp,pre_pt,FONT_HERSHEY_SIMPLEX,0.5,Scalar(0,0,0,255),1,8);//在窗口上显示坐标
        circle(img,pre_pt,2,Scalar(255,0,0,0),CV_FILLED,CV_AA,0);//划圆
        imshow("img",img);
    }
    else if (event == CV_EVENT_MOUSEMOVE && !(flags & CV_EVENT_FLAG_LBUTTON))//左键没有按下的情况下鼠标移动的处理函数
    {
        sample.copyTo(tmp);//将img复制到临时图像tmp上，用于显示实时坐标
        sprintf(temp,"(%d,%d)",x,y);
        cur_pt = Point(x,y);
        putText(tmp,temp,cur_pt,FONT_HERSHEY_SIMPLEX,0.5,Scalar(0,0,0,255));//只是实时显示鼠标移动的坐标
        imshow("img",tmp);
    }
    else if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))//左键按下时，鼠标移动，则在图像上划矩形
    {
        sample.copyTo(tmp);
        sprintf(temp,"(%d,%d)",x,y);
        cur_pt = Point(x,y);
        putText(tmp,temp,cur_pt,FONT_HERSHEY_SIMPLEX,0.5,Scalar(0,0,0,255));
        rectangle(tmp,pre_pt,cur_pt,Scalar(0,255,0,0),1,8,0);//在临时图像上实时显示鼠标拖动时形成的矩形
        imshow("img",tmp);
    }
    else if (event == CV_EVENT_LBUTTONUP)//左键松开，将在图像上划矩形
    {
        sample.copyTo(img);
        sprintf(temp,"(%d,%d)",x,y);
        cur_pt = Point(x,y);
        putText(img,temp,cur_pt,FONT_HERSHEY_SIMPLEX,0.5,Scalar(0,0,0,255));
        circle(img,pre_pt,2,Scalar(255,0,0,0),CV_FILLED,CV_AA,0);
        rectangle(img,pre_pt,cur_pt,Scalar(0,255,0,0),1,8,0);//根据初始点和结束点，将矩形画到img上
        imshow("img",img);
        sample.copyTo(tmp);
        //截取矩形包围的图像，并保存到dst中
        int width = abs(pre_pt.x - cur_pt.x)/xscale;
        int height = abs(pre_pt.y - cur_pt.y)/yscale;
        if (width == 0 || height == 0)
        {
            printf("width == 0 || height == 0");
            return;
        }
        dst = org(Rect(min(cur_pt.x,pre_pt.x)/xscale,min(cur_pt.y,pre_pt.y)/yscale,width,height));
        namedWindow("dst");
        imshow("dst",dst);
    }
    if (event == CV_EVENT_RBUTTONDOWN)
    {
        if(dst.empty())
            return ;
        else
        {  //按下回车键保存
            char temp[20];
            char tempdesc[20]="descriptor.desc";

            char path[256];
            char pathdesc[256];

            strcpy(path,dir);
            strcpy(pathdesc,dir);

            count++;
            sprintf(temp,"%d.jpg",count);

            strcat(path,temp);
            strcat(pathdesc,tempdesc);

            if(!imwrite(path,dst)){
                printf("影像写入文件 %s 失败！\n",path);
                count--;
            }
            else
            {
                printf("影像写入文件 %s 成功！\n",path);
                FILE *fptr=fopen(pathdesc,"a+");
                fprintf(fptr,"%s\n",path);
                fclose(fptr);
            }

            cv::destroyWindow("dst");
            dst.release();
        }
    }

}

/*
 * 文件夹中文件编目情况获取
 */
bool file_catalog(char* file)
{
    FILE *fptr=fopen(file,"r+");
    if(fptr==NULL)
        return false;
    char line[256];
    int num=0;
    do{
        fgets(line,256,fptr);
        if(line!="")
            num++;
    }while(!feof(fptr));
    count = num;
    return true;
}

/*
 * 将影像文件转换为二进制码
 */
bool tranImageToBinnary(char* fcatalog, char* fbinnary)
{
    FILE* fcptr=fopen(fcatalog,"r+");
    if(fcptr==NULL)
        return false;
    if(fbinnary==NULL)
        return false;

    FILE* fbptr=fopen(fbinnary,"wb+");
    GDALAllRegister();
    do{
        char img[256];
        fgets(img,256,fcptr);
        if(img=="")
            continue;
        printf("转换影像:%s",img);
        int len = strlen(img);
        char* p = new char[len];
        for(int i=0;i<len-1;++i)
            p[i]=img[i];
        p[len-1]='\0';
        GDALDatasetH m_dataset = GDALOpen(p,GA_ReadOnly);
        int xsize = GDALGetRasterXSize(m_dataset);
        int ysize = GDALGetRasterYSize(m_dataset);
        int bands = GDALGetRasterCount(m_dataset);

        int cxsize = 64;
        int cyszie = 64;
        int *img_data = new int[cxsize*cyszie];

        int bandIdx = 1;

        //三个波段
        /*
        if(bands>=3)
        {
            for (int i = 0; i < 3; ++i) {
                GDALRasterIO(GDALGetRasterBand(m_dataset,bandIdx),GF_Read,0,0,xsize,ysize,img_data,cxsize,cyszie,GDT_Int32,0,0);
                fwrite(img_data,sizeof(int),cxsize*cyszie,fbptr);
                bandIdx++;
            }
        }
        else if(bands==2){
            for (int i = 0; i < 3; ++i) {
                GDALRasterIO(GDALGetRasterBand(m_dataset,bandIdx),GF_Read,0,0,xsize,ysize,img_data,cxsize,cyszie,GDT_Int32,0,0);
                fwrite(img_data,sizeof(int),cxsize*cyszie,fbptr);
                bandIdx++;
                if(bandIdx>bands)
                    bandIdx=bandIdx%bands;
            }
        } else{
            for (int i = 0; i < 3; ++i) {
                GDALRasterIO(GDALGetRasterBand(m_dataset,bandIdx),GF_Read,0,0,xsize,ysize,img_data,cxsize,cyszie,GDT_Int32,0,0);
                fwrite(img_data,sizeof(int),cxsize*cyszie,fbptr);
            }
        }
         */

        //一个波段
        GDALRasterIO(GDALGetRasterBand(m_dataset,bandIdx),GF_Read,0,0,xsize,ysize,img_data,cxsize,cyszie,GDT_Int32,0,0);
        fwrite(img_data,sizeof(int),cxsize*cyszie,fbptr);

        delete []img_data;img_data=NULL;
        delete []p;p=NULL;
    }while(!feof(fcptr));
    return true;
}

int main()
{
    //首先读取文件夹中记录的文件信息，确定编号
    char* fcatalog = "/home/wuwei/Picture/Cut/descriptor.desc";
    char* imgpath  = "/home/wuwei/Picture/bridge_ly.jpg";
    char* bin  = "/home/wuwei/Picture/data.bin";

    /*file_catalog(fcatalog);
    org = imread(imgpath);
    if(org.empty())
    {
        return -1;
    }

    //判断是否需要重采样
    int xsize = 1080;
    int ysize = 768;
    if(org.cols>1080)
        xscale = float(xsize)/float(org.cols);
    if(org.rows>768)
        yscale = float(ysize)/float(org.rows);

    //重采样
    resize(org,sample,Size(0,0),xscale,yscale);
    sample.copyTo(img);
    sample.copyTo(tmp);
    namedWindow("img");//定义一个img窗口
    setMouseCallback("img",on_mouse,0);//调用回调函数
    imshow("img",sample);
    cv::waitKey(0);*/

    //转换
    printf("影像重采样并转换为二进制文件...\n");
    tranImageToBinnary(fcatalog,bin);
    printf("转换完成!\n");
    return 0;
}