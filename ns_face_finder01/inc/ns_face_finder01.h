#ifndef	_NS_FACE_FINDER01_H_
#define _NS_FACE_FINDER01_H_

#include "ns_face_type.h"


typedef void* NS_FACE_FINDER01_HANDLE;

#if defined(__cplusplus)
extern "C"
{
#endif

	int ns_face_finder01_init(NS_FACE_FINDER01_HANDLE *handle, char *data_path, char *message);

	int ns_face_finder01_destroy(NS_FACE_FINDER01_HANDLE *handle, char *message);

	int ns_face_finder01(NS_FACE_FINDER01_HANDLE handle, NSF_IMAGE *image, NSF_FD_PARAM *fd_param, NSF_FACE *face, int max_face_num, int *face_num, char *message);


#if defined(__cplusplus)
}
#endif


#endif	/* _NS_FACE_FINDER01_H_ */