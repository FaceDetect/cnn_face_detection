#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ns_face_finder01.h"
#include "cnn.h"

#define MAX(a,b) ( ((a)>(b)) ? (a):(b) )
#define MIN(a,b) ( ((a)>(b)) ? (b):(a) )

// test parameters
#define NET01_SIZE 12
#define NET02_SIZE 24
#define NET_PROB 5
//#define SCALE_NUM 15
#define SCALE_STEP 0.9
#define NET01_STEP 2
#define NET01_THR 0.9
#define NET02_THR 0.5  
#define NMS_NET01 0.5
#define NMS_NET02 0.1

//debug
#define DEBUG_CASCADE 1

// bootstrap
//#define NET01_SIZE 12
//#define NET02_SIZE 24
//#define NET_PROB 5
//#define SCALE_NUM 10
//#define NET01_STEP 2
//#define NET01_THR 0.4
//#define NET02_THR 0.4  
//#define NMS_NET01 0.9
//#define NMS_NET02 0.95


typedef struct _ns_face_finder01_handle_struct_
{
	Net *detection_net_12;
	Net *detection_net_24;

}NS_FACE_FINDER01_HANDLE_STRUCT;

// image resize
int ns_image_resize(NSF_IMAGE *original, unsigned char *temp, NSF_IMAGE *output);

// image crop
int ns_image_crop(NSF_IMAGE *image, unsigned char *patch, NSF_IMAGE *crop_image, NSF_FACE *box);

//face detector
int ns_face_detector(NSF_IMAGE *image, Net *detection_net, float *patch, NSF_FACE *box_in, int *face_num_in, 
	NSF_FACE *box_out, int *face_num_out, int net_size, int scan_step, float thr_classifier);

// mapping bounding box to original image
int ns_face_box_map(NSF_FACE *box, int face_num, float scale);

// nms
int ns_sort(NSF_FACE *box, int *index, int n);
int ns_delete_index(int *index, int *idx_count, int n);
int ns_face_nms(NSF_FACE *box_in, int face_num_in, NSF_FACE *box_out,  int *face_num_out, float thr_nms);


int ns_face_finder01_init(NS_FACE_FINDER01_HANDLE *handle, char *data_path, char *message)
{
	char detection_net_12_data[256];
	char detection_net_24_data[256];
	int ret_net_12;
	int ret_net_24;

	sprintf(detection_net_12_data, "%s\\12_detection.bin", data_path);
	sprintf(detection_net_24_data, "%s\\24_detection.bin", data_path);

	NS_FACE_FINDER01_HANDLE_STRUCT *face_finder01_handle;
	face_finder01_handle = (NS_FACE_FINDER01_HANDLE_STRUCT *)malloc(sizeof(NS_FACE_FINDER01_HANDLE_STRUCT));
	memset(face_finder01_handle, 0, sizeof(NS_FACE_FINDER01_HANDLE_STRUCT));

	face_finder01_handle->detection_net_12 = new Net();
	ret_net_12 = face_finder01_handle->detection_net_12->LoadFromFile(detection_net_12_data);
	face_finder01_handle->detection_net_24 = new Net();
	ret_net_24 = face_finder01_handle->detection_net_24->LoadFromFile(detection_net_24_data);

	if(0 == ret_net_12 && 0 == ret_net_24)
	{
		(*handle) = (NS_FACE_FINDER01_HANDLE) face_finder01_handle;
	}
	else
	{
		if(ret_net_12 != 0 && ret_net_24 != 0)
			sprintf(message, "Both 12_detection_net and 24_detection_net model init failed: %d, %d.\n", ret_net_12, ret_net_24);
		else if(ret_net_12 != 0)
			sprintf(message, "12_detection_net model init failed: %d.\n", ret_net_12);
		else
			sprintf(message, "24_detection_net model init failed: %d. \n", ret_net_24);
		(*handle) = NULL;
		return -1;
	}

	return 0;
}

int ns_face_finder01_destroy(NS_FACE_FINDER01_HANDLE *handle, char *message)
{
	NS_FACE_FINDER01_HANDLE_STRUCT *face_finder01_handle = (NS_FACE_FINDER01_HANDLE_STRUCT *) (*handle);

	if(NULL != face_finder01_handle->detection_net_12)
		delete face_finder01_handle->detection_net_12;
	if(NULL != face_finder01_handle->detection_net_24)
		delete face_finder01_handle->detection_net_24;

	free(face_finder01_handle);
	(*handle) = NULL;

	return 0;
}

int ns_face_finder01(NS_FACE_FINDER01_HANDLE handle, NSF_IMAGE *image, NSF_FD_PARAM *fd_param, NSF_FACE *face, int max_face_num, int *face_num, char *message)
{
	NS_FACE_FINDER01_HANDLE_STRUCT *face_finder01_handle = (NS_FACE_FINDER01_HANDLE_STRUCT *) handle;

	if(NULL == face_finder01_handle->detection_net_12 || NULL == face_finder01_handle->detection_net_24)
	{
		sprintf(message, "NULL 12_detection_net model or NULL 24_detection_net model: %d, %d.\n", face_finder01_handle->detection_net_12, face_finder01_handle->detection_net_24);
		return -1;
	}

	if(fd_param == NULL || fd_param->min_face_size <= 0)
	{
		message = "Face detection parameters loading failed.\n";
		return -2;
	}

	int face_num_net01 = 0;
	int face_num_net02 = 0;
	int temp_num = 0;
	float max_scale = NET01_SIZE / float(fd_param->min_face_size);
	float min_scale = NET01_SIZE / float(fd_param->max_face_size);
	//float scale_step = (max_scale - min_scale) / SCALE_NUM ;

	float *img_patch_net01, *img_patch_net02;
	if((img_patch_net01= (float *)malloc(sizeof(float)*NET01_SIZE*NET01_SIZE)) == NULL)
	{
		message = "malloc memory error in img_patch_net01.\n";
		return -3;
	}
	if((img_patch_net02 = (float *)malloc(sizeof(float)*NET02_SIZE*NET02_SIZE)) == NULL)
	{
		message = "malloc memory error in img_patch_net03.\n";
		return -3;
	}
	
	NSF_FACE *face_net01_scale, *face_net01, *face_net02;
	if((face_net01_scale= (NSF_FACE *)malloc(sizeof(NSF_FACE)*1000000)) == NULL)
	{
		message = "malloc memory error in face_net01_scale.\n";
		return -3;
	}
	if((face_net01= (NSF_FACE *)malloc(sizeof(NSF_FACE)*1000000)) == NULL)
	{
		message = "malloc memory error in face_net01_scale.\n";
		return -3;
	}
	if((face_net02= (NSF_FACE *)malloc(sizeof(NSF_FACE)*10000)) == NULL)
	{
		message = "malloc memory error in face_net02.\n";
		return -3;
	}

	NSF_IMAGE *image_scale = (NSF_IMAGE *)malloc(sizeof(NSF_IMAGE));
	

	/* multi-scale scanning via 12_detection_net */
	//for (float scale = min_scale; scale < max_scale; scale = scale + scale_step)
	for (float scale = max_scale; scale >= min_scale; scale = scale*SCALE_STEP)
	{
		int target_width = (int)(image->width * scale);
		int target_height = (int)(image->height * scale);
		image_scale->width = target_width;
		image_scale->height = target_height;
		image_scale->type = image->type;
		image_scale->format = image->format;
		image_scale->data_len = target_height * target_width;
		int step = ceil(NET01_STEP * scale / max_scale);
		unsigned char *resize_patch = (unsigned char*)malloc(sizeof(unsigned char)*image_scale->data_len);

		// image resize
		ns_image_resize(image, resize_patch, image_scale);
		// detector
		ns_face_detector(image_scale, face_finder01_handle->detection_net_12, img_patch_net01, NULL, NULL, face_net01_scale, &face_num_net01, NET01_SIZE, step, NET01_THR);

		// mapping
		if(face_num_net01 > 0)
		{
			ns_face_box_map(face_net01_scale, face_num_net01, scale);
		}
		

		for(int i = 0; i < face_num_net01; i++ )
		{
			(face_net01 + temp_num)->xtl = (face_net01_scale + i)->xtl;
			(face_net01 + temp_num)->xbr = (face_net01_scale + i)->xbr;
			(face_net01 + temp_num)->ytl = (face_net01_scale + i)->ytl;
			(face_net01 + temp_num)->ybr = (face_net01_scale + i)->ybr;
			(face_net01 + temp_num)->confidence = (face_net01_scale + i)->confidence;
			temp_num ++;
		}

		free(resize_patch);
		resize_patch = NULL;
	}
	if(temp_num != 0)
		face_num_net01 = temp_num - 1;
	else
		face_num_net01 = 0;

	if(NULL == face_net01)
	{
		message = "Face detector error!\n";
		return -4;
	}


#ifdef DEBUG_CASCADE
	ns_face_nms(face_net01, face_num_net01, face, face_num, NMS_NET01);
	printf("Only Test 12 detection net! \n");
#endif

#ifndef DEBUG_CASCADE
	/* 12 detection net results NMS*/
	ns_face_nms(face_net01, face_num_net01, face_net01, &face_num_net01, NMS_NET01);

	/* 24 detection net */
	ns_face_detector(image, face_finder01_handle->detection_net_24, img_patch_net02, face_net01, &face_num_net01, face_net02, &face_num_net02, NET02_SIZE, 0, NET02_THR);

	/* 24 detection net results NMS*/
	ns_face_nms(face_net02, face_num_net02, face, face_num, NMS_NET02);
#endif
	


	free(img_patch_net01);
	free(img_patch_net02);
	free(face_net01_scale);
	free(face_net01);
	free(face_net02);
	free(image_scale);
	
	img_patch_net01 = NULL;
	img_patch_net02 = NULL;
	face_net01_scale = NULL;
	face_net01 = NULL;
	face_net02 = NULL;
	image_scale = NULL;
	

	return 0;
}

int ns_image_resize(NSF_IMAGE *original, unsigned char *temp, NSF_IMAGE *output)
{
	unsigned char a, b, c, d;
	int x, y, index;
	float x_diff, y_diff, gray;
	int source_width = original->width;
	int source_height = original->height;
	int target_width = output->width;
	int target_height = output->height;
	float x_ratio = ((float) (source_width - 1)) / target_width;
	float y_ratio = ((float) (source_height - 1)) / target_height;

	for(int i = 0; i < target_height; i++)
	{
		for(int j = 0; j < target_width; j++)
		{
			x = (int) ( x_ratio * j );
			y = (int) ( y_ratio * i);
			x_diff = ( x_ratio * j ) - x;
			y_diff = ( y_ratio * i ) - y;

			index = (y * source_width + x);

			a = *(original->data +index);
			b = *(original->data +index + 1);
			c = *(original->data +index +source_width);
			d = *(original->data +index + source_width +1);

			gray = a*(1-x_diff)*(1-y_diff) + b*(x_diff)*(1-y_diff) +
				c*(y_diff)*(1-x_diff)   + d*(x_diff*y_diff);

			*(temp + i*target_width +j) = (unsigned char) gray;
		}
	}
	output->data = temp;

	return 0;
}

int ns_image_crop(NSF_IMAGE *image, unsigned char *patch, NSF_IMAGE *crop_image, NSF_FACE *box)
{
	int xtl, ytl, xbr, ybr;
	int w, h;
	int index;

	xtl = box->xtl;
	ytl = box->ytl;
	xbr = box->xbr;
	ybr = box->ybr;
	w = xbr - xtl + 1;
	h = ybr - ytl + 1;

	crop_image->height = h;
	crop_image->width = w;
	crop_image->data_len = w*h;
	crop_image->format = image->format;
	crop_image->type = image->type;

	for(int i = 0; i < h; i++)
	{
		index = (ytl + i)*image->width;
		memcpy(patch+i*w, image->data + xtl + index, sizeof(unsigned char)*w);
	}

	crop_image->data = patch;

#ifndef FALSE
	FILE *fp;
	fp = fopen("crop_image.txt", "w+");
	fprintf(fp, "%d %d\n", w, h);
	for(int i = 0; i < w*h; i++)
	{
		fprintf(fp, "%d ", *(patch+i));
	}
	fclose(fp);
#endif

	return 0;
}

int ns_face_detector(NSF_IMAGE *image, Net *detection_net, float *patch, NSF_FACE *box_in, int *face_num_in, 
	NSF_FACE *box_out, int *face_num_out, int net_size, int scan_step, float thr_classifier)
{

	int width = image->width;
	int height = image->height;
	float *data = (float *)malloc(sizeof(float)*width*height);
	Blob *prob = new Blob();
	float *temp = NULL;
	int num = 0;

	// convert unsigned char to float
	for(int i = 0; i < height * width; i ++)
	{
		data[i] = static_cast<float>(image->data[i])/255.0;
	}

	// detection
	if(box_in == NULL)
	{
		int index = 0;
		for(int j = 0; j < width - net_size; j = j + scan_step )
		{
			for(int i = 0; i < height - net_size; i = i + scan_step)
			{
				for(int k = 0; k < net_size; k ++)
				{
					index = (i + k)*width;
					memcpy(patch+k*net_size, data + j + index, sizeof(float)*net_size);
				}

				detection_net->TakeInput(patch, net_size, net_size, 1);
				detection_net->Forward();
				prob = detection_net->get_blob(NET_PROB);
				temp = (float *)(prob->data + 1);
				if(*temp > thr_classifier)
				{
					(box_out + num)->confidence = *temp;
					(box_out + num)->xtl = j;
					(box_out + num)->ytl = i;
					(box_out + num)->xbr = j + net_size - 1;
					(box_out + num)->ybr = i + net_size - 1;
#ifndef FALSE
					FILE *fp;
					char *filename = (char *)malloc(sizeof(char)*256);
					sprintf(filename, "D:\\output\\patch_net01_%d.txt", num);
					fp = fopen(filename, "w+");
					for(int i = 0; i < 144; i++)
					{
						fprintf(fp, "%f ", patch[i]);
					}
					fclose(fp);
#endif
					num = num + 1;
				}
			}
		}
	}
	else
	{
		NSF_IMAGE *crop_image = (NSF_IMAGE*)malloc(sizeof(NSF_IMAGE));
		NSF_IMAGE *resize_image = (NSF_IMAGE*)malloc(sizeof(NSF_IMAGE));
		unsigned char *crop_patch = NULL;
		crop_patch = (unsigned char *)malloc(sizeof(unsigned char)*width*height);
		unsigned char *resize_patch = NULL;
		resize_patch = (unsigned char *)malloc(sizeof(unsigned char)*NET02_SIZE*NET02_SIZE);

		for(int n = 0; n < *face_num_in; n++)
		{
			int xtl = (box_in + n) -> xtl;
			int ytl = (box_in + n) -> ytl;
			ns_image_crop(image, crop_patch, crop_image, box_in +n);

			resize_image->width = NET02_SIZE;
			resize_image->height = NET02_SIZE;
			resize_image->data_len = resize_image->height*resize_image->height;

			ns_image_resize(crop_image, resize_patch, resize_image);

			for(int i = 0; i < resize_image->data_len; i++)
			{
				patch[i] = static_cast<float>(resize_image->data[i])/255.0;
			}

			detection_net->TakeInput(patch, net_size, net_size, 1);
			detection_net->Forward();
			prob = detection_net->get_blob(NET_PROB);
			temp = (float *)(prob->data + 1);
			if(*temp > thr_classifier)
			{
				(box_out + num)->confidence = *temp;
				(box_out + num)->xtl = xtl;
				(box_out + num)->ytl = ytl;
				(box_out + num)->xbr = (box_in + n)->xbr;
				(box_out + num)->ybr =  (box_in + n)->ybr;

#ifndef FALSE
				FILE *fp;
				char *filename = (char *)malloc(sizeof(char)*256);
				sprintf(filename, "D:\\output\\patch_net02_crop_%d.txt", num);

				fp = fopen(filename, "w+");			
				fprintf(fp, "%d %d\n", crop_image->width, crop_image->height);
				for(int i = 0; i < crop_image->data_len; i++)
				{
					fprintf(fp, "%d ", crop_image->data[i]);
				}
				fclose(fp);

				sprintf(filename, "D:\\output\\patch_net02_%d.txt", num);
				fp = fopen(filename, "w+");
				for(int i = 0; i < resize_image->data_len; i++)
				{
					fprintf(fp, "%f ", patch[i]);
				}
				fclose(fp);
#endif
				num = num + 1;
			}

			
		}
		free(crop_patch);
		free(resize_patch);
		free(crop_image);
		free(resize_image);
		crop_image = NULL;
		resize_image = NULL;
		crop_patch = NULL;
		resize_patch = NULL;
	}

	*face_num_out = num;

	free(data);
	data = NULL;

	return 0;
}

int ns_face_box_map(NSF_FACE *box, int face_num, float scale)
{
	int xtl, ytl, xbr, ybr;
	int w, h;

	for(int i = 0; i < face_num; i++)
	{

		xtl = (box + i)->xtl ;
		xbr = (box + i)->xbr ;
		ytl = (box + i)->ytl;
		ybr = (box + i)->ybr;

		w = (xbr - xtl + 1) / scale;
		h = (ybr - ytl + 1) / scale;

		(box+i)->xtl = xtl / scale;
		(box+i)->ytl = ytl / scale;
		(box+i)->xbr = xtl / scale + w - 1;
		(box+i)->ybr = ytl / scale + h - 1;
	}

	return 0;
}

int ns_sort(NSF_FACE *box, int *index, int n)
{
	int ti, tj;
	for(int i = 0; i < n; i++)
	{
		for(int j = i+1; j < n; j++)
		{
			ti = *(index + i);
			tj = *(index + j);
			if((box + tj)->confidence < (box + ti)->confidence)
			{
				*(index+i) = tj;
				*(index+j) = ti;
			}
		}
	}

	return 0;
}

int ns_delete_index(int *index, int *idx_count, int n)
{
	int i = 0, j = 0;

	while(i < n)
	{
		if(index[i] == -1)
		{
			if(j < i +1)
				j = i + 1;
			while (j < n)
			{
				if(index[j] == -1)
					j++;
				else
				{
					index[i] = index[j];
					index[j] = -1;
					j++;
					break;
				}
			}
			if(j == n)
				break;
		}
		i++;
	}
	*idx_count = i;
	return 0;
}

// NMS algorithm
int ns_face_nms(NSF_FACE *box_in, int face_num_in, NSF_FACE *box_out,  int *face_num_out, float thr_nms)
{
	float *area = (float *)malloc(sizeof(float)*face_num_in);
	int xtl, ytl, xbr, ybr;
	int *index = (int *)malloc(sizeof(int)*face_num_in);
	int *pick = (int *)malloc(sizeof(int)*face_num_in);
	int idx_count = face_num_in;
	int n;
	int counter = 0;
	int xx1, xx2, yy1, yy2;
	int w, h;
	float overlap;


	// sort the bounding box 
	for(int i = 0; i < face_num_in; i++) 
	{
		*(index + i) = i;
	}
	ns_sort(box_in, index, face_num_in);

	// compute area per bounding box
	for(int i = 0; i < face_num_in; i++)
	{
		xtl = (box_in + i)->xtl;
		ytl = (box_in + i)->ytl;
		xbr = (box_in + i)->xbr;
		ybr = (box_in + i)->ybr;
		*(area+i) = (xbr - xtl + 1)*(ybr - ytl + 1);
	}

	// nms
	while(idx_count > 0)
	{
		int tmp_count = idx_count - 1;
		int last_idx = *(index + tmp_count);
		*(pick + counter) = last_idx;
		counter = counter + 1;

		xtl = (box_in + last_idx)->xtl;
		ytl = (box_in + last_idx)->ytl;
		xbr = (box_in + last_idx)->xbr;
		ybr = (box_in + last_idx)->ybr;

		*(index + tmp_count) = -1;

		for(int i = tmp_count - 1; i != -1; i--)
		{
			xx1 = MAX(xtl, ((box_in+index[i])->xtl));
			xx2 = MIN(xbr,  ((box_in+index[i])->xbr));
			yy1 = MAX(ytl,  ((box_in+index[i])->ytl));
			yy2 = MIN(ybr,  ((box_in+index[i])->ybr));

			w = MAX(0, (xx2 - xx1 +1));
			h = MAX(0, (yy2 - yy1 + 1));

			//int tmp_area = MIN(area[index[i]], area[last_idx]);
			int tmp_area = area[index[i]];
			overlap = (float)(w*h) / (float)tmp_area;

			if(overlap > thr_nms)
			{
				*(index + i) = -1;
			}
		}
		n = idx_count;
		ns_delete_index(index, &idx_count, n);
	}

	for(int i = 0; i < counter; i++)
	{
		(box_out + i)->xtl = (box_in + pick[i])->xtl;
		(box_out + i)->ytl = (box_in + pick[i])->ytl;
		(box_out + i)->xbr = (box_in + pick[i])->xbr;
		(box_out + i)->ybr = (box_in + pick[i])->ybr;
		(box_out + i)->confidence = (box_in + pick[i])->confidence;
	}

	*face_num_out = counter;

	free(area);
	free(index);
	free(pick);
	area = NULL;
	index = NULL;
	pick = NULL;

	return 0;
}