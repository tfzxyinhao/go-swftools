typedef enum tagErrorCode
{
	NOERROR,
	PARAM_ERROR,
	FILE_NOT_EXISTS,
	FILE_OPEN_ERROR,
	RANGE_ERROR,
	SAVE_ERROR
}ErrorCode;

ErrorCode convert(const char* filename, const char* page_range, const char* outputname, const char* password, const char* langDir);
