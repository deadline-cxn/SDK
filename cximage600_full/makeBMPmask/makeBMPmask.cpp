
#include "ximage.h"
#include <ostream.h>
#include <vector>

#pragma comment(lib,"cximage.lib" )
#pragma comment(lib,"Tiff.lib" )
#pragma comment(lib,"jasper.lib" )
#pragma comment(lib,"libdcr.lib" )
#pragma comment(lib,"Jpeg.lib" )
#pragma comment(lib,"mng.lib" )
#pragma comment(lib,"png.lib" )
#pragma comment(lib,"zlib.lib" )


std::vector <std::string> explode(const std::string &delimiter, const std::string &str)
{
	std::vector <std::string> arr;
	int strleng=str.length();
	int delleng=delimiter.length();
	if(delleng==0)
		return arr;
	int i=0;
	int k=0;
	while(i<strleng)
	{
		int j=0;
		while(i+j<strleng && j<delleng && str[i+j]==delimiter[j])
			j++;
		if(j==delleng)
		{
			arr.push_back( str.substr(k,i-k) );
			i+=delleng;
			k=i;
		}
		else
		{
			i++;
		}

	}
	arr.push_back( str.substr(k,i-k) );
	return arr;

}

int main(int argc, char* argv[])
{
		
		std::vector <std::string> vs;

		bool fillblack=false;
		bool ignoreblack=false;
		bool keybottomright=false;

		char hfar[1024]; memset(hfar,0,1024);
		char jfar[1024]; memset(jfar,0,1024);
		CxImage  image;

		if(argc>1)
		{
			vs=explode(".",argv[1]);

			if(argc>2)
			{
				if(!strcmp(argv[2],"-fb"))
				{
					fillblack=true;
				}
				if(!strcmp(argv[2],"-itlb"))
				{
					ignoreblack=true;
				}
				if(!strcmp(argv[2],"-kbr"))
				{
					keybottomright=true;
				}


			}
			
			sprintf(hfar,"%smask.bmp",(char *)vs[0].c_str());
			sprintf(jfar,"%s.bmp",(char *)vs[0].c_str());


			cout << "Converting BMP [" << argv[1] << "] 2 BMP [" << hfar << "]\n";

			image.Load(argv[1],CXIMAGE_FORMAT_BMP);

			if (image.IsValid())
			{
			
				image.IncreaseBpp(24);
				image.Save(jfar,CXIMAGE_FORMAT_BMP);

				
				RGBQUAD tl;
				RGBQUAD rgbq;
				int x,y;

				tl=image.GetPixelColor(0,0,1);

				if(keybottomright)
					tl=image.GetPixelColor(image.GetWidth(),image.GetHeight(),1);


				if( ignoreblack==false )
					if( (tl.rgbBlue==0) &&
						(tl.rgbRed==0) &&
						(tl.rgbGreen==0) )
							fillblack=true;
				

				for(x=0;x<image.GetWidth();x++)
				{
					for(y=0;y<image.GetHeight();y++)
					{
						rgbq=image.GetPixelColor(x,y,1);

						if(fillblack==true)
						{
							image.SetPixelColor(x,y,0);

						}
						else
						{


							if( (rgbq.rgbBlue	== tl.rgbBlue) &&
								(rgbq.rgbRed	== tl.rgbRed) &&
								(rgbq.rgbGreen	== tl.rgbGreen) )
							{
								image.SetPixelColor(x,y,RGB(255,255,255));
							
							}
							else
							{
								image.SetPixelColor(x,y,0);
									
							}
						}
					}

				}
					
				
				
				image.Save(hfar,CXIMAGE_FORMAT_BMP);
			}
		

		}
		else
		{
			cout << "BMP file needed to be able to convert!\n";
		}

	return 0;
}



