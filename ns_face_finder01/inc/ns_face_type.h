#ifndef _NS_FACE_TYPE_H_
#define _NS_FACE_TYPE_H_

#include "ns_face_define.h"

typedef struct _ns_point_ {
	int x;
	int y;
} NS_POINT;

typedef struct _ns_rect_ {
	int left;
	int top;
	int right;
	int bottom;
} NS_RECT;

#define  MAX_RECT_NUM  16

typedef struct _nsf_roi_ {
	int num;
	NS_RECT rc[MAX_RECT_NUM];
} NSF_ROI;


typedef struct _nsf_face_ {
	int xtl;            /* x top-left     */
	int ytl;            /* y top-left     */
	int xtr;            /* x top-right    */
	int ytr;            /* y top-right    */
	int xbr;            /* x bottom-right */
	int ybr;            /* y bottom-right */
	int xbl;            /* x bottom-left  */
	int ybl;            /* y bottom-left  */
	int pose_angle[3];  /* YPR */
	int size;
	// float???
	float confidence;
	unsigned char quality; 
} NSF_FACE;

typedef struct _nsf_fd_param_ {
	int min_face_size;
	int max_face_size;
	NSF_ROI *roi;
} NSF_FD_PARAM;

typedef struct _nsf_eye_ {
	int xleft;
	int yleft;
	int xright;
	int yright;
	int confidence;
	unsigned char l_quality;
	unsigned char r_quality;	
} NSF_EYE;

typedef struct _nsf_lm_info_ {
	NSF_FACE face;
	NSF_EYE eye;
} NSF_LM_INFO; //landmark info

typedef struct _nsf_image_ {
	unsigned char *data;
	int data_len;
	int width;
	int height;
	int type;
	int format;
	NSF_LM_INFO lm_info;
} NSF_IMAGE;

typedef struct _nsf_feature_info_ {
	int type;
	int size;
} NSF_FEATURE_INFO;


#define NS_DATA_HEAD_SIZE	12
typedef struct _ns_data_ {
	char data_name[32];
	int  data_size;
	int  offset;	
	unsigned char* data_buf;	
} NS_DATA;




#endif