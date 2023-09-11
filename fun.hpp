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
extern const string FOLDER;//�ļ���
extern const int KER_MEAN;//��ֵ��
extern const int KER_TAR;//Ŀ���
extern const bool FILTE;//����
extern const bool NEIGHBOR_DELETE;//ǰ��֡�˳�
extern const float contra_threshs;
extern const float contra_thresh;
extern const float contra_fina;
extern const int H, W;
extern const unsigned short hard_thresh;
extern int pre_num7[nums_target];//��¼ǰһ֡��
extern vector<int> res;
extern bool not_rec_flag[nums_target];
extern const int delete_width;//��Ե�е��Ŀ��
extern const bool same_position_six_frames_delete;
extern int pre_six_frames[42];
extern int six_frames_positions_index;
extern const int count_for_same_position;
//extern const int frameth = 729;
void mpcm_cal(unsigned short* data, unsigned short* mean_map, unsigned short* mpcm_map, float* mpcm_contra, int height, int width, int kernel_mean, int kernel_tar, float contra_thresh, unsigned short hard_thresh);//���ֺ�Ӳ��һ�£���������kernel
void folder_detect();//�����ļ����µ�raw�ļ�
void single_raw_detect(string&& path);//����raw�ļ����
char vision_16bit(unsigned short* mpcm_map, string& savePath, int frame, unsigned short* data, int height, int width, unsigned short* mean_map, vector<int>& points, unsigned short hard_thresh, unsigned short* eigen_value, float* eigen_contra, int ker_tar);//��Ŀ��ͼ�����ŵģ�����ת������
void check(int* arr, int height, int width);//ǰ��֡����
void findLowOutliers(double rate, unsigned short* eigen_value, float* eigen_contra);
bool is_same_position(int index, int* ref, int num, int need);
void insert_six_frames_position(int index);
static int eight_elements[8][2] = { {-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1} };