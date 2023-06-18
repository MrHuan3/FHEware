#include <iostream>
#include <opencv/cv.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>

#define Randmod(x) rand() % x

using namespace std;
using namespace cv;

class server_couting {
public:
	int ***ct_reduce_1; //计算结果
	int ***ct_reduce_2; //计算结果

	server_couting read_result(char path[], int pn, int bnm, int rdm);
};

class sd {
	public:
		int height;		 //图像的高度
		int width;		 //图像的宽度
		int pix_num;	 //图像总像素的个数
		int *origin;	 //灰度图像转换为一维数组
		int ext_num;	 //图像像素扩展倍数
		int *aftext;	 //图像扩展之后的数组
		int b_num_max;	 //投影基的最大个数
		int *b_num;		 //每个数分解的真实模分量的个数
		int error = 1;	 //程序出错
		int **b;		 //投影基
		int **real_b;	 //真实模分量
		int ***red_set;	 //冗余基
		int red_set_max; //每个真实模分量最大生成多少个冗余集
		int **posit;	 //真实模分量的位置
		int ***pre_send; //发送集
		int **back_1;	 //真实结果1
		int **back_2;	 //真实结果2
		int *end;		 //恢复后的结果
	
		sd rd_pic(IplImage *img0);		   //读取图片
		sd gpic2arry(IplImage *img0);	   //图片转换为一维数组
		sd ext(int i, sd pic);			   //原始数组扩展
		sd real_b_gen(sd pic);			   //生成真实模分量
		sd red_set_gen(int red_set_max, sd pic); //生成冗余集
		sd insert(sd pic);						 //真实模分量插入冗余集中
		sd recovery(server_couting pic, sd ori); //恢复真实计算结果
		sd cn_xq(sd pic, sd ori);
		sd read_first(char* path);
};

class send	{
	public:
		int ***pre_send; //发送集
		int **b;		 //投影基
		int pix_num;	 //图像总像素的个数
		int b_num_max;	 //投影基的最大个数
		int red_set_max; //每个真实模分量最大生成多少个冗余集
};

sd sd::recovery(server_couting pic, sd ori) {
	sd p;
	int **a, i, j;
	int ***piic_1;
	int ***piic_2;
	piic_1 = pic.ct_reduce_1;
	piic_2 = pic.ct_reduce_2;
	a = ori.posit;
	int **q_1;
	q_1 = new int *[ori.pix_num];
	for (i = 0; i < ori.pix_num; i++) {
		q_1[i] = new int[ori.b_num_max]; // 分配像素x模基数量的空间
	}
	for (i = 0; i < ori.pix_num; i++) {
		for (j = 0; j < ori.b_num_max; j++) {
			q_1[i][j] = piic_1[i][j][a[i][j]]; // 将计算结果传入
		}
	}
	int **q_2;
	q_2 = new int *[ori.pix_num];
	for (i = 0; i < ori.pix_num; i++) {
		q_2[i] = new int[ori.b_num_max];
	}
	for (i = 0; i < ori.pix_num; i++) {
		for (j = 0; j < ori.b_num_max; j++) {
			q_2[i][j] = piic_2[i][j][a[i][j]];
		}
	}
	p.back_1 = q_1;
	p.back_2 = q_2;

	return p;
}

int exgcd(int a, int b, int &x, int &y) {
	int d;
	if (b == 0) {
		x = 1;
		y = 0;
		return a;
	}
	d = exgcd(b, a % b, y, x);
	y -= a / b * x;
	return d;
}

//中国剩余定理 ,r[]存放余数 ,prime[]存放两两互质的数
int Chinese_Remainder(int r[], int prime[], int len) {
	int i, d, x, y, m, n = 1, sum = 0;
	//计算所以除数的积n，也是所以除数的最小公倍数
	for (i = 0; i < len; i++) {
		n *= prime[i];
	}
	//计算符合所以条件的数
	for (i = 0; i < len; i++) {
		m = n / prime[i];			  //计算除去本身的所有除数的积m
		d = exgcd(prime[i], m, x, y); //计算w[i]*x+m*y=gcd(w[i],m)的一个解y
									  //累加整数解y的同并不断对n取余，其利用公式:(a+b)%c=(a%c+b%c)%c
		sum = (sum + y * m * r[i]) % n;
	}
	return (n + sum % n) % n; //满足所以方程的最小解
}

sd sd::cn_xq(sd pic, sd ori) {
	//确定模分量的个数
	int *p_1, *p_2, *p_b;
	p_1 = new int[ori.b_num_max];
	p_2 = new int[ori.b_num_max];
	p_b = new int[ori.b_num_max];
	//一维数组用于接收图像的一维信息
	int *q;
	//分配内存空间
	q = new int[ori.pix_num];
	int i, j, temp_1, temp_2;
	for (i = 0; i < ori.pix_num; i++) {
		for (j = 0; j < ori.b_num_max; j++) {
			p_1[j] = pic.back_1[i][j];
			p_2[j] = pic.back_2[i][j];
			p_b[j] = ori.b[i][j];
		}
		temp_1 = Chinese_Remainder(p_1, p_b, ori.b_num_max);
		temp_2 = Chinese_Remainder(p_2, p_b, ori.b_num_max);
		if (temp_1 > temp_2) {
			q[i] = temp_2;
		} else {
			q[i] = temp_1 / 3;
		}
	}
	ori.end = q;

	return ori;
}

sd sd::read_first(char* path) {
	sd temp;
	// open file to read
	FILE *fp = fopen(path, "rb");
	temp.height = getw(fp);
	temp.width = getw(fp);
	temp.pix_num = getw(fp);
	temp.ext_num = getw(fp);
	temp.b_num_max = getw(fp);
	temp.error = getw(fp);
	temp.red_set_max = getw(fp);
	
	// allocate space
	temp.origin = (int*) calloc(temp.pix_num, sizeof(int));
	temp.aftext = (int*) calloc(temp.pix_num, sizeof(int));
	temp.b_num = (int*) calloc(temp.pix_num, sizeof(int));
	temp.b = (int**) calloc(temp.pix_num, sizeof(int*));
	temp.real_b = (int**) calloc(temp.pix_num, sizeof(int*));
	temp.red_set = (int***) calloc(temp.pix_num, sizeof(int**));
	temp.posit = (int**)calloc(temp.pix_num, sizeof(int*));
	temp.pre_send = (int***) calloc(temp.pix_num, sizeof(int**));
	for (int i = 0; i < temp.pix_num; i++) {
		temp.b[i] = (int*) calloc(temp.b_num_max, sizeof(int));
		temp.real_b[i] = (int*) calloc(temp.b_num_max, sizeof(int));
		temp.red_set[i] = (int**) calloc(temp.b_num_max, sizeof(int*));
		temp.posit[i] = (int*) calloc(temp.b_num_max, sizeof(int));
		temp.pre_send[i] = (int**) calloc(temp.b_num_max, sizeof(int*));
		for (int j = 0; j < temp.b_num_max; j++) {
            temp.red_set[i][j] = (int*) calloc(temp.red_set_max, sizeof(int));
			temp.pre_send[i][j] = (int*) calloc(temp.red_set_max + 1, sizeof(int));
        }
	}
	
	// read
	fread(temp.origin, sizeof(int), temp.pix_num, fp);
    fread(temp.aftext, sizeof(int), temp.pix_num, fp);
    fread(temp.b_num, sizeof(int), temp.pix_num, fp);
    for (int i = 0; i < temp.pix_num; i++) {
        fread(temp.b[i], sizeof(int), temp.b_num_max, fp);
		fread(temp.real_b[i], sizeof(int), temp.b_num_max, fp);
		fread(temp.posit[i], sizeof(int), temp.b_num_max, fp);
		for (int j = 0; j < temp.b_num_max; j++) {
            fread(temp.red_set[i][j], sizeof(int), temp.red_set_max, fp);
			fread(temp.pre_send[i][j], sizeof(int), temp.red_set_max + 1, fp);
        }
    }
	// close file
    fclose(fp);

	return temp;
}

server_couting server_couting::read_result(char* path, int pn, int bnm, int rdm) {
	server_couting temp;
	// open file to read
	FILE *fp = fopen(path, "rb");
	// allocate space
	temp.ct_reduce_1 = (int***) calloc(pn, sizeof(int**));
	temp.ct_reduce_2 = (int***) calloc(pn, sizeof(int**));
	for (int i = 0; i < pn; i++) {
		temp.ct_reduce_1[i] = (int**) calloc(bnm, sizeof(int*));
		temp.ct_reduce_2[i] = (int**) calloc(bnm, sizeof(int*));
		for (int j = 0; j < bnm; j++) {
			temp.ct_reduce_1[i][j] = (int*) calloc(rdm, sizeof(int));
			temp.ct_reduce_2[i][j] = (int*) calloc(rdm, sizeof(int));
		}
	}
	// read
	for (int i = 0; i < pn; i++) {
        for (int j = 0; j < bnm; j++) {
            fread(temp.ct_reduce_1[i][j], sizeof(int), rdm, fp);
			fread(temp.ct_reduce_2[i][j], sizeof(int), rdm, fp);
        }
    }
	// save and close file
	fclose(fp);

	return temp;
}

sd sd::insert(sd pic) {
	int max, i, j, k;
	int ***p;
	int ***q;
	int **t;
	max = red_set_max + 1;
	q = red_set;
	t = posit;
	//分配内存空间
	p = new int **[pic.pix_num];
	for (i = 0; i < pic.pix_num; i++) {
		p[i] = new int *[pic.b_num_max];
		for (j = 0; j < pic.b_num_max; j++) {
			p[i][j] = new int[max];
		}
	}
	//初始化三维数组
	for (i = 0; i < pic.pix_num; i++) {
		for (j = 0; j < pic.b_num_max; j++) {
			for (k = 0; k < max; k++) {
				p[i][j][k] = -1;
			}
		}
	}
	//插入操作
	for (i = 0; i < pic.pix_num; i++) {
		for (j = 0; j < pic.b_num_max; j++) {
			p[i][j][pic.posit[i][j]] = pic.real_b[i][j];
			for (k = 0; k < max; k++) {
				if (k < pic.posit[i][j]) {
					p[i][j][k] = q[i][j][k];
				}
				if (k > pic.posit[i][j] || k == pic.posit[i][j]) {
					p[i][j][k + 1] = q[i][j][k];
				}
			}
		}
	}
	pic.pre_send = p;
	return pic;
}

//  生成冗余集
sd sd::red_set_gen(int red_set_max, sd pic) {
	int max, i, j, k;
	max = red_set_max; // 定义最大冗余集个数
	pic.red_set_max = max;
	int ***p;
	int **q;
	q = pic.real_b;
	//分配内存空间
	p = new int **[pic.pix_num];
	for (i = 0; i < pic.pix_num; i++) {
		p[i] = new int *[pic.b_num_max];
		for (j = 0; j < pic.b_num_max; j++) {
			p[i][j] = new int[max];
		}
	}
	//初始化三维数组
	for (i = 0; i < pic.pix_num; i++) {
		for (j = 0; j < pic.b_num_max; j++) {
			for (k = 0; k < pic.red_set_max; k++) {
				p[i][j][k] = -1;
			}
		}
	}
	// 随机生成冗余模基
	for (i = 0; i < pic.pix_num; i++) {
		for (j = 0; j < pic.b_num_max; j++) {
			for (k = 0; k < pic.red_set_max; k++) {
				if (q[i][j] != -1) {
					p[i][j][k] = q[i][j] + Randmod(2);
				}
			}
		}
	}
	pic.red_set = p;
	return pic;
}

sd sd::real_b_gen(sd pic) {
	int **p; //真实模分量
	int **q; //投影基
	int *r;	 //被模的扩展后的数
	int *t;	 //每组真实模分量的个数
	int i, j;
	t = pic.b_num;
	r = pic.aftext;
	q = pic.b;
	p = new int *[pic.pix_num];
	for (i = 0; i < pic.pix_num; i++) {
		p[i] = new int[pic.b_num_max]; // 分配空间
	}
	//真实模分量初始化
	for (i = 0; i < pic.pix_num; i++) {
		for (j = 0; j < pic.b_num_max; j++)
		{
			p[i][j] = -1;
		}
	}
	for (i = 0; i < pic.pix_num; i++) {
		for (j = 0; j < t[i]; j++) {
			p[i][j] = r[i] % q[i][j]; // 计算每个像素点上放大被模之后的值
		}
	}
	pic.real_b = p; // 传入真实模基
	return pic;
}

// 读取图片信息，传入长款和像素数量
sd sd::rd_pic(IplImage *origin) {
	sd pic;
	pic.height = origin->height;
	pic.width = origin->width;
	pic.pix_num = origin->height * origin->width;
	return pic;
}

// 将图像转换为一维数组
sd sd::gpic2arry(IplImage *origin) {
	sd pic;
	int i, j, k;
	int **p;
	int *q;
	pic.height = origin->height;
	pic.width = origin->width;
	pic.pix_num = origin->height * origin->width; // 将图像的长宽和像素数量都传入
	p = new int *[pic.height];
	q = new int[pic.pix_num]; // 分配空间
	for (i = 0; i < pic.height; i++) {
		p[i] = new int[pic.width]; // 创建与原图像等大等形状的矩阵p
	}
	uchar *ptr;
	// 完成将二维图像转换为q的一维数组
	for (i = 0, k = 0; i < pic.height; i++) {
		ptr = (uchar *)origin->imageData + i * origin->widthStep; // 计算偏差
		for (j = 0; j < origin->width; j++) {
			p[i][j] = (int)*(ptr + j);
			q[k] = p[i][j]; // 赋值
			k++;
		}
	}
	pic.origin = q;
	return pic;
}

sd sd::ext(int ext, sd pic) {
	int *p, *q;
	int i;
	p = new int[pic.pix_num];		  // 给p分配跟输入图像一样大的一维数组空间
	q = pic.origin;					  // q为输入图像的像素一维数组
	// 遍历输入图像像素
	for (i = 0; i < pic.pix_num; i++) {
		p[i] = q[i] * ext + 2; // p为原始图像的像素进行倍数扩展+2
	}
	pic.ext_num = ext; // 设定扩大背书
	pic.aftext = p;	   // 将扩大后的数组传入原始图片保存
	return pic;		   // 返回图片
}

sd after(sd pic, char *a) {
	char *b = a;
	sd test;
	Mat image;
	IplImage *img0 = NULL;
	img0 = cvLoadImage(b, 0);		// 将图片导入img0
	test = test.rd_pic(img0);		// 读取图片的长款和像素数量
	test = test.gpic2arry(img0);	// 将图片用一维数组test表示
	test = test.ext(3, test);		// 将test每个像素值扩大3倍+2
	test.b_num = pic.b_num;			// 传入背景帧真实模分量数量
	test.b_num_max = pic.b_num_max; // 传入背景帧投影模基最大数量
	test.b = pic.b;					// 传入背景帧投影模基
	test = test.real_b_gen(test);	// 生成真实模分量
	test = test.red_set_gen(3, test); // 生成冗余集
	test.posit = pic.posit;			  // 传入背景帧模基位置
	test = test.insert(test);		  // 将真实模分量插入冗余集
	//整合发送集
	return test;
}

void decr(sd now_1, char bg[], char video_name[], int frame_num, int i) {
	char txt_path[100];
	sprintf(txt_path, "../ClientPool/S2C/%s/%d.txt", video_name, i);
	// judge whether txt exist
	if (access(txt_path, F_OK) == 0) {
		server_couting result;
		sd after_recovery, back_1;
		char cur[100];
		sprintf(cur, "../ClientPool/Slides/%s/%d.png", video_name, i);
		char* fcur = cur;
		result = result.read_result(txt_path, now_1.pix_num, now_1.b_num_max, now_1.red_set_max + 1);
		back_1 = after(now_1, fcur);
		// 恢复真实数据
		after_recovery = back_1.recovery(result, now_1);
		// 中国剩余定理计算恢复now_1
		now_1 = now_1.cn_xq(after_recovery, now_1);
		int match_num = 0;
		// recreate image based on results
		IplImage *img_1 = cvLoadImage(bg); // 传入背景帧到img1
		IplImage *img_2 = cvLoadImage(fcur); // 传入当前帧到img2
		IplImage *img = cvCreateImage(cvSize(back_1.width, back_1.height), 8, 1); // 创建图片尺寸
		// define different pixel number
		int dif = 0;
		for (int j = 0; j < back_1.width * back_1.height; j++) {
			uchar *pixel2 = new uchar;				  // 新建像素点2
			pixel2 = (uchar *)(img_2->imageData + j); // 将img2的当前像素传入像素点2
			int temp2 = (*pixel2) + 0;				  // 类型转换
			uchar *pixel1 = new uchar;				  // 新建像素点1
			pixel1 = (uchar *)(img_1->imageData + j); // 将img1的当前像素传入像素点1
			int temp1 = (*pixel1) + 0;				  // 类型转换
			int temp = 0;							  // 新建差分像素点
			temp = temp1 > temp2 ? temp1 - temp2 : temp2 - temp1;
			temp = temp > 50 ? 255 : 0;  // 设定阈值为25, 差值超过阈值为白色, 不超过阈值为黑色
			// 若恢复后的像素超过阈值
			if (now_1.end[j] > 50) {
				now_1.end[j] = 255; // 设为白色
				dif++;
			} else {
				now_1.end[j] = 0; // 不超过阈值设为黑色
			}
			if (temp == now_1.end[j]) {
				match_num += 1; // 真实的结果+1
			}
			img->imageData[j] = now_1.end[j]; // 将结果数据传入
		}
		double total = static_cast<double>(back_1.width * back_1.height);
		double match = static_cast<double>(match_num);
		double front = 1.0 * dif / (back_1.width * back_1.height);
		// if motion pixel rate is more than certain value
		if (front >= 0.01) {
			// add image to reduced video
			// form transform
			Mat image = cvarrToMat(img);
			cv::resize(image, image, cv::Size(352, 288));
			// save frames
			char rec_save_path[100];
            sprintf(rec_save_path, "../ClientPool/Recovery/%s/%d.png", video_name, i);
            // save frame to the fold
            imwrite(rec_save_path, image);
		} else {
			// not add image to reduced video
		}
	} else {
		// do nothing
	}
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		cout << "Arg error" << endl;
		exit(-1);
	}
	cout << "Program AfterClient Begin" << endl;
	char path[100];
	// define video name to input
    char video_name[100];
	sprintf(video_name, "%s", argv[1]);
	sprintf(path, "../ClientPool/BackGround/%s.txt", video_name);
	// read frame number
	char fn_path[100];
	sprintf(fn_path, "../ClientPool/FrameNumber/%s.txt", video_name);
	FILE *ffn = fopen(fn_path, "rb");
	// read frame number
	int frame_num;
	frame_num = getw(ffn);
	fclose(ffn);
	char rec_path[100];
	sprintf(rec_path, "../ClientPool/Recovery/%s", video_name);
	if (access(rec_path, F_OK) == 0) {
		// fold exist and do nothing
	} else {  // fold is not exist
		// create fold
		mkdir(rec_path, S_IRWXU);
	}
	cout << "Decryption Begin" << endl;
	// read background data
	sd now_1 = now_1.read_first(path);
	// read fisrt data from txt
	char result_path[100];
	// get full path of txt
	sprintf(result_path, "../ClientPool/S2C/%s", video_name);
	// judge whether fold to store cipher txt is exist
	if (access(result_path, F_OK) == 0) {
		// fold exist and do nothing
	} else {  // fold is not exist
		// create fold
		mkdir(result_path, S_IRWXU);
	}
	char bg[100];
	sprintf(bg, "../ClientPool/Slides/%s/0.png", video_name);
	for (int i = 1; i < frame_num; i += 8) {
		// decrypt with multi threads
		thread th0(decr, now_1, bg, video_name, frame_num, i);
		thread th1(decr, now_1, bg, video_name, frame_num, i + 1);
		thread th2(decr, now_1, bg, video_name, frame_num, i + 2);
		thread th3(decr, now_1, bg, video_name, frame_num, i + 3);
		thread th4(decr, now_1, bg, video_name, frame_num, i + 4);
		thread th5(decr, now_1, bg, video_name, frame_num, i + 5);
		thread th6(decr, now_1, bg, video_name, frame_num, i + 6);
		thread th7(decr, now_1, bg, video_name, frame_num, i + 7);
		th0.join();
		th1.join();
		th2.join();
		th3.join();
		th4.join();
		th5.join();
		th6.join();
		th7.join();
		float p = 100.0 * i / (frame_num - 1);
		printf("Decryption Done: %.2f%%\n", p);
	}
	cout << "Decryption End" << endl;
	cout << "Rebuild Reduced Video Begin" << endl;
	// define reduced videos stored path
	char reduced_v[100];
	sprintf(reduced_v, "../ClientPool/ReducedVideos/%s", video_name);
	// declare reduced video
	VideoWriter reduced_video;
	reduced_video.open(reduced_v, VideoWriter::fourcc('X', '2', '6', '4'), 25, Size(352, 288), 0);
	for (int i = 1; i < frame_num; i++) {
		char reb_path[100];
		sprintf(reb_path, "../ClientPool/Recovery/%s/%d.png", video_name, i);
		// judge whether recovered frame exist
		if (access(reb_path, F_OK) == 0) {
			// frame exist and write into video
			IplImage *image = NULL;
			image = cvLoadImage(reb_path, 0);
			Mat img = cvarrToMat(image);
			cv::resize(img, img, cv::Size(352, 288));
			reduced_video.write(img);
		} else {
			// do nothing
		}
	}
	// release VideoWriter
	reduced_video.release();
	cout << "Rebuild Reduced Video End" << endl;
	cout << "Program AfterClient End" << endl;
	
	return 0;
}

