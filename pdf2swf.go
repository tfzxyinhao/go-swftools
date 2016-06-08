package pdf2swf

/*
#cgo CFLAGS: -I/usr/include
#cgo LDFLAGS:-L/lib64 -L/usr/lib64 -lpdf2swf
#include <libpdf2swf.h>
*/
import (
	"C"
	"log"
	"os"
	"path"
)

/*
   pdf : pdf  文件的路径
   output : 输出swf的路径,例如 /home/data/name%.swf
            如果输出路径包含%则忽略pages参数生成每一页的swf
   pages : 1-20 或者 1,4,6,9-11,空字符串表示生成每一页
   password : pdf的密码,没有时传空字符串
*/
func ToSWF(pdf, output, pages, password string) int {
	dir, err = os.Getwd()
	if err != nil {
		return 0
	} else {
		log.Println("current dir:", dir)
		return int(C.convert(C.CString(pdf), C.CString(pages), C.CString(output), C.CString(password)), C.CString(path.Join(dir, "lang")))
	}
}
