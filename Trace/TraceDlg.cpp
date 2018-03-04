
// TraceDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Trace.h"
#include "TraceDlg.h"
#include "afxdialogex.h"
#include "WzdSplash.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
IplImage* src;
	CvHaarClassifierCascade* cascade;
	CvMemStorage *storage;

	const char* cascade_name =
    "haarcascade_frontalface_alt.xml";
	
void detect_and_draw(IplImage*img)
{

	static CvScalar colors[] = 
    {
        {{0,0,255}},
        {{0,128,255}},
        {{0,255,255}},
        {{0,255,0}},
        {{255,128,0}},
        {{255,255,0}},
        {{255,0,0}},
        {{255,0,255}}
    };

    double scale = 1.3;
    IplImage* gray = cvCreateImage( cvSize(img->width,img->height), 8, 1 );
    IplImage* small_img = cvCreateImage( cvSize( cvRound (img->width/scale),
                         cvRound (img->height/scale)),
                     8, 1 );
	//cvRead()
    int i;

    cvCvtColor( img, gray, CV_BGR2GRAY );
    cvResize( gray, small_img, CV_INTER_LINEAR );
    cvEqualizeHist( small_img, small_img );
    cvClearMemStorage( storage );

   if( cascade )
   {
		//检测人脸
        CvSeq* faces = cvHaarDetectObjects( small_img, cascade, storage,
                                            1.1, 2, 0/*CV_HAAR_DO_CANNY_PRUNING*/,
                                            cvSize(30, 30) );  
 
        for( i = 0; i < (faces ? faces->total : 0); i++ )
        {
            CvRect* r = (CvRect*)cvGetSeqElem( faces, i );
            CvPoint center;
            int radius;
            center.x = cvRound((r->x + r->width*0.5)*scale);
            center.y = cvRound((r->y + r->height*0.5)*scale);
			
            radius = cvRound((r->width + r->height)*0.25*scale);
            cvCircle( img, center, radius, colors[i%8], 3, 8, 0 );
        }
   }
    IplImage* image = cvLoadImage("./1.jpg", -1);
	CvSize size;

	size.width = image->width*0.5;
	size.height = image->height*0.5;
		//创建图片并缩放
	IplImage*detImg = cvCreateImage(size, image->depth, image->nChannels);
		// ·CV_INTER_NN - 最近-邻居插补
		// ·CV_INTER_LINEAR - 双线性插值（默认方法）
		// ·CV_INTER_AREA - 像素面积相关重采样。当缩小图像时，该方法可以避免波纹的出现。当放大图像时，类似于方法CV_INTER_NN。
		// ·CV_INTER_CUBIC - 双三次插值。)
		cvResize(image, detImg, CV_INTER_AREA); 
    cvShowImage( "人脸检测", detImg);
    cvReleaseImage( &gray );
    cvReleaseImage( &small_img );

}




















typedef unsigned char pixel;
#define HISTOGRAM_LENGTH   4096
int imgWidth;
int imgHeight;
int trackWinWidth=20;
int trackWinHeight=20;
int currentX=0;
int currentY=0;
float currHistogram[HISTOGRAM_LENGTH];
float tempHistogram[HISTOGRAM_LENGTH];

void DrawTrackBox(IplImage*frame)

{
	for(int i=currentX;i<min(imgWidth,currentX+trackWinWidth);i++)
	{

	  ((uchar*)(frame->imageData+currentY*frame->widthStep))[i*frame->nChannels+0]=0;
	  ((uchar*)(frame->imageData+currentY*frame->widthStep))[i*frame->nChannels+1]=0;
	  ((uchar*)(frame->imageData+currentY*frame->widthStep))[i*frame->nChannels+2]=255;

	  ((uchar*)(frame->imageData+min(imgHeight-1,currentY+trackWinHeight)*frame->widthStep))[i*frame->nChannels+0]=0;
	  ((uchar*)(frame->imageData+min(imgHeight-1,currentY+trackWinHeight)*frame->widthStep))[i*frame->nChannels+1]=0;
	  ((uchar*)(frame->imageData+min(imgHeight-1,currentY+trackWinHeight)*frame->widthStep))[i*frame->nChannels+2]=255;
	 
	}

	for(int j=currentY;j<min(imgHeight-1,currentY+trackWinHeight);j++)
	{
	((uchar*)(frame->imageData+j*frame->widthStep))[currentX*frame->nChannels+0]=0;
	((uchar*)(frame->imageData+j*frame->widthStep))[currentX*frame->nChannels+1]=0;
	((uchar*)(frame->imageData+j*frame->widthStep))[currentX*frame->nChannels+2]=255;

	((uchar*)(frame->imageData+j*frame->widthStep))[min(imgWidth-1,currentX+trackWinWidth)*frame->nChannels+0]=0;
	 ((uchar*)(frame->imageData+j*frame->widthStep))[min(imgWidth-1,currentX+trackWinWidth)*frame->nChannels+1]=0;
	 ((uchar*)(frame->imageData+j*frame->widthStep))[min(imgWidth-1,currentX+trackWinWidth)*frame->nChannels+2]=255;
	}


}































IplImage*image=0,*hsv=0,*hue=0,*mask=0,*backproject=0,*histimg=0; 
CvHistogram *hist;

int backproject_mode=0;
int select_object=0;
int track_object=0;
int show_hist=1;
CvPoint origin;
CvRect selection;
CvRect track_window;
CvBox2D track_box;
CvConnectedComp track_comp;
int hdims=48;
float hranges_arr[]={0,180};
float *hranges=hranges_arr;
int vmin=10,vmax=255,smin=30;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

void on_mouse(int event,int x,int y,int flags,void *param)
{
    if(!image)
		return;
	if(image->origin)
		y=image->height-y;
	if(select_object)
	{                                  

	   selection.x=MIN(x,origin.x);
	   selection.y=MIN(y,origin.y);
	   selection.width=selection.x+CV_IABS(x-origin.x);
	   selection.height=selection.y+CV_IABS(y-origin.y);

	   selection.x=MAX(selection.x,0);
	   selection.y=MAX(selection.y,0);
	   selection.width=MIN(selection.width,image->width);
	   selection.height=MIN(selection.height,image->height);
	   selection.width-=selection.x;
	   selection.height-=selection.y;


	
	}
	switch(event)
	{
	
	
	case CV_EVENT_LBUTTONDOWN:
		origin=cvPoint(x,y);
		selection=cvRect(x,y,0,0);
		select_object=1;
		break; 
	case CV_EVENT_LBUTTONUP:
		select_object=0;
		if(selection.width>0&&selection.height>0)
			track_object=-1;
		//printf("rtgryhtty");
#ifdef _DEBUG
		printf("\n#鼠标的选择区域:");
#endif
		break;



	
	
	}




}
CvScalar hsv2rgb(float hue)
{
   int  rgb[3],p,sector;
   static const int sector_data[][3]={{0,2,1},{1,2,0},{1,0,2},{2,0,1},{2,1,0},{0,1,2}};
   hue*=0.033333333333333333333333333333333f;
   sector=cvFloor(hue);
   p=cvRound(255*(hue-sector));
   p^=sector&1?255:0;
   rgb[sector_data[sector][0]]=255;
   rgb[sector_data[sector][1]]=0;
   rgb[sector_data[sector][2]]=p;



   #ifdef _DEBUG
		printf("\n#Convert HSV TO RGB");
        printf("\n  HUE=%f",hue);
        printf("\n R=%d,G=%d,B=%d",rgb[0],rgb[1],rgb[2]);
#endif
		return cvScalar(rgb[2],rgb[1],rgb[0],0);

}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTraceDlg 对话框




CTraceDlg::CTraceDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTraceDlg::IDD, pParent)
	, m_x(0)
	, m_y(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTraceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_x);
	DDX_Text(pDX, IDC_EDIT2, m_y);
}

BEGIN_MESSAGE_MAP(CTraceDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CTraceDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CTraceDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CTraceDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CTraceDlg::OnBnClickedButton2)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_EDIT2, &CTraceDlg::OnEnChangeEdit2)
	ON_EN_CHANGE(IDC_EDIT1, &CTraceDlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON3, &CTraceDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CTraceDlg 消息处理程序

BOOL CTraceDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();


	 CWzdSplash wndSplash;                 //创建启动窗口类的实例   
wndSplash.Create(IDB_BITMAP1);   
wndSplash.CenterWindow();   
wndSplash.UpdateWindow();          //send WM_PAINT   
Sleep(1500);   
wndSplash.DestroyWindow();//











	// 将“关于...”菜单项添加到系统菜单中。
	CMFCButton *button=new CMFCButton;
	button->Create (_T("Test Button"),WS_VISIBLE,CRect(5,5,88,25),this,IDOK);
	button->SetWindowTextW (_T("自定义跟踪"));
	button->SetFaceColor(RGB(153,217,234));
	button->SetTextColor(RGB(255,255,200));
	button->SetTextHotColor(RGB(63,72,204));
	//button->MoveWindow(10,10,100,70);
	button->SetMouseCursorHand();
	button->SetTooltip(_T("首先选定一片区域，然后进行跟踪"));





	CMFCButton *button1=new CMFCButton;
	button1->Create (_T("Test Button"),WS_VISIBLE,CRect(5,35,88,25),this,IDC_BUTTON1);
	button1->SetWindowTextW (_T("激光点跟踪"));
	button1->SetFaceColor(RGB(153,217,234));
	button1->SetTextColor(RGB(255,255,200));
	button1->SetTextHotColor(RGB(63,72,204));
	button1->MoveWindow(5,35,88,25);
	button1->SetMouseCursorHand();
	button1->SetTooltip(_T("跟踪一个红色的激光点"));





	CMFCButton *button2=new CMFCButton;
	button2->Create (_T("Test Button"),WS_VISIBLE,CRect(5,65,88,25),this,IDC_BUTTON2);
	button2->SetWindowTextW (_T("简单的人脸识别"));
	button2->SetFaceColor(RGB(153,217,234));
	button2->SetTextColor(RGB(255,255,200));
	button2->SetTextHotColor(RGB(63,72,204));
	button2->MoveWindow(5,65,88,25);
	button2->SetMouseCursorHand();
	button2->SetTooltip(_T("检测人脸"));





CMFCButton *button3=new CMFCButton;
	button3->Create (_T("Exit"),WS_VISIBLE,CRect(371,82,80,25),this,IDCANCEL);
	button3->SetImage(IDB_BITMAP3);
	button3->MoveWindow(5,95,80,25);
	button2->SetMouseCursorHand();








	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTraceDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTraceDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		//CDialogEx::OnPaint();
		CPaintDC   dc(this);   
           CRect   rect;   
           GetClientRect(&rect);   
           CDC   dcMem;   
           dcMem.CreateCompatibleDC(&dc);   
           CBitmap   bmpBackground;   
           bmpBackground.LoadBitmap(IDB_BITMAP1);   //IDB_BITMAP是你自己的图对应的ID   ，由于我刚刚加入的位图资源 
                                                                             //被我命名成了IDB_Bg，因而我这句就是bmpBackground.LoadBitmap(IDB_Bg);  
                  
           BITMAP   bitmap;   
           bmpBackground.GetBitmap(&bitmap);   
           CBitmap   *pbmpOld=dcMem.SelectObject(&bmpBackground); 
		    dc.BitBlt(0,0,rect.Width(),rect.Height(),&dcMem,0,0,SRCCOPY); 
          // dc.StretchBlt(0,0,rect.Width(),rect.Height(),&dcMem,0,0,   
         //bitmap.bmWidth,bitmap.bmHeight,SRCCOPY);   

















	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTraceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTraceDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
	




}


void CTraceDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
	CvCapture *capture=0;
	IplImage *frame=0;

	//if(argc==1||(argc==2&&strlen(argv[1])==1&&isdigit(argv[1][0])))
		//capture=cvCaptureFromCAM(argc==2?argv[1][0]-'0':0);
	//else if(argc==2)

		capture=cvCreateCameraCapture(-1);

	/*if(!capture)

	{
	
		fprintf(stderr,"Could not initialize capturing...\n");
		return -1;
	

	}*/

	/*printf("Hot keys:\n""\tESC-quit the program\n"
		"\tc-stop the tracking\n"
		"\tb-switch to/from backprojection view\n"
		"\th-show/hide object histogram\n"
		"To initialize tracking,select the object with mouse\n");*/
	cvNamedWindow("CamShiftDemo",1);
//	cvNamedWindow("Histogram",1);
	//CvMouseCallback on_mouse="LBUTTON";
	cvSetMouseCallback("CamShiftDemo",on_mouse,NULL);
	//on_mouse( CV_EVENT_LBUTTONDOWN,10,10,0);
	//on_mouse(  CV_EVENT_LBUTTONUP,10,10,0);
	cvCreateTrackbar("Vmin","CamShiftDemo",&vmin,256,0);
	cvCreateTrackbar("Vmax","CamShiftDemo",&vmax,256,0);
	cvCreateTrackbar("Smin","CamShiftDemo",&smin,256,0);
	frame = cvQueryFrame(capture);
	for(;;)
	{


































	
	int i,bin_w,c;

	
	if(!frame)
		break;
	if(!image)
	{
	   image=cvCreateImage(cvGetSize(frame),8,3);
	   image->origin=frame->origin;
	   hsv=cvCreateImage(cvGetSize(frame),8,3);
	    hue=cvCreateImage(cvGetSize(frame),8,1);
		 mask=cvCreateImage(cvGetSize(frame),8,1);

		 backproject=cvCreateImage(cvGetSize(frame),8,1);
		 hist=cvCreateHist(1,&hdims,CV_HIST_ARRAY,&hranges,1);
		 histimg=cvCreateImage(cvSize(320,200),8,3);
		 cvZero(histimg);




	
	
	}

	cvCopy(frame,image,0);
	cvCvtColor(image,hsv,CV_BGR2HSV);
	if(track_object)
	{
	int _vmin=vmin,_vmax=vmax;
	cvInRangeS(hsv,cvScalar(0,smin,MIN(_vmin,_vmax),0),cvScalar(180,256,MAX(_vmax,_vmin),0),mask);
	cvSplit(hsv,hue,0,0,0);
	if(track_object<0)
	{
	  float max_val=0.f;
	  cvSetImageROI(hue,selection);
	  cvSetImageROI(mask,selection);
	  cvCalcHist(&hue,hist,0,mask);
	  cvGetMinMaxHistValue(hist,0,&max_val,0,0);
	  cvConvertScale(hist->bins,hist->bins,max_val?255./max_val:0,0);
	  cvResetImageROI(hue);
	  cvResetImageROI(mask);
	  track_window=selection;
	  track_object=1;
	  cvZero(histimg);
	  bin_w=histimg->width/hdims;

	  for(i=0;i<hdims;i++)
	  {
	    int val=cvRound(cvGetReal1D(hist->bins,i)*histimg->height/255);
		CvScalar color=hsv2rgb(i*180.f/hdims);
		cvRectangle(histimg,cvPoint(i*bin_w,histimg->height),cvPoint((i+1)*bin_w,histimg->height-val),color,-1,8,0);


	  
	  
	  }
	
	}


	cvCalcBackProject(&hue,backproject,hist);
	cvAnd(backproject,mask,backproject,0);

	cvCamShift(backproject,track_window,cvTermCriteria(CV_TERMCRIT_EPS|CV_TERMCRIT_ITER,10,1),&track_comp,&track_box);
	track_window=track_comp.rect;
	if(backproject_mode)
		cvCvtColor(backproject,image,CV_GRAY2BGR);
	if(image->origin)
     track_box.angle=-track_box.angle;
	m_x=track_box.center.x;
	m_y=track_box.center.y;

	UpdateData(FALSE);
	cvEllipseBox(image,track_box,CV_RGB(255,0,0),3,CV_AA,0);

	
	
	}

	if(select_object&&selection.width>0&&selection.height>0)
	{
	   cvSetImageROI(image,selection);
	   cvXorS(image,cvScalarAll(255),image,0);
	   cvResetImageROI(image);

	
	
	}
	cvShowImage("CamShiftDemo",image);
	cvShowImage("Histogram",histimg);
	c=cvWaitKey(10);
	if(c==27)
		break;
	switch(c)
	{
	case 'b':
		backproject_mode^=1;
		break;
	case 'c':
		track_object=0;
		cvZero(histimg);
		break;
	case 'h':
		show_hist^=1;
		if(!show_hist)
			cvDestroyWindow("Histogram");
		else
			cvNamedWindow("Histogram",1);
		break;
	default :;
			




















	
	
	}
	
	
	}
	

	cvReleaseCapture(&capture);
	cvDestroyWindow("CamShiftDemo");



















}


void CTraceDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

	
float h=0.0;


IplImage*frame=NULL;

CvCapture *pCapture=NULL;
cvNamedWindow("跟踪",1);

for(int i=0;i<HISTOGRAM_LENGTH;i++)

{
    currHistogram[i]=0;
	tempHistogram[i]=0;
}
    pCapture=cvCreateCameraCapture(-1);
   frame=cvQueryFrame(pCapture);
   imgWidth=frame->width;
		imgHeight=frame->height;

	for(int i=0;i<imgHeight;i++)
			for(int j=0;j<imgWidth;j++)
			{
				
			if( ((uchar*)(frame->imageData+i*frame->widthStep))[j*frame->nChannels+0]<200&&((uchar*)(frame->imageData+i*frame->widthStep))[j*frame->nChannels+0]<240&&/*((uchar*)(frame->imageData+i*frame->widthStep))[j*frame->nChannels+1]>200&&((uchar*)(frame->imageData+i*frame->widthStep))[j*frame->nChannels+1]<250&&*/((uchar*)(frame->imageData+i*frame->widthStep))[j*frame->nChannels+2]>250)
			{
				currentY=i;
					currentX=j;
					break;
			
			}
			}

		
			int count=0;


			while(pCapture=cvCreateCameraCapture(-1))
        {


			 frame=cvQueryFrame(pCapture);
			for(int i=max(0,currentY-5);i<min(imgHeight-1,currentY+5);i++)
				for(int j=max(0,currentX-5);j<min(imgWidth-1,currentX+5);j++)
				{


					if( ((uchar*)(frame->imageData+i*frame->widthStep))[j*frame->nChannels+0]<200&&/*((uchar*)(frame->imageData+i*frame->widthStep))[j*frame->nChannels+0]<240&&((uchar*)(frame->imageData+i*frame->widthStep))[j*frame->nChannels+1]>200&&((uchar*)(frame->imageData+i*frame->widthStep))[j*frame->nChannels+1]<250&&*/((uchar*)(frame->imageData+i*frame->widthStep))[j*frame->nChannels+2]>250)
			{
				    currentY=i;
					currentX=j;
					m_x=j;
	                m_y=i;

	               UpdateData(FALSE);
					count=1;
					
					break;


				
				
				
						}
				
				
				
				
				}
				

				if(count==0)
				{

		    for(int i=0;i<imgHeight;i++)
			for(int j=0;j<imgWidth;j++)
			{
			if( ((uchar*)(frame->imageData+i*frame->widthStep))[j*frame->nChannels+0]<200&&/*((uchar*)(frame->imageData+i*frame->widthStep))[j*frame->nChannels+0]<240&&((uchar*)(frame->imageData+i*frame->widthStep))[j*frame->nChannels+1]>200&&((uchar*)(frame->imageData+i*frame->widthStep))[j*frame->nChannels+1]<250&&*/((uchar*)(frame->imageData+i*frame->widthStep))[j*frame->nChannels+2]>250)
			{
				    currentY=i;
					currentX=j;
					m_x=j;
	                m_y=i;

	               UpdateData(FALSE);
					break;
			
			}
			
				

			
			}}



		
		count=0; 
		DrawTrackBox(frame);


		cvShowImage("跟踪",frame);

	     char c=cvWaitKey(10);
		  /*if(c>0)
			  break;*/
		  if(c==27)
		{
		
		break;}

			
			
			
	}

    cvDestroyWindow("跟踪");
	
	cvReleaseImage(&frame);
	















}


void CTraceDlg::OnBnClickedButton2()
{
	int x=0,y=0;
	// TODO: 在此添加控件通知处理程序代码
	CvCapture *pCapture=NULL;
    cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );
    storage = cvCreateMemStorage(0);

		cvNamedWindow( "人脸检测", 1 );
	//加载不成功则显示错误讯息，并退出
    if(cascade)
   {
	   pCapture = cvCreateCameraCapture(-1);
	   	while(src = cvQueryFrame(pCapture))
        {


			// ;
			
			
		  detect_and_draw(src);
		
		

	     char c=cvWaitKey(10);
		  if(c>0)
			  break;
		  if(c==27)
		{
		
		break;}

			
			
	}
		


		

		//如果图片存在则分析并显示结果，否则退出程序
		
		}

		
	
	else
	{
		AfxMessageBox(L"无法加载分类器，请确认后重试！");
	}

	    cvReleaseImage(&src);
		cvReleaseMemStorage( &storage );
	   cvReleaseHaarClassifierCascade( &cascade );
	   







	    cvDestroyWindow("人脸检测");





}


void CTraceDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	cvDestroyAllWindows();
	// TODO: 在此处添加消息处理程序代码
}


void CTraceDlg::OnEnChangeEdit2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CTraceDlg::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CTraceDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	CRect rect;
	GetDlgItem(IDCANCEL)->GetWindowRect(&rect);
	

	ScreenToClient(rect);
	
	m_x=rect.right;
	m_y=rect.top;
	UpdateData(FALSE);



}
