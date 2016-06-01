package pdf2swf
/*
#cgo CFLAGS: -I/usr/include
#cgo LDFLAGS: -L/usr/lib -lpdf2swf
#include <libpdf2swf.h>
*/
import (
"C"
)
func ToSWF(str string){
 C.output(C.CString(str))
}
