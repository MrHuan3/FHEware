#include <iostream>

int main(int argc, char* argv[]) {
    char path[100];
    char mode[5];
    char fn[50];
    sprintf(path, "%s", argv[1]);
    sprintf(mode, "%s", argv[2]);
    sprintf(fn, "%s", argv[3]);
    int frame_num = atoi(fn);
    FILE *fp = fopen(path, mode);
    fwrite(&frame_num, sizeof(int), 1, fp);
	fclose(fp);
    return 0;
}