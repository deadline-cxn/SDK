
#include "ximage.h"
#include <ostream.h>

#pragma comment(lib,"cximage.lib" )
#pragma comment(lib,"Tiff.lib" )
#pragma comment(lib,"jasper.lib" )
#pragma comment(lib,"libdcr.lib" )
#pragma comment(lib,"Jpeg.lib" )
#pragma comment(lib,"mng.lib" )
#pragma comment(lib,"png.lib" )
#pragma comment(lib,"zlib.lib" )

int main(int argc, char* argv[])
{
		
		char hfar[1024]; memset(hfar,0,1024);
		CxImage  image;

		if(argc>1)
		{
			sprintf(hfar,"%s.bmp",argv[1]);
			cout << "Converting TGA [" << argv[1] << "] 2 BMP [" << hfar << "\n";

			image.Load(argv[1],CXIMAGE_FORMAT_TGA);

			if (image.IsValid()){
				if(!image.IsGrayScale()) image.IncreaseBpp(24);							
				image.Save(hfar,CXIMAGE_FORMAT_BMP);
			}
		

		}
		else
		{
			cout << "TGA file needed to be able to convert!\n";
		}

	return 0;
}



