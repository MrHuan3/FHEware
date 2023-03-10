#include <iostream>
#include <opencv/cv.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <thread>

#define Randmod(x) rand() % x

using namespace std;
using namespace cv;

class server_couting;

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
	int **back_2;	 //真是结果2
	int *end;		 //恢复后的结果

	sd rd_pic(IplImage *img0);		   //读取图片
	sd gpic2arry(IplImage *img0);	   //图片转换为一维数组
	sd ext(int i, sd pic);			   //原始数组扩展
	sd mod_gen(int b_num_max, sd pic); //每组有多少个真实模分量
	int gcd(int x, int y);			   //计算两个数是否互素，如果是互素的则返回1
	sd b_gen(sd pic);				   //生成投影基
	sd real_b_gen(sd pic);			   //生成真实模分量
	sd GetArrayLen(sd pic);
	sd red_set_gen(int red_set_max, sd pic); //生成冗余集
	sd message(sd pic);						 //输出图片的真实信息
	sd posit_set(sd pic);					 //真实模分量的位置
	sd insert(sd pic);						 //真实模分量插入冗余集中
	sd recovery(server_couting pic, sd ori); //恢复真实计算结果
	int logestic(int min, int max);			 //伪随机数发生器
	sd cn_xq(sd pic, sd ori);
};

class send {
    public:
        int ***pre_send; //发送集
        int **b;		 //投影基
        int pix_num;	 //图像总像素的个数
        int b_num_max;	 //投影基的最大个数
        int red_set_max; //每个真实模分量最大生成多少个冗余集

        send send_gen(sd pic, send pic_t);
		send txt2enc(char path[]);
};

class server_couting {
public:
	int ***ct_reduce_1; //计算结果
	int ***ct_reduce_2; //计算结果

	server_couting server_reduce(send back, send now);
};

server_couting server_couting::server_reduce(send back, send now) {
	int ***p;
	int ***q;
	int **r;
	int **t;
	int i, j, k;
	int max = back.red_set_max + 1;
	server_couting z;
	p = back.pre_send;
	q = now.pre_send;
	r = back.b;
	t = back.b;
	int ***new_reduce_1;
	int ***new_reduce_2; // 定义参数
	new_reduce_1 = new int **[back.pix_num];
	for (i = 0; i < back.pix_num; i++) {
		new_reduce_1[i] = new int *[back.b_num_max];
		for (j = 0; j < back.b_num_max; j++) {
			new_reduce_1[i][j] = new int[max]; // 分配空间1
		}
	}
	new_reduce_2 = new int **[back.pix_num];
	for (i = 0; i < back.pix_num; i++) {
		new_reduce_2[i] = new int *[back.b_num_max];
		for (j = 0; j < back.b_num_max; j++) {
			new_reduce_2[i][j] = new int[max]; // 分配空间2
		}
	}
	for (i = 0; i < back.pix_num; i++) {
		for (j = 0; j < back.b_num_max; j++) {
			for (k = 0; k < max; k++) {
				new_reduce_1[i][j][k] = p[i][j][k] - q[i][j][k]; // 将背景帧和当前帧的差值传入NR1
				// 如果差值小于0
                while (new_reduce_1[i][j][k] < 0) {
					new_reduce_1[i][j][k] = new_reduce_1[i][j][k] + r[i][j]; // 加一个模基值保证数为正
				}
				new_reduce_2[i][j][k] = q[i][j][k] - p[i][j][k]; // 将当前帧和背景帧的差值传入NR2
				// 如果差值小于0
                while (new_reduce_2[i][j][k] < 0) {
					new_reduce_2[i][j][k] = new_reduce_2[i][j][k] + r[i][j]; // 加一个模基值保证数为正
				}
			}
		}
	}
	server_couting aa;
	aa.ct_reduce_1 = new_reduce_1;
	aa.ct_reduce_2 = new_reduce_2;

	return aa;
}

// read data from txt to struct
send send::txt2enc(char* path) {
	// define sd struct to store data
	send temp;
	// open cipher
	FILE *fp = fopen(path, "rb");
	// read cipher data to struct
	temp.pix_num = getw(fp);
	temp.b_num_max = getw(fp);
	temp.red_set_max = getw(fp);
	// allocate space
	temp.b = (int**) calloc(temp.pix_num, sizeof(int*));
	temp.pre_send = (int***) calloc(temp.pix_num, sizeof(int**));
	for (int i = 0; i < temp.pix_num; i++) {
		temp.b[i] = (int*) calloc(temp.b_num_max, sizeof(int));
		temp.pre_send[i] = (int**) calloc(temp.b_num_max, sizeof(int*));
		for (int j = 0; j < temp.b_num_max; j++) {
			temp.pre_send[i][j] = (int*) calloc(temp.red_set_max, sizeof(int));
		}
	}
	// read data
	for (int i = 0; i < temp.pix_num; i++) {
		fread(temp.b[i], sizeof(int), temp.b_num_max, fp);
		for (int j = 0; j < temp.b_num_max; j++) {
			fread(temp.pre_send[i][j], sizeof(int), temp.red_set_max + 1, fp);
		}
	}
	// close txt
	fclose(fp);
	
	// return struct
	return temp;
}

// save result to txt
void save2txt(server_couting result, char* path, int pn, int bnm, int rsm) {
	// define file to store
	FILE *fp = fopen(path, "wb");
	// write
	for (int i = 0; i < pn; i++) {
		for (int j = 0; j < bnm; j++) {
			fwrite(result.ct_reduce_1[i][j], sizeof(int), rsm, fp);
			fwrite(result.ct_reduce_2[i][j], sizeof(int), rsm, fp);
		}
	}
	// save and close file
	fclose(fp);
}

void blind_sub(send back, char video_name[], int frame_num, char clientID[], int i) {
	if (i < frame_num) {
		// define fold to store cipher
		char path[100];
		// get full path of cipher txt
		sprintf(path, "/root/FHEserver/ServerPool/C2S/%s/%d.txt", video_name, i);
		char* p = path;
		// define S2C fold
		char store_path[100];
		sprintf(store_path, "/root/FHEserver/ServerPool/S2C/%s/%s", clientID, video_name);
		if (access(path, F_OK) == 0) {
			// define current frame
			send now;
			// define server calculation result
			server_couting result;
			now = now.txt2enc(p);
			// 背景相减
			result = result.server_reduce(now, back);
			int pn = now.pix_num;
			int bnm = now.b_num_max;
			int rsm = now.red_set_max + 1;
			// save result
			// judge whether fold to store cipher txt is exist
			if (access(store_path, F_OK) == 0) {
				// fold exist and do nothing
			} else {  // fold is not exist
				// create fold
				mkdir(store_path, S_IRWXU);
			}
			char *temp = store_path;
			char n[10];
			sprintf(n, "/%d", i);
			strcat(temp, n);
			strcat(temp, ".txt");
			char* sp = temp;
			save2txt(result, sp, pn, bnm, rsm);
		} else {
			// do nothing
		}
	} else {
		// do nothing
	}
}

int main(int argc, char* argv[]) {
	// check argv number
	if (argc != 3) {
		cout << "Arg error" << endl;
		exit(-1);
	}
	cout << "Program Server Begin" << endl;
	// get video name
	char video_name[100];
	sprintf(video_name, "%s", argv[1]);
	// get client ID
	char clientID[15];
	sprintf(clientID, "%s", argv[2]);
	// define C2S fold
	char path[100];
	sprintf(path, "/root/FHEserver/ServerPool/C2S/%s/", video_name);
	// read frame number
	char fn_path[100];
	sprintf(fn_path, "/root/FHEserver/ServerPool/FrameNumber/%s.txt", video_name);
	FILE *ffn = fopen(fn_path, "rb");
	// read frame number
	int frame_num;
	frame_num = getw(ffn);
	fclose(ffn);
	// judge whether fold to store cipher txt is exist
	if (access(path, F_OK) == 0) {
        // fold exist and do nothing
    } else {  // fold is not exist
        // create fold
        mkdir(path, S_IRWXU);
    }
	// define client fold
	char client_path[100];
	sprintf(client_path, "/root/FHEserver/ServerPool/S2C/%s", clientID);
	if (access(client_path, F_OK) == 0) {
		// fold exist and do nothing
	} else {  // fold is not exist
		// create fold
		mkdir(client_path, S_IRWXU);
	}
	// define client video fold
	char S2C_path[100];
	sprintf(S2C_path, "/root/FHEserver/ServerPool/S2C/%s/%s", clientID, video_name);
	if (access(S2C_path, F_OK) == 0) {
		// fold exist and do nothing
	} else {  // fold is not exist
		// create fold
		mkdir(S2C_path, S_IRWXU);
	}
	cout << "Blind Extract Begin" << endl;
	// define background frame
	send back;
	// get background cipher
	char bg_path[100];
	sprintf(bg_path, "/root/FHEserver/ServerPool/C2S/%s/0.txt", video_name);
	char* bg = bg_path;
	back = back.txt2enc(bg);
	for (int i = 1; i < frame_num; i += 8) {
		// blind sub with multi threads
		thread th0(blind_sub, back, video_name, frame_num, clientID, i);
		thread th1(blind_sub, back, video_name, frame_num, clientID, i + 1);
		thread th2(blind_sub, back, video_name, frame_num, clientID, i + 2);
		thread th3(blind_sub, back, video_name, frame_num, clientID, i + 3);
		thread th4(blind_sub, back, video_name, frame_num, clientID, i + 4);
		thread th5(blind_sub, back, video_name, frame_num, clientID, i + 5);
		thread th6(blind_sub, back, video_name, frame_num, clientID, i + 6);
		thread th7(blind_sub, back, video_name, frame_num, clientID, i + 7);
		th0.join();
		th1.join();
		th2.join();
		th3.join();
		th4.join();
		th5.join();
		th6.join();
		th7.join();
		float per = 100.0 * i / frame_num;
		printf("Blind Extract Done: %.2f%%\n", per);
	}
	cout << "Blind Extract End" << endl;
	cout << "Program Server End" << endl;
	return 0;
}
