
// TraceDlg.cpp : ʵ���ļ�
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
		//�������
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
		//����ͼƬ������
	IplImage*detImg = cvCreateImage(size, image->depth, image->nChannels);
		// ��CV_INTER_NN - ���-�ھӲ岹
		// ��CV_INTER_LINEAR - ˫���Բ�ֵ��Ĭ�Ϸ�����
		// ��CV_INTER_AREA - �����������ز���������Сͼ��ʱ���÷������Ա��Ⲩ�Ƶĳ��֡����Ŵ�ͼ��ʱ�������ڷ���CV_INTER_NN��
		// ��CV_INTER_CUBIC - ˫���β�ֵ��)
		cvResize(image, detImg, CV_INTER_AREA); 
    cvShowImage( "�������", detImg);
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

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

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
		printf("\n#����ѡ������:");
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

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CTraceDlg �Ի���




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


// CTraceDlg ��Ϣ�������

BOOL CTraceDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();


	 CWzdSplash wndSplash;                 //���������������ʵ��   
wndSplash.Create(IDB_BITMAP1);   
wndSplash.CenterWindow();   
wndSplash.UpdateWindow();          //send WM_PAINT   
Sleep(1500);   
wndSplash.DestroyWindow();//











	// ��������...���˵�����ӵ�ϵͳ�˵��С�
	CMFCButton *button=new CMFCButton;
	button->Create (_T("Test Button"),WS_VISIBLE,CRect(5,5,88,25),this,IDOK);
	button->SetWindowTextW (_T("�Զ������"));
	button->SetFaceColor(RGB(153,217,234));
	button->SetTextColor(RGB(255,255,200));
	button->SetTextHotColor(RGB(63,72,204));
	//button->MoveWindow(10,10,100,70);
	button->SetMouseCursorHand();
	button->SetTooltip(_T("����ѡ��һƬ����Ȼ����и���"));





	CMFCButton *button1=new CMFCButton;
	button1->Create (_T("Test Button"),WS_VISIBLE,CRect(5,35,88,25),this,IDC_BUTTON1);
	button1->SetWindowTextW (_T("��������"));
	button1->SetFaceColor(RGB(153,217,234));
	button1->SetTextColor(RGB(255,255,200));
	button1->SetTextHotColor(RGB(63,72,204));
	button1->MoveWindow(5,35,88,25);
	button1->SetMouseCursorHand();
	button1->SetTooltip(_T("����һ����ɫ�ļ����"));





	CMFCButton *button2=new CMFCButton;
	button2->Create (_T("Test Button"),WS_VISIBLE,CRect(5,65,88,25),this,IDC_BUTTON2);
	button2->SetWindowTextW (_T("�򵥵�����ʶ��"));
	button2->SetFaceColor(RGB(153,217,234));
	button2->SetTextColor(RGB(255,255,200));
	button2->SetTextHotColor(RGB(63,72,204));
	button2->MoveWindow(5,65,88,25);
	button2->SetMouseCursorHand();
	button2->SetTooltip(_T("�������"));





CMFCButton *button3=new CMFCButton;
	button3->Create (_T("Exit"),WS_VISIBLE,CRect(371,82,80,25),this,IDCANCEL);
	button3->SetImage(IDB_BITMAP3);
	button3->MoveWindow(5,95,80,25);
	button2->SetMouseCursorHand();








	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CTraceDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
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
           bmpBackground.LoadBitmap(IDB_BITMAP1);   //IDB_BITMAP�����Լ���ͼ��Ӧ��ID   �������Ҹոռ����λͼ��Դ 
                                                                             //������������IDB_Bg�������������bmpBackground.LoadBitmap(IDB_Bg);  
                  
           BITMAP   bitmap;   
           bmpBackground.GetBitmap(&bitmap);   
           CBitmap   *pbmpOld=dcMem.SelectObject(&bmpBackground); 
		    dc.BitBlt(0,0,rect.Width(),rect.Height(),&dcMem,0,0,SRCCOPY); 
          // dc.StretchBlt(0,0,rect.Width(),rect.Height(),&dcMem,0,0,   
         //bitmap.bmWidth,bitmap.bmHeight,SRCCOPY);   

















	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CTraceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTraceDlg::OnBnClickedCancel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CDialogEx::OnCancel();
	




}


void CTraceDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	
float h=0.0;


IplImage*frame=NULL;

CvCapture *pCapture=NULL;
cvNamedWindow("����",1);

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


		cvShowImage("����",frame);

	     char c=cvWaitKey(10);
		  /*if(c>0)
			  break;*/
		  if(c==27)
		{
		
		break;}

			
			
			
	}

    cvDestroyWindow("����");
	
	cvReleaseImage(&frame);
	















}


void CTraceDlg::OnBnClickedButton2()
{
	int x=0,y=0;
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CvCapture *pCapture=NULL;
    cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );
    storage = cvCreateMemStorage(0);

		cvNamedWindow( "�������", 1 );
	//���ز��ɹ�����ʾ����ѶϢ�����˳�
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
		


		

		//���ͼƬ�������������ʾ����������˳�����
		
		}

		
	
	else
	{
		AfxMessageBox(L"�޷����ط���������ȷ�Ϻ����ԣ�");
	}

	    cvReleaseImage(&src);
		cvReleaseMemStorage( &storage );
	   cvReleaseHaarClassifierCascade( &cascade );
	   







	    cvDestroyWindow("�������");





}


void CTraceDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	cvDestroyAllWindows();
	// TODO: �ڴ˴������Ϣ����������
}


void CTraceDlg::OnEnChangeEdit2()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}


void CTraceDlg::OnEnChangeEdit1()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}


void CTraceDlg::OnBnClickedButton3()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CRect rect;
	GetDlgItem(IDCANCEL)->GetWindowRect(&rect);
	

	ScreenToClient(rect);
	
	m_x=rect.right;
	m_y=rect.top;
	UpdateData(FALSE);



}
