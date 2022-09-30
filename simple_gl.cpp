#include <cmath>
#include <limits>
#include <cstdlib>
#include "simple_gl.h"

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

IShader::~IShader() {}

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

// 视口矩阵
void viewport(int x, int y, int w, int h)
{
    Viewport = Matrix::identity();
    Viewport[0][3] = x + w / 2.f;
    Viewport[1][3] = y + h / 2.f;
    Viewport[2][3] = 255.f / 2.f;
    Viewport[0][0] = w / 2.f;
    Viewport[1][1] = h / 2.f;
    Viewport[2][2] = 255.f / 2.f;
}

void projection(float coeff)
{
    Projection = Matrix::identity();
    Projection[3][2] = coeff;
}

// lookat函数
void lookat(Vec3f eye, Vec3f center, Vec3f up)
{
    Vec3f z = (eye - center).normalize();
    Vec3f x = cross(up, z).normalize();
    Vec3f y = cross(z, x).normalize();
    ModelView = Matrix::identity();
    for (int i = 0; i < 3; i++)
    {
        ModelView[0][i] = x[i];
        ModelView[1][i] = y[i];
        ModelView[2][i] = z[i];
        ModelView[i][3] = -center[i];
    }
}

// // 三角形绘制算法（扫线）
// void triangle(Vec3i t0, Vec3i t1, Vec3i t2, TGAImage &image, float intensity, int *zbuffer)
// {
// 	// 将顶点t0、t1、t2从低到高排序
// 	if (t0.y == t1.y && t0.y == t2.y)
// 		// 不关注退化三角形
// 		return;
// 	if (t0.y > t1.y)
// 	{
// 		std::swap(t0, t1);
// 	}
// 	if (t0.y > t2.y)
// 	{
// 		std::swap(t0, t2);
// 	}
// 	if (t1.y > t2.y)
// 	{
// 		std::swap(t1, t2);
// 	}
// 	// 总高度
// 	int total_height = t2.y - t0.y;
// 	// 遍历整个三角形
// 	for (int i = 0; i < total_height; i++)
// 	{
// 		// 确定现在是上下哪部分
// 		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
// 		// 不同的部分用不同的公式计算局部高度
// 		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
// 		float alpha = (float)i / total_height;
// 		//小心除以0
// 		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;
// 		Vec3i A = t0 + Vec3f(t2 - t0) * alpha;
// 		Vec3i B = second_half ? t1 + Vec3f(t2 - t1) * beta : t0 + Vec3f(t1 - t0) * beta;
// 		if (A.x > B.x)
// 		{
// 			std::swap(A, B);
// 		}
// 		for (int j = A.x; j <= B.x; j++)
// 		{
// 			float phi = B.x == A.x ? 1. : (float)(j - A.x) / (float)(B.x - A.x);
// 			Vec3i P = Vec3f(A) + Vec3f(B - A) * phi;
// 			int idx = P.x + P.y * width;
// 			if (zbuffer[idx] < P.z)
// 			{
// 				zbuffer[idx] = P.z;
// 				image.set(P.x, P.y, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
// 			}
// 		}
// 	}
// }

// // 二维光栅化（ybuffer）
// void rasterize(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int ybuffer[])
// {
// 	if (p0.x > p1.x)
// 	{
// 		std::swap(p0, p1);
// 	}
// 	// 遍历
// 	for (int x = p0.x; x <= p1.x; x++)
// 	{
// 		float t = (x - p0.x) / (float)(p1.x - p0.x);
// 		int y = p0.y * (1. - t) + p1.y * t;
// 		// 判断ybuffer的大小，如果小于当前y
// 		if (ybuffer[x] < y)
// 		{
// 			// 当前y设为ybuffer
// 			ybuffer[x] = y;
// 			// 绘制像素
// 			image.set(x, 0, color);
// 		}
// 		// 否则，不绘制
// 	}
// }

// 重心坐标
Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P)
{
    // 封装向量
    Vec3f s[2];
    for (int i = 2; i--;)
    {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    // 计算叉积
    Vec3f u = cross(s[0], s[1]);
    // `pts`和`P`有整数值作为坐标，所以`abs(u[2])`<1意味着`u[2]`是0，这意味着三角形是退化的，在这种情况下，返回负坐标的东西。
    if (std::abs(u[2]) > 1e-2)
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    // 在这种情况下产生负坐标，它将被光栅化器扔掉。
    return Vec3f(-1, 1, 1);
}

// 三角形光栅化（重心坐标）
void triangle(Vec4f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer)
{
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    // 迭代三角形，寻找最小/最大坐标
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
        }
    }
    Vec2i P;
    TGAColor color;
    // 遍历，找出三角形内的点。
    // 遍历x
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        // 遍历y
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            // 重心坐标
            Vec3f c = barycentric(proj<2>(pts[0] / pts[0][3]), proj<2>(pts[1] / pts[1][3]), proj<2>(pts[2] / pts[2][3]), proj<2>(P));
            float z = pts[0][2] * c.x + pts[1][2] * c.y + pts[2][2] * c.z;
            float w = pts[0][3] * c.x + pts[1][3] * c.y + pts[2][3] * c.z;
            int frag_depth = std::max(0, std::min(255, int(z / w + .5)));
            // 是否在边界内
            if (c.x < 0 || c.y < 0 || c.z < 0 || zbuffer.get(P.x, P.y)[0] > frag_depth)
                continue;
            bool discard = shader.fragment(c, color);
            if (!discard)
            {
                // 绘制像素
                zbuffer.set(P.x, P.y, TGAColor(frag_depth));
                image.set(P.x, P.y, color);
            }
        }
    }
}