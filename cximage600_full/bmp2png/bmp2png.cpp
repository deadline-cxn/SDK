#include "ximage.h"
#include <stdlib.h>
int main(int argc, char* argv[]){
		char hfar[1024]; memset(hfar,0,1024);
		CxImage  image;
		if(argc>1) {
			sprintf(hfar,"%s.bmp",argv[1]);
			printf("Converting BMP [%s] 2 PNG [%s]\n",argv[1],hfar );
			image.Load(argv[1],CXIMAGE_FORMAT_BMP);
			if (image.IsValid()){
				image.Save(hfar,CXIMAGE_FORMAT_PNG);
			}
		}
		else {
			printf("BMP file needed to be able to convert!\n");
		}
	return 0;
}



