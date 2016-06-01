enum ErrorCode
{
	NOERROR,
	PARAM_ERROR,
	FILE_NOT_EXISTS,
	FILE_OPEN_ERROR,
	RANGE_ERROR,
	SAVE_ERROR
};

ErrorCode convert(const char* filename, int page, const char* out, const char* password = 0);
