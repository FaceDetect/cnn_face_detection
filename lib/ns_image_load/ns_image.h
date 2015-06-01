#ifndef _NS_IMAGE_H_
#define _NS_IMAGE_H_

#include "ns_image_type.h"

//////////////////////////////////////////////////////////////////////////
#ifdef _WINDOWS
	#ifdef NS_IMAGE_EXPORTS
		#define NS_IMAGE_API __declspec(dllexport)
	#else
		#define NS_IMAGE_API __declspec(dllimport)
	#endif
#else	//_WINDOWS
	#define NS_IMAGE_API
#endif	//_WINDOWS

//////////////////////////////////////////////////////////////////////////

typedef enum NS_ImageEncodeType
{
	NS_IET_NONE = 0,
	NS_IET_JPG,
	NS_IET_BMP,
	NS_IET_TIF,
	NS_IET_PGM,
	NS_IET_PNG,
	NS_IET_J2K,
	NS_IET_JP2,
} NS_ImageEncodeType;

typedef struct NS_IMAGE
{
	unsigned char *pEnc;			// 保存编码数据的缓冲区
	unsigned char *pRaw;			// 保存原始图像数据的缓冲区，8位，灰度或者BGR彩色

	NS_ImageEncodeType nEncType;	// 编码数据的格式
	int nEncLen;					// 编码缓冲区长度

	int nWidth, nHeight, nClrBit;	// 图像的宽，高及每个像素占用的bit数
	int nFlag;						// 内部使用

} NS_IMAGE;

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif

	NS_IMAGE_API NS_RESULT NS_Image_Init(NS_IMAGE *pImage);
	/*
	描述：
		初始化NS_IMAGE对象
	参数：
		pImage	[io]:		待初始化的NS_IMAGE对象
	返回值：
		NS_S_OK			成功
		NS_E_NULL			输入指针为空
	说明：
		每个NS_IMAGE对象必须且只能调用一次NS_Image_Init，多次调用可能导致内存泄露
	*/

	NS_IMAGE_API NS_RESULT NS_Image_Destroy(NS_IMAGE *pImage);
	/*
	描述：
		清除NS_IMAGE对象
	参数：
		pImage	[io]:		待清除的NS_IMAGE对象
	返回值：
		NS_S_OK			成功
		NS_E_NULL			输入指针为空
	说明：
		使用完NS_IMAGE后需要调用此函数进行清除，清除后的状态与初始化后一致，可继续使用。
	*/

	NS_IMAGE_API NS_RESULT NS_Image_Load(NS_IMAGE *pImage, const char *pszPath, int bKeepEnc = 0, int bDecode = 1);
	/*
	描述：
		从文件读取图像
	参数：
		pImage	[io]:		保存读取结果的对象
		pszPath	[in]:		输入图像文件路径
		bKeepEnc[in]:		若非0，则成功读取图像后在pImage中保存图像的原始编码缓冲区
		bDecode	[in]:		若非0，则读取图像后进行解码
	返回值：
		NS_S_OK			成功
		NS_E_NULL			输入指针为空
		NS_E_FORMAT		文件格式错误
		NS_E_MEMORY		申请内存失败
		NS_E_STORAGE_OPEN	打开文件失败
		NS_E_STORAGE_READ	读取文件错误
		NS_E_UNSUPPORT		不支持的图像格式
		NS_E_FAIL			图像解码失败
	说明：
		该函数从文件中读取图像，将结果保存在NS_IMAGE结构中。若读取成功，pImage中保存指定信息，若失败，pImage中原有内容会被清空
	*/

	NS_IMAGE_API NS_RESULT NS_Image_SetEncBuf(NS_IMAGE *pImage, const unsigned char *pEnc, int nLen, int bCopy = 0);
	/*
	描述：
		设置编码数据
	参数：
		pImage	[io]:		保存编码数据的NS_IMAGE对象
		pEnc	[in]:		指定的编码数据缓冲区
		nLen	[in]:		编码数据缓冲区长度
		bCopy	[in]:		若非0，则函数将申请内存，并复制所指定缓冲区的内容
	返回值：
		NS_S_OK			成功
		NS_E_NULL			输入指针为空
		NS_E_MEMORY		申请内存失败
	说明：
		该函数在pImage中保存编码数据缓冲区，为解码提供数据。该函数不会修改原始图像数据缓冲区。
	*/

	NS_IMAGE_API NS_RESULT NS_Image_DecodeInfo(NS_IMAGE *pImage);
	/*
	描述：
		对编码数据进行解码，获取基本信息，不解码原始图像数据
	参数:
		pImage	[io]:		待解码的图像对象
	返回值：
		NS_S_OK			成功
		NS_E_NULL			输入指针为空或未设置编码缓冲区
		NS_E_UNSUPPORT		不支持的图像格式
		NS_E_FAIL			图像解码失败
	说明：
		该函数对编码数据进行解码，获取图像的格式，长，宽以及像素bit信息。
	*/

	NS_IMAGE_API NS_RESULT NS_Image_Decode(NS_IMAGE *pImage, unsigned char *pRaw = NULL, int nLen = 0);
	/*
	描述：
		对编码数据进行解码，获取原始图像数据
	参数:
		pImage	[io]:		待解码的图像对象
		pRaw	[out]:		指向输出原始图像数据缓冲区的指针，可以为NULL
		nLen	[in]		原始图像数据缓冲区长度，pRaw为NULL时，该值被忽略
	返回值：
		NS_S_OK			成功
		NS_E_NULL			输入指针为空或未设置编码缓冲区
		NS_E_UNSUPPORT		不支持的图像格式
		NS_E_BUFFER		输入的原始图像数据缓冲区大小不够
		NS_E_MEMORY		申请内存失败
		NS_E_FAIL			图像解码失败
	说明：
		该函数对编码数据进行解码，获取图像信息及原始图像数据。
		若pRaw非空，则解码得到的原始图像数据将放至所指定的缓冲区中，否则，函数将内部申请空间存放数据。
	*/

	NS_IMAGE_API NS_RESULT NS_Image_SetRawBuf(NS_IMAGE *pImage, const unsigned char *pRaw, int nWidth, int nHeight, int nClrBit, int bCopy = 0);
	/*
	描述：
		设置原始图像数据
	参数：
		pImage	[io]:		保存原始图像数据的NS_IMAGE对象
		pRaw	[in]:		指定的原始图像数据缓冲区
		nWidth	[in]:		原始图像宽
		nHeight [in]:		原始图像高
		nClrBit	[in]:		原始图像像素bit
		bCopy	[in]:		若非0，则函数将申请内存拷贝所指定缓冲区的内容
	返回值：
		NS_S_OK			成功
		NS_E_NULL			输入指针为空
		NS_E_MEMORY		申请内存失败
	说明：
		该函数在pImage中保存原始图像数据，为Decode提供数据。该函数不会修改原始数据缓冲区的内容。
	*/

	NS_IMAGE_API NS_RESULT NS_Image_Encode(NS_IMAGE *pImage, NS_ImageEncodeType nEncType, const int *pnParam = NULL, unsigned char *pEnc = NULL, int nLen = 0);
	/*
	描述：
		对原始图像数据进行编码，获得编码数据
	参数：
		pImage	[io]:		待编码的图像对象
		nEncType[in]:		编码的图像格式
		pnParam	[in]:		编码参数，详见说明部分
		pEnc	[in]:		指向输出编码数据缓冲区的指针，可以为NULL
		nLen	[in]:		编码数据缓冲区长度，pEnc为NULL时，该值被忽略
	返回值：
		NS_S_OK			成功
		NS_E_NULL			输入指针为空或未设置原始图像数据缓冲区
		NS_E_PARAM			nEncType 参数错误
		NS_E_MEMORY		申请内存失败
		NS_E_UNSUPPORT		不支持的图像格式
		NS_E_BUFFER		输入的编码数据缓冲区大小不够
		NS_E_FAIL			图像编码失败
	说明：
		该函数对原始图像数据进行编码，获取编码数据。
		若pEnc非空，则编码得到的数据将放置到所指定的缓冲区中，否则，函数将内部申请空间存放数据。
		pnParam指定了编码时的参数，对于不同编码格式(nEncType)有不同含义：
			NS_IET_JPG
				pnParam[0]:	图像质量，范围[0,100]，默认值 80
			NS_IET_J2K
				pnParam[0]:	图像质量，范围[0,100]，默认值 70
			NS_IET_JP2
				pnParam[0]:	图像质量，范围[0,100]，默认值 70
	*/

	NS_IMAGE_API NS_RESULT NS_Image_Gray(NS_IMAGE *pImage, unsigned char *pGray = NULL, int nLen = 0);
	/*
	描述：
		将BGR彩色图像转成灰度图像
	参数：
		pImage	[io]:		待转换的灰度图像
		pGray	[out]:		输出灰度图像缓冲区
		nLen	[in]:		灰度图像缓冲区长度，pGray为NULL，该值被忽略
	返回值：
		NS_S_OK			成功
		NS_E_NULL			输入指针为空或未设置原始图像数据缓冲区
		NS_E_BUFFER		输入灰度图像缓冲区大小不够
	说明：
		该函数将BGR彩色图像转换成灰度图像。若pGray为空，得到的灰度图像将保存在原始图像数据缓冲区中，原来的
		彩色图像被破坏，图像的像素bit数变为8。若pGray不为空，得到的灰度图像将保存在pGray所指定的缓冲区中，
		pImage对象中的数据保持不变。
	*/

#ifdef __cplusplus
};
#endif

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus

class CNS_Image : public NS_IMAGE
{
public:
	CNS_Image(void)		{NS_Image_Init(this);}
	~CNS_Image()		{NS_Image_Destroy(this);}
	void Clear()		{NS_Image_Destroy(this);}
	
	NS_RESULT Load(const char *pszPath, int nKeepEnc = 0, int nDecode = 1)
	{
		return NS_Image_Load(this, pszPath, nKeepEnc, nDecode);
	}
	NS_RESULT SetEncBuf(const unsigned char *pEnc, int nLen, int nCopy = 0)
	{
		return NS_Image_SetEncBuf(this, pEnc, nLen, nCopy);
	}
	NS_RESULT DecodeInfo()
	{
		return NS_Image_DecodeInfo(this);
	}
	NS_RESULT Decode(unsigned char *pRaw = NULL, int nLen = 0)
	{
		return NS_Image_Decode(this, pRaw, nLen);
	}
	NS_RESULT SetRawBuf(const unsigned char *pRaw, int nWidth, int nHeight, int nClrBit, int bCopy = 0)
	{
		return NS_Image_SetRawBuf(this, pRaw, nWidth, nHeight, nClrBit, bCopy);
	}
	NS_RESULT Encode(NS_ImageEncodeType nEncType, const int *pnParam = NULL, unsigned char *pEnc = NULL, int nLen = 0)
	{
		return NS_Image_Encode(this, nEncType, pnParam, pEnc, nLen);
	}
	NS_RESULT Gray(unsigned char *pGray = NULL, int nLen = 0)
	{
		return NS_Image_Gray(this, pGray, nLen);
	}
};

#endif	// __cplusplus

//////////////////////////////////////////////////////////////////////////

#endif	// _NS_IMAGE_H_