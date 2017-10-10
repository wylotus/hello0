#include <stdio.h>
#include <stdlib.h>

#include "png.h"
#include "zlib.h"

#include "GL/freeglut.h"

unsigned char* buffer = NULL;
png_uint_32 width, height, color_type;


static T_PicFileParser g_tPNGParser = {
	.name           = "png",
	.isSupport      = isPNGFormat,
	.GetPixelDatas  = GetPixelDatasFrmPNG,
	.FreePixelDatas = FreePixelDatasForPNG,	
};

#define PNG_BYTES_TO_CHECK 4
static int isPNGFormat(char *file_name, FILE **fp)
{
   char buf[PNG_BYTES_TO_CHECK];

   /* Open the prospective PNG file. */
   if ((*fp = fopen(file_name, "rb")) == NULL)
      return 0;

   /* Read in some of the signature bytes */
   if (fread(buf, 1, PNG_BYTES_TO_CHECK, *fp) != PNG_BYTES_TO_CHECK)
      return 0;

   /* Compare the first PNG_BYTES_TO_CHECK bytes of the signature.
      Return nonzero (true) if they match */

   return(!png_sig_cmp(buf, (png_size_t)0, PNG_BYTES_TO_CHECK));
}


//获取每一行所用的字节数，需要凑足4的倍数
int getRowBytes(int width){
	//刚好是4的倍数
	if((width * 3) % 4 == 0){
		return width * 3;
	}else{
		return ((width * 3) / 4 + 1) * 4;
	}
}

//显示图片
void myDisplay() {
	glClear(GL_COLOR_BUFFER_BIT);
	//图片是否有透明度
	if (color_type == PNG_COLOR_TYPE_RGB) {
		glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	} else if (color_type == PNG_COLOR_TYPE_RGBA) {
		glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	}
	glFlush();
}

int main(int c, char** v) {
	png_structp png_ptr;
	png_infop info_ptr;
	int bit_depth;
	FILE *fp;

	printf("lpng[%s], zlib[%s]\n", PNG_LIBPNG_VER_STRING, ZLIB_VERSION);

	if ((fp = fopen("testrgb.png", "rb")) == NULL) {
		return EXIT_FAILURE;
	}
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
	{
		fclose(fp);
		return EXIT_FAILURE;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return EXIT_FAILURE;
	}
	if (setjmp(png_jmpbuf(png_ptr))) {
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		/* If we get here, we had a problem reading the file */
		return EXIT_FAILURE;
	}
	/* Set up the input control if you are using standard C streams */
	png_init_io(png_ptr, fp);
	//读取png文件
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);
	//获取png图片相关信息
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
			NULL, NULL, NULL);
	printf("width[%d], height[%d], bit_depth[%d], color_type[%d]\n",
			width, height, bit_depth, color_type);

	//获得所有png数据
	png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);
	//计算buffer大小
	unsigned int bufSize = 0;
	if (color_type == PNG_COLOR_TYPE_RGB) {
		bufSize = getRowBytes(width) * height;
	} else if (color_type == PNG_COLOR_TYPE_RGBA) {
		bufSize = width * height * 4;
	} else {
		return EXIT_FAILURE;
	}
	//申请堆空间
	buffer = (unsigned char*) malloc(bufSize);
	int i;
	for (i = 0; i < height; i++) {
		//拷贝每行的数据到buffer，
		//opengl原点在下方，拷贝时要倒置一下
		if(color_type == PNG_COLOR_TYPE_RGB){
			memcpy(buffer + getRowBytes(width) * i, row_pointers[height - i - 1], width * 3);
		}else if(color_type == PNG_COLOR_TYPE_RGBA){
			memcpy(buffer + i * width * 4, row_pointers[height - i - 1], width * 4);
		}
	}
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	fclose(fp);

	glutInit(&c, v);
	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("hello png");
	glutDisplayFunc(&myDisplay);
	glutMainLoop();
	return 0;
}

