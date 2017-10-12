#include "stdafx.h"  
#include "stdlib.h"  
#include <windows.h>    
#include <stdio.h>    
#include <mmsystem.h>    
#pragma comment(lib, "winmm.lib")      
#define BUFFER_SIZE (44100*16*2/8*5)    // 录制声音长度  
#define FRAGMENT_SIZE 1024              // 缓存区大小  
#define FRAGMENT_NUM 4                  // 缓存区个数   
static unsigned char buffer[BUFFER_SIZE] = { 0 };
static int buf_count = 0;
// 函数定义  
void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
// 录音回调函数  
void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	LPWAVEHDR pwh = (LPWAVEHDR)dwParam1;
	if ((WIM_DATA == uMsg) && (buf_count<BUFFER_SIZE))
	{
		int temp = BUFFER_SIZE - buf_count;
		temp = (temp>pwh->dwBytesRecorded) ? pwh->dwBytesRecorded : temp;
		memcpy(buffer + buf_count, pwh->lpData, temp);
		buf_count += temp;
		waveInAddBuffer(hwi, pwh, sizeof(WAVEHDR));
	}
}
void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
// 放音回调函数  
void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	if (WOM_DONE == uMsg)
	{
		buf_count = BUFFER_SIZE;
	}
}

// 入口  

int _tmain(int argc, _TCHAR* argv[])
{
	/* 录音*/ // Device    
	int nReturn = waveInGetNumDevs();//定义输入设备的数目  
	printf("输入设备数目：%d\n", nReturn);

	//识别输入的设备  
	for (int i = 0; i<nReturn; i++)
	{
		WAVEINCAPS wic;  //WAVEINCAPS结构描述波形音频输入设备的能力  
		waveInGetDevCaps(i, &wic, sizeof(WAVEINCAPS)); //waveInGetDevCaps功能检索一个给定的波形音频输入设备的能力  
		printf("#%d\t设备名：%s\n", i, wic.szPname);
	}

	// open    

	HWAVEIN hWaveIn;//波形音频数据格式Wave_audio数据格式  
	WAVEFORMATEX wavform;//WAVEFORMATEX结构定义了波形音频数据格式。包括在这个结构中唯一的格式信息，共同所有波形音频数据格式。对于需要额外的信息的格式，这个结构包含在另一个结构的第一个成员，以及与其他信息  
	wavform.wFormatTag = WAVE_FORMAT_PCM;  //WAVE_FORMAT_PCM即脉冲编码  
	wavform.nChannels = 2;  // 声道  
	wavform.nSamplesPerSec = 44100; // 采样频率  
	wavform.nAvgBytesPerSec = 44100 * 16 * 2 / 8;  // 每秒数据量  
	wavform.nBlockAlign = 4;
	wavform.wBitsPerSample = 16; // 样本大小  
	wavform.cbSize = 0;  //大小，以字节，附加额外的格式信息WAVEFORMATEX结构  

	//打开录音设备函数  
	waveInOpen(&hWaveIn, WAVE_MAPPER, &wavform, (DWORD_PTR)waveInProc, 0, CALLBACK_FUNCTION);

	//识别打开的录音设备  
	WAVEINCAPS wic;
	waveInGetDevCaps((UINT_PTR)hWaveIn, &wic, sizeof(WAVEINCAPS));
	printf("打开的输入设备：%s\n", wic.szPname);


	// prepare buffer  
	static WAVEHDR wh[FRAGMENT_NUM];
	for (int i = 0; i<FRAGMENT_NUM; i++)
	{
		wh[i].lpData = new char[FRAGMENT_SIZE];
		wh[i].dwBufferLength = FRAGMENT_SIZE;
		wh[i].dwBytesRecorded = 0;
		wh[i].dwUser = NULL;
		wh[i].dwFlags = 0;
		wh[i].dwLoops = 1;
		wh[i].lpNext = NULL;
		wh[i].reserved = 0;

		//为录音设备准备缓存函数：  
		//MMRESULT waveInPrepareHeader(  HWAVEIN hwi,  LPWAVEHDR pwh, UINT bwh );  
		waveInPrepareHeader(hWaveIn, &wh[i], sizeof(WAVEHDR));

		//给输入设备增加一个缓存：  
		//MMRESULT waveInAddBuffer(  HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh );  
		waveInAddBuffer(hWaveIn, &wh[i], sizeof(WAVEHDR));
	}
	// record    
	printf("Start to Record...\n");
	buf_count = 0; //刚开始录音的时候缓冲区的个数初始化为  
	//开始录音函数  
	//MMRESULT waveInStart(  HWAVEIN hwi  );    
	waveInStart(hWaveIn); //开始录音  

	while (buf_count < BUFFER_SIZE)
	{
		Sleep(1);
	}

	printf("Record Over!\n\n");
	waveInStop(hWaveIn);//waveInStop功能停止的波形音频输入  
	//停止录音函数：  
	//MMRESULT waveInReset( HWAVEIN hwi );   
	waveInReset(hWaveIn);//停止录音  
	//清除缓存函数：  
	//MMRESULT waveInUnprepareHeader( HWAVEIN hwi,LPWAVEHDR pwh, UINT cbwh);    
	for (int i = 0; i<FRAGMENT_NUM; i++)
	{
		waveInUnprepareHeader(hWaveIn, &wh[i], sizeof(WAVEHDR));
		delete wh[i].lpData;
	}
	//关闭录音设备函数：  
	//MMRESULT waveInClose( HWAVEIN hwi );  
	waveInClose(hWaveIn);
	//system("pause");   
	printf("\n");

	/* 放音*/

	// Device    
	nReturn = waveOutGetNumDevs();  //定义输出设备的数目  
	printf("\n输出设备数目：%d\n", nReturn);
	for (int i = 0; i<nReturn; i++)
	{
		WAVEOUTCAPS woc;  //WAVEINCAPS结构描述波形音频输出设备的能力  
		waveOutGetDevCaps(i, &woc, sizeof(WAVEOUTCAPS));
		printf("#%d\t设备名：%s\n", i, wic.szPname);
	}

	// open    
	HWAVEOUT hWaveOut;//打开回放设备函数　  
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wavform, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);

	WAVEOUTCAPS woc; //WAVEINCAPS结构描述波形音频输出设备的能力  
	waveOutGetDevCaps((UINT_PTR)hWaveOut, &woc, sizeof(WAVEOUTCAPS));
	printf("打开的输出设备：%s\n", wic.szPname);

	// prepare buffer    
	WAVEHDR wavhdr;
	wavhdr.lpData = (LPSTR)buffer;
	wavhdr.dwBufferLength = BUFFER_SIZE;  //MMRESULT waveOutPrepareHeader(HWAVEOUT hwo,LPWAVEHDR pwh,UINT cbwh);  
	wavhdr.dwFlags = 0;  //为回放设备准备内存块函数　  
	wavhdr.dwLoops = 0;
	waveOutPrepareHeader(hWaveOut, &wavhdr, sizeof(WAVEHDR));
	// play    
	printf("Start to Play...\n");
	buf_count = 0;
	//MMRESULT waveOutWrite(HWAVEOUT hwo,LPWAVEHDR pwh,UINT cbwh);  
	waveOutWrite(hWaveOut, &wavhdr, sizeof(WAVEHDR)); //写数据（放音）函数  
	//声音文件还没有放完的话运行不能退出  
	while (buf_count < BUFFER_SIZE)
	{
		Sleep(1);
	}
	// clean    
	waveOutReset(hWaveOut); //停止放音  
	//MMRESULT waveOutPrepareHeader(HWAVEOUT hwo,LPWAVEHDR pwh,UINT cbwh);  
	waveOutUnprepareHeader(hWaveOut, &wavhdr, sizeof(WAVEHDR)); //为回放设备准备内存块函数　  
	waveOutClose(hWaveOut);  //关闭放音设备函数  
	printf("Play Over!\n\n");
	printf("Play Over!\n\n");
	return 0;
}