#include "fun.hpp"

//const string FOLDER = "E:\\11��Զ������\\152502˫Ŀ��\\230831152544\\CHANGBO";//˫Ŀ��
//const string FOLDER = "E:\\11��Զ������\\153123\\230831153214\\CHANGBO";//����
//const string FOLDER = "E:\\11��Զ������\\152005\\230831152244\\CHANGBO";//������Ŀ��
//const string FOLDER = "E:\\11��Զ������\\153403\\230831153354\\CHANGBO";//��Ŀ��ɾ�����
const string FOLDER = "E:\\11��Զ������\\153822\\230831153824\\CHANGBO";//��������

const int H = 320, W = 256;

//const string FOLDER = "F:\\11Զ��\\153822\\230831153822\\ZHONGBO";//��������
//const string FOLDER = "F:\\11Զ��\\154046\\230831154036\\ZHONGBO";
//const string FOLDER = "F:\\11Զ��\\152502˫Ŀ��\\230831152536\\ZHONGBO";
//const string FOLDER = "F:\\11Զ��\\152502˫Ŀ��\\230831152455\\ZHONGBO";
//const string FOLDER = "F:\\11Զ��\\152005\\230831152054\\ZHONGBO";
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
int pre_num7[nums_target] = { 0 };//��¼ǰһ֡��
int pre_six_frames[42] = { 0 };
int six_frames_positions_index = 0;
const int count_for_same_position = 3;
vector<int> res;
bool not_rec_flag[nums_target] = { 0 };

int main() {
	folder_detect();
	return 0;
}