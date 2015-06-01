#ifndef _NS_IMAGE_TYPE_H_
#define _NS_IMAGE_TYPE_H_

//////////////////////////////////////////////////////////////////////////
//	Definition for NS_RESULT

typedef enum NS_RESULT
{
	NS_S_OK				= 0,
	NS_S_FALSE				= -1,
	NS_E_FIRST				= 2031223845,
	
	NS_E_SECTION0			= NS_E_FIRST,
	NS_E_NORESULT,			// no result available
	NS_E_FAIL,				// general operation fail
	NS_E_NULL,				// unexpected NULL pointer
	NS_E_PARAM,			// invalid parameters
	NS_E_MEMORY,			// fail to alloc memory
	NS_E_BUFFER,			// insufficient input buffer
	NS_E_FORMAT,			// format error
	NS_E_QUOTA,
	NS_E_NOTREADY,
	NS_E_INUSE,
	NS_E_DECRYPT,
	NS_E_EXPIRE,
	NS_E_PERMIT,
	NS_E_NOTFOUND,
	NS_E_UNSUPPORT,
	NS_E_LICENSE,
	
	NS_E_SECTION1			= NS_E_SECTION0 + 32,
	NS_E_PACKAGE_STATUS,
	
	NS_E_SECTION2			= NS_E_SECTION1 + 16,
	NS_E_STORAGE_OPEN,
	NS_E_STORAGE_READ,
	NS_E_STORAGE_WRITE,
	NS_E_STORAGE_DELETE,
	
	NS_E_SECTION3			= NS_E_SECTION2 + 16,
	NS_E_COURIER_ADDRESS,
	NS_E_COURIER_CREATE,
	NS_E_COURIER_BIND,
	NS_E_COURIER_LISTEN,
	NS_E_COURIER_CONNECT,
	NS_E_COURIER_ACCEPT,
	NS_E_COURIER_OPTION,

	NS_E_SECTION4			= NS_E_SECTION3 + 16,
	NS_E_SERVICE_SCM,
	NS_E_SERVICE_OPEN,
	NS_E_SERVICE_PARAM,

	NS_E_SECTION5			= NS_E_SECTION4 + 16,
	NS_E_MISSION_ROLE,

	NS_E_SECTION6			= NS_E_SECTION5 + 16,
	NS_E_SECTION7			= NS_E_SECTION6 + 16,
	NS_E_SECTION8			= NS_E_SECTION7 + 16,
	NS_E_SECTION9			= NS_E_SECTION8 + 16,
	
	NS_E_LAST,

} NS_RESULT;

#define NS_TRUE(x)	((x) == NS_S_OK)
#define NS_SUCC(x)	((x) <= NS_S_OK)
#define NS_FAIL(x)	((x) >  NS_S_OK)

//////////////////////////////////////////////////////////////////////////
//	Some general definition

#define NS_MIN(a, b) ((a) > (b) ? (b) : (a))
#define NS_MAX(a, b) ((a) > (b) ? (a) : (b))

#endif	//_NS_IMAGE_TYPE_H_

