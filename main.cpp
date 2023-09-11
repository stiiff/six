#include "fun.hpp"

//const string FOLDER = "E:\\11所远距数据\\152502双目标\\230831152544\\CHANGBO";//双目标
//const string FOLDER = "E:\\11所远距数据\\153123\\230831153214\\CHANGBO";//多云
//const string FOLDER = "E:\\11所远距数据\\152005\\230831152244\\CHANGBO";//多云有目标
//const string FOLDER = "E:\\11所远距数据\\153403\\230831153354\\CHANGBO";//单目标干净背景
const string FOLDER = "E:\\11所远距数据\\153822\\230831153824\\CHANGBO";//超难序列

const int H = 320, W = 256;

//const string FOLDER = "F:\\11远距\\153822\\230831153822\\ZHONGBO";//超难序列
//const string FOLDER = "F:\\11远距\\154046\\230831154036\\ZHONGBO";
//const string FOLDER = "F:\\11远距\\152502双目标\\230831152536\\ZHONGBO";
//const string FOLDER = "F:\\11远距\\152502双目标\\230831152455\\ZHONGBO";
//const string FOLDER = "F:\\11远距\\152005\\230831152054\\ZHONGBO";
//const int H = 640, W = 512;

const float contra_threshs = 0.5;//contrac
const float contra_thresh = 0.5;//contrac
const float contra_fina = 0.5;
const unsigned short hard_thresh = 10;//eigen
const bool FILTE = true;
const int KER_MEAN = 3;
const int KER_TAR = 5;//kernel
const bool NEIGHBOR_DELETE = false;
const bool same_position_six_frames_delete = false;
const int delete_width = 10;
int pre_num7[nums_target] = { 0 };//记录前一帧的
int pre_six_frames[42] = { 0 };
int six_frames_positions_index = 0;
const int count_for_same_position = 3;
vector<int> res;
bool not_rec_flag[nums_target] = { 0 };

int main() {
	folder_detect();
	return 0;
}