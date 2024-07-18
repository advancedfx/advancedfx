typedef unsigned char FFIBool;

#define FFIBOOL_FALSE 0
#define FFIBOOL_TRUE 1

#define BOOL_TO_FFIBOOL(value) ((FFIBool)(value?FFIBOOL_TRUE:FFIBOOL_FALSE))
#define FFIBOOL_TO_BOOL(value) ((bool)value)
