#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <filesystem>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#define nums_target 100
#define report 4
using namespace std;
using namespace cv;
namespace fs = std::filesystem;
extern const string FOLDER;//文件夹
extern const int KER_MEAN;//均值核
extern const int KER_TAR;//目标核
extern const bool FILTE;//过滤
extern const bool NEIGHBOR_DELETE;//前后帧滤除
extern const float contra_threshs;
extern const float contra_thresh;
extern const float contra_fina;
extern const int H, W;
extern const unsigned short hard_thresh;
extern int pre_num7[nums_target];//记录前一帧的
extern vector<int> res;
extern bool not_rec_flag[nums_target];
extern const int delete_width;//边缘切掉的宽度
extern const bool same_position_six_frames_delete;
extern int pre_six_frames[42];
extern int six_frames_positions_index;
extern const int count_for_same_position;
//extern const int frameth = 729;
void mpcm_cal(unsigned short* data, unsigned short* mean_map, unsigned short* mpcm_map, float* mpcm_contra, int height, int width, int kernel_mean, int kernel_tar, float contra_thresh, unsigned short hard_thresh);//保持和硬件一致，设置两个kernel
void folder_detect();//遍历文件夹下的raw文件
void single_raw_detect(string&& path);//单个raw文件检测
char vision_16bit(unsigned short* mpcm_map, string& savePath, int frame, unsigned short* data, int height, int width, unsigned short* mean_map, vector<int>& points, unsigned short hard_thresh, unsigned short* eigen_value, float* eigen_contra, int ker_tar);//项目的图是躺着的，所以转换操作
void check(int* arr, int height, int width);//前后帧关联
void findLowOutliers(double rate, unsigned short* eigen_value, float* eigen_contra);
bool is_same_position(int index, int* ref, int num, int need);
void insert_six_frames_position(int index);
static int eight_elements[8][2] = { {-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1} };