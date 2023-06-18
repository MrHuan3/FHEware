#include <iostream>
#include <opencv/cv.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <time.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>

#define Randmod(x) rand() % x

using namespace std;
using namespace cv;

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
	
		sd rd_pic(IplImage *img0);		   //读取图片
		sd gpic2arry(IplImage *img0);	   //图片转换为一维数组
		sd ext(int i, sd pic);			   //原始数组扩展
		sd mod_gen(int b_num_max, sd pic); //每组有多少个真实模分量
		int gcd(int x, int y);			   //计算两个数是否互素，如果是互素的则返回1
		sd b_gen(sd pic);				   //生成投影基
		sd real_b_gen(sd pic);			   //生成真实模分量
		sd red_set_gen(int red_set_max, sd pic); //生成冗余集
		sd posit_set(sd pic);					 //真实模分量的位置
		sd insert(sd pic);						 //真实模分量插入冗余集中
};

class send	{
	public:
		int ***pre_send; //发送集
		int **b;		 //投影基
		int pix_num;	 //图像总像素的个数
		int b_num_max;	 //投影基的最大个数
		int red_set_max; //每个真实模分量最大生成多少个冗余集

		send send_gen(sd pic, send pic_t);
};


send main_2(sd test) {
	send test_01;
	test_01.b = test.b;						// 传入投影基
	test_01.pre_send = test.pre_send;		// 传入发送集
	test_01.pix_num = test.pix_num;			// 传入像素总数
	test_01.b_num_max = test.b_num_max;		// 传入模基最大数
	test_01.red_set_max = test.red_set_max; // 传入冗余集最大数

	return test_01;
}

sd first(char *a) {
	char *b = a; // b指向传入的图片
	sd test;
	IplImage *img0 = NULL;		  // 定义并初始化img0
	img0 = cvLoadImage(b, 0);	  // 将传入的图像传入img0，同时保护原图像，将读取的图像转换为灰度图
	test = test.rd_pic(img0);	  // 将img0的尺寸信息传给test
	test = test.gpic2arry(img0);  // 将img0转换为一维数组来进行存储
	test = test.ext(3, test);	  // 将test的所有像素扩大3倍+2
	test = test.mod_gen(3, test); // test的每个像素生成3个模基
	test = test.b_gen(test);	  // 生成test的投影基
	test = test.real_b_gen(test); // 生成test的真实模分量
	test = test.red_set_gen(3, test); //  生成冗余集
	test = test.posit_set(test); // 确定真实模分量的位置
	test = test.insert(test);	 // 将真实模分量插入冗余集
	//整合发送集

	return test;
}

sd after(sd pic, char *a) {
	char *b = a;
	sd test;
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

// 生成投影基
sd sd::b_gen(sd pic) {
	int **p;
	int *q;
	int *t;
	int i, j, temp, k;
	p = new int *[pic.pix_num]; //初始化数组
	q = pic.b_num;
	t = pic.aftext;

	//给数组分配内存单元
	for (i = 0; i < pic.pix_num; i++) {
		p[i] = new int[pic.b_num_max];
	}
	//初始化数组
	for (i = 0; i < pic.pix_num; i++) {
		for (j = 0; j < pic.b_num_max; j++) {
			p[i][j] = -1; // 初始化矩阵
		}
	}

	 // 在图像总像素数内进行遍历
	for (i = 0; i < pic.pix_num; i++) {
 		// 在每个像素的真实模基数内进行遍历
		for (j = 0; j < q[i]; j++) {
		next:
			p[i][j] = Randmod(24); // 随机生成不超过24的数
			while (p[i][j] < 24 / 2) {
				p[i][j] = p[i][j] + 1; // 保证生成数不小于12
			}
			// 当前像素不为第一列像素时
			if (j > 0) {
				temp = j;
				for (k = 0; k < temp; k++) {
					// 保证模基之间互素
					if (pic.gcd(p[i][k], p[i][j]) > 1) {
						goto next;
					}
				}
			}
		}
	}
	// 原图像的投影基为p
	pic.b = p;
	return pic;
}

// 对原始图片进行像素扩大
sd sd::ext(int ext, sd pic) {
	int *p, *q;
	int i;
	p = new int[pic.pix_num];		  // 给p分配跟输入图像一样大的一维数组空间
	q = pic.origin;					  // q为输入图像的像素一维数组
	// 遍历输入图像像素
	for (i = 0; i < pic.pix_num; i++) {
		p[i] = q[i] * ext + 2; // p为原始图像的像素进行倍数扩展+2
	}
	pic.ext_num = ext; // 设定扩大倍数
	pic.aftext = p;	   // 将扩大后的数组传入原始图片保存
	return pic;		   // 返回图片
}

int sd::gcd(int x, int y) {
	int t;
	while (y) {
		t = x, x = y, y = t % y;
	}
	return x;
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

// 生成指定数量的模基
sd sd::mod_gen(int b_num_max, sd pic) {
	int *p;
	int i;
	p = new int[pic.pix_num]; // 分配与图像像素数量等大的空间
	// 确保生成的模基数要大于2个，安全性保障
	if (b_num_max < 2) {
		cout << "出错： 生成的最小个数为3个" << endl;
		pic.error = -1;
		return pic;
	}
	// 将投影基的数量赋值给p
	for (i = 0; i < pic.pix_num; i++) {
		p[i] = b_num_max;
	}
	pic.b_num = p;			   // 将p传入原图像的每个像素的模基数量
	pic.b_num_max = b_num_max; // 设定原图像的最大模基数
	return pic;
}

// 真实模分量的位置
sd sd::posit_set(sd pic) {
	int max = pic.red_set_max;
	int **p;
	int i, j;
	int temp = pic.b_num_max + 1;
	p = new int *[pic.pix_num];
	for (i = 0; i < pic.pix_num; i++) {
		p[i] = new int[pic.b_num_max]; // 分配每个像素点的模基空间
	}
	for (i = 0; i < pic.pix_num; i++) {
		for (j = 0; j < pic.b_num_max; j++) {
			p[i][j] = -1; // 初始化每个像素点的模基空间
		}
	}
	for (i = 0; i < pic.pix_num; i++) {
		for (j = 0; j < pic.b_num_max; j++) {
			p[i][j] = Randmod(temp); // 不太确定，确定每个模基位置
		}
	}
	pic.posit = p;
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
		for (j = 0; j < pic.b_num_max; j++) {
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

send send::send_gen(sd pic, send pic_t) {
	int ***p;
	int **q;
	int max, i, j, k;
	max = pic.red_set_max + 1;
	p = new int **[pic.pix_num];
	for (i = 0; i < pic.pix_num; i++) {
		p[i] = new int *[pic.b_num_max];
		for (j = 0; j < pic.b_num_max; j++) {
			p[i][j] = new int[max]; // 分配发送集的空间
		}
	}
	for (i = 0; i < pic.pix_num; i++) {
		for (j = 0; j < pic.b_num_max; j++) {
			for (k = 0; k < pic.red_set_max; k++) {
				p[i][j][k] = -1; // 初始化发送集
			}
		}
	}
	p = pic.pre_send;	// 传入发送集
	pic_t.pre_send = p; // 发送集传递
	q = new int *[pic.pix_num];
	for (i = 0; i < pic.pix_num; i++) {
		q[i] = new int[pic.b_num_max]; // 分配投影基空间
	}
	q = pic.b;	 // 传入投影基
	pic_t.b = q; // 传递投影基
	return pic_t;
}

int DivideFrame(char* video_name) {
	// define frame number
    int i = 0;
    // define video path to read
    char video_path[100];
    // judge whether the fold exist
    // define path to store temp frames
    char temp_path[100];
	sprintf(temp_path, "../ClientPool/Slides/");
	// get full path 
    strcat(temp_path, video_name);
    if (access(temp_path, F_OK) == 0) {
        // fold exist and do nothing
    } else {  // fold is not exist
        // create fold
        mkdir(temp_path, S_IRWXU);
    }
    // connect the full path of the video
	sprintf(video_path, "../ClientPool/Videos/%s", video_name);
    // capture video
    char res_path[100];
	sprintf(res_path, "../ClientPool/ResizedVideos/%s", video_name);
    VideoCapture cap(video_path);
	VideoWriter resized_video;
    resized_video.open(res_path, VideoWriter::fourcc('X', '2', '6', '4'), 25, Size(352, 288), 1);
    // difine video frame
    Mat img;
    while (true) {
        // capture next frame of the video
        bool sig;
        // define sig whether next img exist
        sig = cap.read(img);
        // if the frame is not the last one
        if (sig) {
            // save frames
            char saved_path[100];
            sprintf(saved_path, "../ClientPool/Slides/%s/%d.png", video_name, i);
			// save frame to the fold
			cv::resize(img, img, cv::Size(176, 144));
            // reduced_video.write(img);
			imwrite(saved_path, img);
            cv::resize(img, img, cv::Size(352, 288));
            resized_video.write(img);
            // frame number + 1
            i++;
        } else {  // if the frame is the last one
            // stop and break
            break;
        }
    }
	// reduced_video.release();
    resized_video.release();
	// return number of frames
	return i;
}

sd encrypt(char* video_name, int num, sd bg) {
	sd cur;
	//用户端：制作发送集
	// allocate space for current frame
	char *cur_frame = (char*) malloc (sizeof(char) * 100);
	// define temp path
	char temp[100];
	// get full path
	sprintf(temp, "../ClientPool/Slides/%s/%d.png", video_name, num);
	// transfer char[] to char*
	cur_frame = temp;
	// 整合发送集
	cur = after(bg, cur_frame);
		
	return cur;
}

// store send struct to txt
void save2txt(send send_frame, char* video_name, int num) {
	// define path to store cipher txt
	char *path = (char*)malloc(sizeof(char) * 100);
	// get full path
	sprintf(path, "../ClientPool/C2S/%s", video_name);
	// judge whether fold to store cipher txt is exist
	if (access(path, F_OK) == 0) {
        // fold exist and do nothing
    } else {  // fold is not exist
        // create fold
        mkdir(path, S_IRWXU);
    }
	// connect the full path of the video
	sprintf(path, "../ClientPool/C2S/%s/%d.txt", video_name, num);
	// open cipher txt to transfer data
	FILE *f = fopen(path, "wb");
	// write 图像总像素的个数 into txt
	fwrite(&send_frame.pix_num, sizeof(int), 1, f);
	// write 投影基的最大个数 into txt
	fwrite(&send_frame.b_num_max, sizeof(int), 1, f);
	// write 每个真实模分量最大生成多少个冗余集 into txt
	fwrite(&send_frame.red_set_max, sizeof(int), 1, f);
	// write 投影基 into txt
	for (int i = 0; i < send_frame.pix_num; i++) {
		fwrite(send_frame.b[i], sizeof(int), send_frame.b_num_max, f);
		for (int j = 0; j < send_frame.b_num_max; j++) {
            // write 发送集 into txt
			fwrite(send_frame.pre_send[i][j], sizeof(int), send_frame.red_set_max + 1, f);
        }
    }
	// close and save cipher txt
    fclose(f);
}

// save bg frame cipher to ReadAgain
void save_first(sd bg_cipher, char *video_name) {
	// define read again path for the back ground cipher
	char ra_path[100];
	// get full path
	sprintf(ra_path, "../ClientPool/BackGround/%s.txt", video_name);
	FILE *fp = fopen(ra_path, "wb");
	// write
	fwrite(&bg_cipher.height, sizeof(int), 1, fp);
	fwrite(&bg_cipher.width, sizeof(int), 1, fp);
	fwrite(&bg_cipher.pix_num, sizeof(int), 1, fp);
	fwrite(&bg_cipher.ext_num, sizeof(int), 1, fp);
	fwrite(&bg_cipher.b_num_max, sizeof(int), 1, fp);
	fwrite(&bg_cipher.error, sizeof(int), 1, fp);
	fwrite(&bg_cipher.red_set_max, sizeof(int), 1, fp);
	fwrite(bg_cipher.origin, sizeof(int), bg_cipher.pix_num, fp);
	fwrite(bg_cipher.aftext, sizeof(int), bg_cipher.pix_num, fp);
	fwrite(bg_cipher.b_num, sizeof(int), bg_cipher.pix_num, fp);
	for (int i = 0; i < bg_cipher.pix_num; i++) {
		fwrite(bg_cipher.b[i], sizeof(int), bg_cipher.b_num_max, fp);
		fwrite(bg_cipher.real_b[i], sizeof(int), bg_cipher.b_num_max, fp);
		fwrite(bg_cipher.posit[i], sizeof(int), bg_cipher.b_num_max, fp);
		for (int j = 0; j < bg_cipher.b_num_max; j++) {
			fwrite(bg_cipher.red_set[i][j], sizeof(int), bg_cipher.red_set_max, fp);
			fwrite(bg_cipher.pre_send[i][j], sizeof(int), bg_cipher.red_set_max + 1, fp);
		}
		
	}
	fclose(fp);
}

void enc(sd bg_cipher, char video_name[], int frame_num, int i) {
	if (i < frame_num) {
		// encrypt No:i frame
		// define sd to store current frame
		sd cur_cipher;
		// get current frame cipher
		cur_cipher = encrypt(video_name, i, bg_cipher);
		// define send cur
		send send_cur;
		// make current frame sending block
		send_cur = main_2(cur_cipher);
		// save cur as No:i to txt
		save2txt(send_cur, video_name, i);
	} else {
	}
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		cout << "arg error" << endl;
		exit(-1);
	}
	cout << "Program BeforeClient Begin" << endl;
	// define video name to input
    char video_name[100];
    // printf("请输入视频名称: ");
	// input video_name
    // scanf("%s", video_name);
	sprintf(video_name, "%s", argv[1]);
	// divide video into frames and get frame number
	cout << "Divide Frame Begin" << endl;
	int frame_num = DivideFrame(video_name);

	char fn_path[100];
	// get full path
	sprintf(fn_path, "../ClientPool/FrameNumber/%s.txt", video_name);
	FILE *ffn = fopen(fn_path, "wb");
	// write
	fwrite(&frame_num, sizeof(int), 1, ffn);
	fclose(ffn);
	cout << "Divide Frame Done" << endl;
	cout << "Encryption Begin" << endl;
	// define sd to store background frame
	sd bg_cipher;
	// 背景帧
	char *bg_frame = (char*) malloc(sizeof(char) * 100);
	// connect full path of frames
	sprintf(bg_frame, "../ClientPool/Slides/%s/0.png", video_name);
	// get background cipher
	bg_cipher = first(bg_frame);
	save_first(bg_cipher, video_name);
	// define send bg
	send send_bg;
	// 制作背景帧发送集
	send_bg = main_2(bg_cipher);
	// save bg as No:0 to txt
	save2txt(send_bg, video_name, 0);
	for (int i = 1; i < frame_num; i += 8) {
		// make cipher with multi threads
		thread th0(enc, bg_cipher, video_name, frame_num, i);
		thread th1(enc, bg_cipher, video_name, frame_num, i + 1);
		thread th2(enc, bg_cipher, video_name, frame_num, i + 2);
		thread th3(enc, bg_cipher, video_name, frame_num, i + 3);
		thread th4(enc, bg_cipher, video_name, frame_num, i + 4);
		thread th5(enc, bg_cipher, video_name, frame_num, i + 5);
		thread th6(enc, bg_cipher, video_name, frame_num, i + 6);
		thread th7(enc, bg_cipher, video_name, frame_num, i + 7);
		th0.join();
		th1.join();
		th2.join();
		th3.join();
		th4.join();
		th5.join();
		th6.join();
		th7.join();
		float p = 100.0 * i / (frame_num - 1);
		printf("Encryption Done: %.2f%%\n", p);
	}
	cout << "Encryption End" << endl;
	cout << "Program BeforeClient End" << endl;

	return 0;
}
