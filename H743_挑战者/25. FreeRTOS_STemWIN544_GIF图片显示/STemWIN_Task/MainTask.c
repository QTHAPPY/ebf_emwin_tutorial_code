/*********************************************************************
*                                                                    *
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
*                                                                    *
**********************************************************************
*                                                                    *
* C-file generated by:                                               *
*                                                                    *
*        GUI_Builder for emWin version 5.44                          *
*        Compiled Nov 10 2017, 08:53:57                              *
*        (c) 2017 Segger Microcontroller GmbH & Co. KG               *
*                                                                    *
**********************************************************************
*                                                                    *
*        Internet: www.segger.com  Support: support@segger.com       *
*                                                                    *
**********************************************************************
*/
#include <stddef.h>
#include <string.h>
/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
/* STemWIN头文件 */
#include "ScreenShot.h"
#include "MainTask.h"
#include "./usart/bsp_usart.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

/*********************************************************************
*
*       Types
*
**********************************************************************
*/


/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static char *_acbuffer = NULL;
static char _acBuffer[1024 * 4];

UINT    f_num;
extern FATFS   fs;								/* FatFs文件系统对象 */
extern FIL     file;							/* file objects */
extern FRESULT result; 
extern DIR     dir;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/**
  * @brief 从存储器中读取数据
  * @note 无
  * @param 
  * @retval NumBytesRead：读到的字节数
  */
int _GetData(void * p, const U8 ** ppData, unsigned NumBytesReq, U32 Off)
{
	static int FileAddress = 0;
	UINT NumBytesRead;
	FIL *Picfile;
	
	Picfile = (FIL *)p;
	
	if(NumBytesReq > sizeof(_acBuffer))
	{NumBytesReq = sizeof(_acBuffer);}
	
	if(Off == 1) FileAddress = 0;
	else FileAddress = Off;
	result = f_lseek(Picfile, FileAddress);
	
	/* 进入临界段 */
	taskENTER_CRITICAL();
	result = f_read(Picfile, _acBuffer, NumBytesReq, &NumBytesRead);
	/* 退出临界段 */
	taskEXIT_CRITICAL();
	
	*ppData = (const U8 *)_acBuffer;
	
	return NumBytesRead;
}

/**
  * @brief 直接从存储器中绘制BMP图片数据
  * @note 无
  * @param sFilename：需要加载的图片名
  * @retval 无
  */
static void ShowGIFEx(const char *sFilename)
{
	GUI_GIF_INFO Gifinfo = {0};
	GUI_GIF_IMAGE_INFO Imageinfo = {0};
	int i = 0;
	
	/* 进入临界段 */
	taskENTER_CRITICAL();
	/* 打开图片 */
	result = f_open(&file, sFilename, FA_READ);
	if ((result != FR_OK))
	{
		printf("文件打开失败！\r\n");
		_acBuffer[0]='\0';
	}
	/* 退出临界段 */
	taskEXIT_CRITICAL();
  
	/* 获取GIF文件信息 */
	GUI_GIF_GetInfoEx(_GetData, &file, &Gifinfo);	
	/* 循环显示所有的GIF帧 */
	for(i = 0; i < Gifinfo.NumImages; i++)
	{
    /* 获取GIF子图象信息 */
    GUI_GIF_GetImageInfoEx(_GetData, &file, &Imageinfo, i);
    /* 绘制GIF子图象 */
		GUI_GIF_DrawSubEx(_GetData, &file,
										(LCD_GetXSize() - Gifinfo.xSize) / 2,
										(LCD_GetYSize() - Gifinfo.ySize) / 2, i);
    /* 帧延时 */
		GUI_Delay(Imageinfo.Delay);
	}
	/* 读取完毕关闭文件 */
	f_close(&file);
}

/**
  * @brief 加载GIT图片到内存中并绘制
  * @note 无
  * @param sFilename：需要加载的图片名
  * @retval 无
  */
static void ShowGIF(const char *sFilename)
{
	WM_HMEM hMem;
	GUI_GIF_INFO Gifinfo = {0};
	GUI_GIF_IMAGE_INFO Imageinfo = {0};
	int i = 0;
	int j = 0;
  
	/* 进入临界段 */
	taskENTER_CRITICAL();
	/* 打开图片 */
	result = f_open(&file, sFilename, FA_READ);
	if ((result != FR_OK))
	{
		printf("文件打开失败！\r\n");
		_acbuffer[0]='\0';
	}
	
	/* 申请一块动态内存空间 */
	hMem = GUI_ALLOC_AllocZero(file.fsize);
	/* 转换动态内存的句柄为指针 */
	_acbuffer = GUI_ALLOC_h2p(hMem);

	/* 读取图片数据到动态内存中 */
	result = f_read(&file, _acbuffer, file.fsize, &f_num);
	if(result != FR_OK)
	{
		printf("文件读取失败！\r\n");
	}
	/* 读取完毕关闭文件 */
	f_close(&file);
	/* 退出临界段 */
	taskEXIT_CRITICAL();
  
	/* 获取GIF文件信息 */
	GUI_GIF_GetInfo(_acbuffer, file.fsize, &Gifinfo);
	/* 显示2遍GIF */
	for(j = 0; j < 2; j++)
	{
		/* 循环显示所有的GIF帧 */
		for(i = 0; i<Gifinfo.NumImages; i++)
		{
      /* 获取GIF子图象信息 */
      GUI_GIF_GetImageInfo(_acbuffer, file.fsize, &Imageinfo, i);
      /* 绘制GIF子图象 */
			GUI_GIF_DrawSub(_acbuffer, file.fsize,
											(LCD_GetXSize() - Gifinfo.xSize) / 2,
											(LCD_GetYSize() - Gifinfo.ySize) / 2, i);
       /* 帧延时 */
			GUI_Delay(Imageinfo.Delay);
		}
	}
	/* 释放内存 */
	GUI_ALLOC_Free(hMem);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/**
  * @brief GUI主任务
  * @note 无
  * @param 无
  * @retval 无
  */
void MainTask(void)
{
	/* 设置背景色 */
	GUI_SetBkColor(GUI_WHITE);
	GUI_Clear();
	/* 设置字体 */
	GUI_SetFont(GUI_FONT_24B_ASCII);
	GUI_SetColor(GUI_BLACK);
	while (1)
	{
		/* 直接从存储器中绘制BMP图片数据 */
		GUI_DispStringHCenterAt("ShowGIFEx", LCD_GetXSize()/2, 10);
		ShowGIFEx("0:/image/dolphin.gif");
		GUI_Delay(100);
		GUI_Clear();
		
		/* 加载GIT图片到内存中并绘制 */
		GUI_DispStringHCenterAt("ShowGIF", LCD_GetXSize()/2, 10);
		ShowGIF("0:/image/dolphin.gif");
		GUI_Delay(100);
		GUI_Clear();
		
		GUI_DispStringHCenterAt("ShowGIF", LCD_GetXSize()/2, 10);
		ShowGIF("0:/image/rabbit.gif");
		GUI_Delay(100);
		GUI_Clear();
		
		GUI_DispStringHCenterAt("ShowGIF", LCD_GetXSize()/2, 10);
		ShowGIF("0:/image/Groundhog.gif");
		GUI_Delay(100);
		GUI_Clear();
	}
}

/*************************** End of file ****************************/
