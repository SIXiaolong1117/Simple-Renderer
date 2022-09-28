#include <vector>
#include <iostream>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

// 定义颜色常量
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
// 设定模型
Model *model = NULL;
// 设定宽高
const int width = 800;
const int height = 800;
// 光照方向
Vec3f light_dir(0, 0, -1);

// 画线算法
void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color)
{
	// 是否转置（记得转回去）
	bool steep = false;
	// 如果图像“陡峭”，对图像进行转置
	if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y))
	{
		std::swap(p0.x, p0.y);
		std::swap(p1.x, p1.y);
		steep = true;
	}
	// 当 x0 > x1，对二者进行交换
	if (p0.x > p1.x)
	{
		std::swap(p0, p1);
	}
	// 从循环中取出。注意这里用的是int类型，然后通过一个derror和error来实现浮点数。
	int dx = p1.x - p0.x;
	int dy = p1.y - p0.y;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = p0.y;
	// 一样的遍历算法，不多赘述
	for (int x = p0.x; x <= p1.x; x++)
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
			y += (p1.y > p0.y ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}

// 三角形绘制算法
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color)
{
	// 将顶点t0、t1、t2从低到高排序
	if (t0.y > t1.y)
		std::swap(t0, t1);
	if (t0.y > t2.y)
		std::swap(t0, t2);
	if (t1.y > t2.y)
		std::swap(t1, t2);
	// 总高度
	int total_height = t2.y - t0.y;
	// 遍历整个三角形
	for (int i = 0; i < total_height; i++)
	{
		// 确定现在是上下哪部分
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		// 不同的部分用不同的公式计算局部高度
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;
		//小心除以0
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;
		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
		if (A.x > B.x)
			std::swap(A, B);
		for (int j = A.x; j <= B.x; j++)
		{
			// 注意，由于int casts t0.y+i != A.y
			image.set(j, t0.y + i, color);
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
		model = new Model("obj/bun_zipper.obj");
	}
	// 设定“画板”大小
	TGAImage image(width, height, TGAImage::RGB);
	// 绘制
	// 遍历所有面
	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		Vec2i screen_coords[3];
		Vec3f world_coords[3];
		// 遍历，将所有的顶点储存在screen_coords中，同时将三角形边的向量储存在world_coords中
		for (int j = 0; j < 3; j++)
		{
			Vec3f v = model->vert(face[j]);
			screen_coords[j] = Vec2i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.);
			world_coords[j] = v;
		}
		// 计算每一个三角形的法向量
		Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
		// 归一化法向量
		n.normalize();
		// 计算光线强度
		float intensity = n * light_dir;
		// 如果光线强度大于0
		if (intensity > 0)
		{
			// 依据光线强度，绘制填充三角形
			triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
		}
	}
	// 原点在图像的左下角。
	image.flip_vertically();
	// 设置输出文件名
	image.write_tga_file("output.tga");
	return 0;
}
