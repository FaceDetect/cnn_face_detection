#ifndef _NS_FACE_DEFINE_H_
#define _NS_FACE_DEFINE_H_

#define NSF_SUCCESS 0
#define NSF_FAILED  -1

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif

// Image data type, one digit
#define  NSF_IMG_TYPE_GRAY		0
#define  NSF_IMG_TYPE_BGR		1
#define  NSF_IMG_TYPE_RGB		2
#define  NSF_IMG_TYPE_YUV		3
#define  NSF_IMG_TYPE_UNKNOWN    9

// Image format, one digit
#define  NSF_IMG_FORMAT_RAW		0
#define  NSF_IMG_FORMAT_BMP		1
#define  NSF_IMG_FORMAT_JPEG		2
#define  NSF_IMG_FORMAT_DIB		3
#define  NSF_IMG_FORMAT_J2K      4
#define  NSF_IMG_FORMAT_UNKNOWN  9

// Gender
#define  NSF_GENDER_UNKNOWN		0
#define  NSF_GENDER_MALE			1
#define  NSF_GENDER_FEMALE		2

// Age
#define NSF_BIRTHYEAR_UNKNOWN	0

#endif