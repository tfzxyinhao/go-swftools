# go-swftools
golang binding of pdf2swf(swftools)

#Installation
git clone https://github.com/tfzxyinhao/go-swftools.git  
cd go-swftools  
chmod +x ./configure  
./configure  
make  
make install  
make installgo  

#dependencies
* freetype
* jpeglib

#notice
set the environment variable GOPATH before install 

#example

`package main`

`import ("pdf2swf")`

`func main(){`  
&nbsp;&nbsp;&nbsp;&nbsp;`pdf2swf.ToSWF("julia-cn.pdf","out%.swf","1-3","")`  
`}`
