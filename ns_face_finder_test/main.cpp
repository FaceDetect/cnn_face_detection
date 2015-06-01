#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "ns_image.h"
#include "ns_face_finder01.h"

#define MAX(a,b) ( ((a)>(b)) ? (a):(b) )
#define MIN(a,b) ( ((a)>(b)) ? (b):(a) )

int demo_face_detection_test()
{
	// load detection image
	char *fn  = "Z:\\Face_DB\\aflw\\image\\0\\image01622.jpg";
	//char *fn = "Z:\\Face_DB\\aflw\\image\\0\\image00002.jpg";
	//char *fn = "K:\\AFW\\testimages\\134212.jpg";
	//char *fn = "K:\\AFW\\testimages\\1602308.jpg";

	//char *fn = "K:\\FDDB\\originalPics\\2002\\07\\19\\big\\img_18.jpg";
	//char *fn = "Z:\\Face_DB\\lfw\\image\\Aaron_Eckhart\\Aaron_Eckhart_0001.jpg";
	CNS_Image img;
	if(img.Load(fn, 0, 1) != NS_S_OK)
	{
		printf("Load image error!\n");
		return 1;
	}
	if(img.Gray(NULL, 0) != NS_S_OK)
	{
		printf("Convert image to gray error!\n");
		return 1;
	}
	NSF_IMAGE *image = (NSF_IMAGE *)malloc(sizeof(NSF_IMAGE));
	image->width = img.nWidth;
	image->height = img.nHeight;
	image->data_len = img.nWidth*img.nHeight*img.nClrBit;
	image->data = img.pRaw;
	image->type = NSF_IMG_TYPE_GRAY;
	image->format = NSF_IMG_FORMAT_RAW;

	// process
	char *data_path = "U:\\face_detection\\model";
	char *message = (char *)malloc(sizeof(char)*256);
	NS_FACE_FINDER01_HANDLE handle = (NS_FACE_FINDER01_HANDLE *)malloc(sizeof(NS_FACE_FINDER01_HANDLE));
	int ret_init, ret_face_finder, ret_destroy;

	NSF_FD_PARAM *face_param = (NSF_FD_PARAM*)malloc(sizeof(NSF_FD_PARAM));
	face_param->max_face_size = MIN(image->width, image->height);
	face_param->min_face_size = 40;
	NSF_FACE *box_out = (NSF_FACE *)malloc(sizeof(NSF_FACE)*1000);
	int *face_num = (int *)malloc(sizeof(int));

	clock_t start,finish;
	start = clock();
	ret_init = ns_face_finder01_init(&handle, data_path, message);

	ret_face_finder = ns_face_finder01(handle, image, face_param, box_out, 10000, face_num, message);

	ret_destroy = ns_face_finder01_destroy(&handle, message);

	//output
	FILE *fp;
	fp = fopen("test_nms.txt", "w+");
	fprintf(fp, "%d\n", *face_num);
	for(int i=0; i < *face_num; i++)
	{
		int xtl = (box_out+i)->xtl;
		int xbr = (box_out+i)->xbr;
		int ytl = (box_out+i)->ytl;
		int ybr = (box_out+i)->ybr;
		fprintf(fp, "%d %d %d %d %f\n", xtl, ytl, xbr, ybr, (box_out+i)->confidence);
	}
	fclose(fp);

	finish = clock();
	printf("Time is %f\n", (double)(finish-start)/CLOCKS_PER_SEC);

	return 0;
}

int extract_negative_samples(char *image_data_path, char *data_path, char *list_path, char *output_path)
{
	// image path
	//char *image_data_path = "Z:\\User\\wuxiang\\code\\matlab\\face_detection\\data\\InriaPerson_octave_0_train\\train\\negatives_octave_0.0"; //bootstrap01
	//char *image_data_path = "Z:\\Face_DB";					//bootstrap02
	//char *image_data_path = "K:\\AFW\\testimages";   //test AFW
	//char *image_data_path = "Z:\\User\\wuxiang\\code\\matlab\\face_detection\\data\\facedata\\facedata";
	FILE *fp = NULL;
	FILE *fp_out = NULL;
	int ret;
	char buff[256];
	char image_path[1024];
	char fr_path[1024];
	fp = fopen(list_path, "r");
	if(fp == NULL)
	{
		printf("Open image list error!\n");
		return -1;
	}

	//detection net init
	//char *data_path = "U:\\face_detection\\model";
	char *message = (char *)malloc(sizeof(char)*256);
	NS_FACE_FINDER01_HANDLE handle = (NS_FACE_FINDER01_HANDLE *)malloc(sizeof(NS_FACE_FINDER01_HANDLE));
	int ret_init, ret_face_finder, ret_destroy;
	NSF_IMAGE *image = (NSF_IMAGE *)malloc(sizeof(NSF_IMAGE));
	NSF_FD_PARAM *face_param = (NSF_FD_PARAM*)malloc(sizeof(NSF_FD_PARAM));
	NSF_FACE *box_out = (NSF_FACE *)malloc(sizeof(NSF_FACE)*1000);
	int *face_num = (int *)malloc(sizeof(int));
	ret_init = ns_face_finder01_init(&handle, data_path, message);

	while(!feof(fp))
	{
		ret = fscanf(fp, "%s", buff);
		sprintf(image_path, "%s\\%s", image_data_path, buff);
		CNS_Image img;
		if(img.Load(image_path, 0, 1) != NS_S_OK)
		{
			printf("Load image error!\n");
			continue;
		}
		if(img.Gray(NULL, 0) != NS_S_OK)
		{
			printf("Convert image to gray error!\n");
			continue;
		}

		image->width = img.nWidth;
		image->height = img.nHeight;
		image->data_len = img.nWidth*img.nHeight*img.nClrBit;
		image->data = img.pRaw;
		image->type = NSF_IMG_TYPE_GRAY;
		image->format = NSF_IMG_FORMAT_RAW;
		face_param->max_face_size = MIN(image->width, image->height);
		face_param->min_face_size = 40;


		ret_face_finder = ns_face_finder01(handle, image, face_param, box_out, 10000, face_num, message);

		if(ret_face_finder != 0)
		{
			printf("Error in face detector, %d Error\n", ret_face_finder);
			continue;
		}

		//output
		sprintf(fr_path, "%s\\%s.fr", output_path, buff);
		fp_out = fopen(fr_path, "w+");


		if(*face_num ==0)
		{
			fprintf(fp_out, "%d\n", 0);
		}
		else
		{
			fprintf(fp_out, "%d\n", *face_num);
			for(int i=0; i < *face_num; i++)
			{
				int xtl = (box_out+i)->xtl;
				int xbr = (box_out+i)->xbr;
				int ytl = (box_out+i)->ytl;
				int ybr = (box_out+i)->ybr;
				fprintf(fp_out, "%d %d %d %d %f\n", xtl, ytl, xbr, ybr, (box_out+i)->confidence);
			}
		}

		fclose(fp_out);

		printf("%s is complete!\n", buff);

		img.Clear();

	}

	ret_destroy = ns_face_finder01_destroy(&handle, message);
	fclose(fp);
	fp = NULL;

	return 0;
}

int main(int argc, char **argv)
{
	if(argc!=5)
	{
		printf("Usage: ns_face_finder_test image_data_path model_data_path image_list output!\n");
		return 0;
	}
	char *image_data_path = argv[1];
	char *data_path = argv[2];
	char *list_path = argv[3];
	char *output_path = argv[4];

	extract_negative_samples(image_data_path, data_path, list_path, output_path);
	//demo_face_detection_test();

	return 0;
}