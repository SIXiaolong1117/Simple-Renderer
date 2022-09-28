#include <vector>
#include<iostream>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

// 定义颜色常量
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
// 设定模型
Model *model = NULL;
// 设定宽高
const int width = 800;
const int height = 800;

// 画线算法
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
	// 是否转置（记得转回去）
	bool steep = false;
	// 如果图像“陡峭”，对图像进行转置
	if (std::abs(x0 - x1) < std::abs(y0 - y1))
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	// 当 x0 > x1，对二者进行交换
	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	// 从循环中取出。注意这里用的是int类型，然后通过一个derror和error来实现浮点数。
	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = y0;
	// 一样的遍历算法，不多赘述
	for (int x = x0; x <= x1; x++)
	{
		if (steep)
		{
			// 如果转置了，记得转置回来（其实就是x和y反着set）
			image.set(y, x, color);
		}
		else
		{
			image.set(x, y, color);
		}
		error2 += derror2;
		if (error2 > dx)
		{
			y += (y1 > y0 ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}

// 主函数
int main(int argc, char **argv)
{
	// 导入模型
	if (2 == argc)
	{
		model = new Model(argv[1]);
	}
	else
	{
		model = new Model("obj/bun_zipper_res4.obj");
	}
	// 设定“画板”大小
	TGAImage image(width, height, TGAImage::RGB);
	// 绘制线框
	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		for (int j = 0; j < 3; j++)
		{
			// std::cout<<face[j]<<std::endl;
			// std::cout<<face[(j + 1) % 3]<<std::endl;
			Vec3f v0 = model->vert(face[j]);
			Vec3f v1 = model->vert(face[(j + 1) % 3]);
			int x0 = (v0.x + 1.) * width / 2.;
			int y0 = (v0.y + 1.) * height / 2.;
			int x1 = (v1.x + 1.) * width / 2.;
			int y1 = (v1.y + 1.) * height / 2.;
			line(x0, y0, x1, y1, image, white);
		}
	}
	// 原点在图像的左下角。
	image.flip_vertically();
	// 设置输出文件名
	image.write_tga_file("output.tga");
	return 0;
}
