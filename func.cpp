#include "fun.hpp"
ushort grey[nums_target] = { 0 };
int sub_array[nums_target] = { 0 };
bool isCloud = false;
//长波
#define kernel_tars 3
#define stride 2
#define box 30
#define boxthreshold 20
#define none_nums 25
#define HEIGHT 320
#define WIDTH 256
#define mean_radius 3
//中波
//#define kernel_tars 3
//#define stride 2
//#define box 30
//#define boxthreshold 20
//#define none_nums 10
//#define HEIGHT 640
//#define WIDTH 512
//#define mean_radius 3
void mpcm_cal(unsigned short* data, unsigned short* mean_map, unsigned short* mpcm_map, float* mpcm_contra, int height, int width, int kernel_mean, int kernel_tar, float contra_thresh, unsigned short hard_thresh) {//保持和硬件一致，设置两个kernel
	//传入16位原始数据、均值图、特征图
	int size = height * width;
	int offset_mean = kernel_mean >> 1;
	int mean = 0;
	int aver = 0;
	memset(mean_map, 0, sizeof(unsigned short) * size);
	memset(mpcm_map, 0, sizeof(unsigned short) * size);
	for (int i = 5 * height; i < size - 5 * height; i++) {//前三行与后三行不要， 均值图从第五张开始，无论目标kernelsize是多少，都用3来做均值
		mean = 0;
		for (int row = -offset_mean; row <= offset_mean; row++) {
			for (int col = -offset_mean; col <= offset_mean; col++) {
				mean += data[i + row * height + col];
			}
		}
		aver = (mean * 7282) >> 16;
		mean_map[i] = aver;
	}
	int offset_tar = kernel_tar >> 1;
	int start = kernel_tar + offset_tar;
	int neighbor[8] = { 0 }, neighbors[8] = { 0 };
	for (int i = start * height; i < size - start * height; i++) {
		neighbor[0] = mean_map[i] - mean_map[i - kernel_tar * height - kernel_tar];
		neighbor[1] = mean_map[i] - mean_map[i - kernel_tar * height];
		neighbor[2] = mean_map[i] - mean_map[i - kernel_tar * height + kernel_tar];
		neighbor[3] = mean_map[i] - mean_map[i + kernel_tar];
		neighbor[4] = mean_map[i] - mean_map[i + kernel_tar * height + kernel_tar];
		neighbor[5] = mean_map[i] - mean_map[i + kernel_tar * height];
		neighbor[6] = mean_map[i] - mean_map[i + kernel_tar * height - kernel_tar];
		neighbor[7] = mean_map[i] - mean_map[i - kernel_tar];

		for (int cur = 0; cur < 8; cur++) {
			if (neighbor[cur] < 0) neighbor[cur] = 0;
		}

		neighbors[0] = mean_map[i] - mean_map[i - kernel_tars * height - kernel_tars];
		neighbors[1] = mean_map[i] - mean_map[i - kernel_tars * height];
		neighbors[2] = mean_map[i] - mean_map[i - kernel_tars * height + kernel_tars];
		neighbors[3] = mean_map[i] - mean_map[i + kernel_tars];
		neighbors[4] = mean_map[i] - mean_map[i + kernel_tars * height + kernel_tars];
		neighbors[5] = mean_map[i] - mean_map[i + kernel_tars * height];
		neighbors[6] = mean_map[i] - mean_map[i + kernel_tars * height - kernel_tars];
		neighbors[7] = mean_map[i] - mean_map[i - kernel_tars];

		for (int cur = 0; cur < 8; cur++) {
			if (neighbor[cur] < 0) neighbor[cur] = 0;
		}
		int min = INT_MAX;
		float min_contra = 10.0;
		for (int k = 0; k < 4; k++) {
			int tem = (int)neighbor[k] * neighbor[k + 4];
			if (min > tem) {
				min = tem;
				if (neighbor[k] == 0 || neighbor[k + 4] == 0) min_contra = 0;
				else {
					min_contra = neighbors[k] > neighbors[k + 4] ? (float)neighbors[k + 4] / neighbors[k] : (float)neighbors[k] / neighbors[k + 4];
					min_contra = min_contra == 1 ? 0.999 : min_contra;
				}
			}
		}
		//求对比度最小值、八个方向
		min_contra = 10.0;
		for (int k = 0; k < 4; k++) {
			if (neighbor[k] == 0 || neighbor[k + 4] == 0) {
				min_contra = 0;
				break;
			}
			else {
				float tem = neighbors[k] > neighbors[k + 4] ? (float)neighbors[k + 4] / neighbors[k] : (float)neighbors[k] / neighbors[k + 4];
				min_contra = min_contra < tem ? min_contra : tem;
			}
		}
		min = min / 16;
		//mpcm_map[i] = min > hard_thresh ? min : 0;
		mpcm_map[i] = min > 32767 ? 32767 : min;
		mpcm_contra[i] = min_contra;
		mpcm_map[i] = min_contra > contra_thresh ? mpcm_map[i] : 0;//直接处理特征值
		//把边缘的特征值置零
		if (i % height > height - delete_width || i % height < delete_width) mpcm_map[i] = 0;
	}
}

void folder_detect() {//检测文件夹下的raw文件
	string folderPath = FOLDER;

	for (const auto& entry : fs::directory_iterator(folderPath)) {
		if (fs::is_regular_file(entry) && entry.path().extension() == ".raw") {
			single_raw_detect(entry.path().string());
		}
	}

	return;
}

void single_raw_detect(string&& path) {
	int frame = 0;
	for (auto& ch : path) {//转义字符
		if (ch == '\\') ch = '/';
	}
	//cout << path << endl;
	string savePath = path.substr(0, path.size() - 4);
	savePath += "detectResult";
	if (FILTE) savePath += "_FILTE";
	else savePath += "_NO_FILTE";
	/*if (!fs::exists(savePath)) {
		fs::create_directories(savePath);
	}*/
	int height = H, width = W;
	FILE* fp = fopen(path.c_str(), "rb");
	fseek(fp, 0, SEEK_END);
	long long file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	unsigned short* data = new unsigned short[height * width]();
	unsigned short* mean_map = new unsigned short[height * width]();
	unsigned short* mpcm_map = new unsigned short[height * width]();
	unsigned short* tem_mpcm_map = new unsigned short[height * width]();

	float* mpcm_contra = new float[height * width]();

	int num[nums_target] = { 0 };//候选个数:100
	unsigned short eigen_value[nums_target] = { 0 };//特征值
	float eigen_contra[nums_target] = { 0.0 };//对称系数
	int count = nums_target;
	cout << "frameth: " << endl;
	int frameth = 0;
	std::cin >> frameth;
	frame = frameth;
	char key = 'z';
	while (frameth--) {
		fread(data, sizeof(unsigned short), height * width, fp);
	}
	while (!feof(fp)) {//未到raw文件结尾
		long long cur_size = ftell(fp);
		if (file_size - cur_size < (long long)height * width * 2) return;
		memset(eigen_value, 0, sizeof(unsigned short) * nums_target);
		memset(num, 0, sizeof(int) * nums_target);
		memset(eigen_contra, 0, sizeof(float) * nums_target);
		count = nums_target;

		if (key == 'e') {
			frame--;
			if (frame % 100 == 1)
				cout << frame << endl;
			fseek(fp, -2 * sizeof(unsigned short) * height * width, SEEK_CUR);
			fread(data, sizeof(unsigned short), height * width, fp);
		}
		else {
			frame++;
			if (frame % 100 == 1)
				cout << frame << endl;
			fread(data, sizeof(unsigned short), height * width, fp);
		}

		for (int i = 0; i < height * width; i++) {//大小端转换
			unsigned short tem = 0;
			tem = data[i];
			data[i] = (tem & 0x00ff) << 8 | (tem & 0xff00) >> 8;
		}
		for (int i = 0; i < height * width; i++) {//替换命令行
			if (i < 3 * height) data[i] = data[i + 3 * height];
			if (i > height * width - 3 * height) data[i] = data[i - 3 * width];
		}

		mpcm_cal(data, mean_map, mpcm_map, mpcm_contra, height, width, KER_MEAN, KER_TAR, contra_thresh, hard_thresh);//mean, tar
		memcpy(tem_mpcm_map, mpcm_map, sizeof(unsigned short) * height * width);
		int occur_of_100 = 0;

		while (count--) {
			int max = 0, max_index = 0;
			for (int i = 0; i < height * width; i++) {//找到最大的
				if (mpcm_map[i] > max) {
					max = mpcm_map[i];
					max_index = i;
				}
			}
			int sum_eigen = 0;
			int tem_index = 0;
			int sub = 0;

			//for (int i = 0; i < 8; i++) {
			//	int index = max_index + (eight_elements[i][0] * (KER_TAR + 5)) * height + eight_elements[i][1] * (KER_TAR + 5);
			//	//(x + eight_elements[i][0] * ker_tar) * height + y + eight_elements[i][1] * ker_tar;
			//	sum_eigen += data[index];
			//}
			//sum_eigen /= 8;
			//sub = data[max_index] - sum_eigen;
			//sub_array[nums_target - count] = sub;
			//cout << "count:" << count << "sum_eigen=" << sub << endl;

			//for (int row = -20; row <= 20; row++) {
			//	for (int col = -20; col <= 20; col++) {
			//		tem_index = max_index + row * height + col;
			//		if (tem_index <= 0 || tem_index > height * width - 1) continue;
			//		if (row > -10 && row<10 && col>-10 && col < 10)continue;
			//		if (abs(mean_map[max_index] - mean_map[tem_index]) <= 25) {
			//			isCloud = true;
			//			break;
			//		}
			//	}
			//	if (isCloud)break;
			//}
			for (int row = -10; row <= 10; row++) {//最大值周围置零
				for (int col = -10; col <= 10; col++) {
					int index = max_index + row * height + col;
					if (index<0 || index>height * width - 1)continue;
					mpcm_map[index] = 0;
				}
			}
			//if (!isCloud)
			{
				num[occur_of_100] = max_index;
				eigen_value[occur_of_100] = max;
				eigen_contra[occur_of_100] = mpcm_contra[max_index];
				occur_of_100++;
			}
		}
		if (frame != 1 && NEIGHBOR_DELETE) {
			check(num, height, width);
		}

		memcpy(pre_num7, num, sizeof(int) * nums_target);
		res.clear();
		res.reserve(nums_target);
		memset(not_rec_flag, 0, sizeof(bool) * nums_target);
		for (int i = 0; i < nums_target; i++) {//把num加入vector<int> res
			res.push_back(num[i]);
		}
		if (same_position_six_frames_delete) {
			for (int i = 0; i < nums_target; i++)
			{
				if (is_same_position(res[i], pre_six_frames, 42, count_for_same_position)) not_rec_flag[i] = 1;
				insert_six_frames_position(res[i]);
			}
		}
		for (int i = 0; i < nums_target; i++) {
			eigen_value[i] = eigen_value[i] > hard_thresh ? eigen_value[i] : 0;
		}
		int tem_index = 0;
		int none = 0;
		/***********算背景均值*************/
		int numgrey;
		for (int i = 0; i < nums_target; i++)
		{
			numgrey = 0;
			int y = res[i] % height, x = res[i] / height;
			if (eigen_value[i] != 0) {
				int sum_eigen = 0;
				for (int i = 0; i < 8; i++) {
					int index = (x + eight_elements[i][0] * mean_radius) * height + y + eight_elements[i][1] * mean_radius;
					if (index > 0 && mean_map[index] != 0 && index < (HEIGHT * WIDTH)) {
						sum_eigen += mean_map[index];
						numgrey++;
					}
				}

				grey[i] = sum_eigen / numgrey;
			}
		}
		cout << "********************特征值筛选**********************" << endl;
		for (int i = 0; i < nums_target; i++) {
			int y = res[i] % height, x = res[i] / height;
			if (eigen_value[i] != 0)
			{
				cout << i << ":   y: " << setw(5) << y << "   x: " << setw(5) << x << "   eigen:  " << setw(5) << eigen_value[i] << "   contra:  " << setw(5) << eigen_contra[i] << "   grey:  " << grey[i] << setw(5) << "sub" << sub_array[i] << "center" << setw(6) << mean_map[res[i]] << endl;

				//for (int row = -1; row <= 1; row++) {
				//	for (int col = -1; col <= 1; col++) {
				//		cout << setw(5) << mpcm_map[points[i] + row * height + col] << "  ";
				//	}
				//	cout << endl;
				//}
			}
		}

		key = vision_16bit(tem_mpcm_map, savePath, frame, data, height, width, mean_map, res, hard_thresh, eigen_value, eigen_contra, KER_TAR);
	}
	fclose(fp);
	delete[] data;
	delete[] mean_map;
	delete[] mpcm_map;
	delete[] tem_mpcm_map;
	return;
}

char vision_16bit(unsigned short* mpcm_map, string& savePath, int frame, unsigned short* data, int height, int width, unsigned short* mean_map, vector<int>& points, unsigned short hard_thresh, unsigned short* eigen_value = nullptr, float* eigen_contra = nullptr, int ker_tar = 3) {//项目的图是躺着的，所以转换操作
	Mat res = Mat::zeros(height, width, CV_8UC1);
	int max_val = 0, min_val = 65535;
	int* arr = new int[height * width]();
	int orderLine = 1;
	if (height == 320) orderLine = 3;//命令行
	for (int i = orderLine * height + 1; i < height * width - orderLine * height; i++) {
		if (data[i] < min_val) min_val = data[i];
		if (data[i] > max_val) max_val = data[i];//找到最大最小值
		int tem = (int)data[i];
		arr[tem]++;//为了截断量化
	}
	int min_ = INT_MAX, max_ = 0, total = 0;
	for (int i = 0; i <= 65535; i++) {//截断量化
		total = total + (int)arr[i];
		if (total < orderLine * height + 65) min_val = i;
		if (total < height * width - 2 * orderLine * height - 65) max_val = i;
	}

	if (max_val == min_val || min_ == max_) return waitKey(0);
	int num = 0;
	int min = INT_MAX, max = 0;
	for (int col = 0; col < width; col++) {
		for (int row = 0; row < height; row++) {
			res.at<uchar>(row, col) = (data[num] - min_val) * 255 / (max_val - min_val);
			num++;
			if (data[num] > max) max = data[num];
			if (data[num] < min) min = data[num];
		}
	}
	delete[] arr;
	cout << "frame: " << frame << endl;

	//*这里是硬阈值过滤后直接画图*//
	//if (!points.empty()) {
	//	for (int i = 0; i < points.size(); i++) {
	//		if (eigen_value[i] != 0 && eigen_contra[i] > contra_threshs) {
	//			if (eigen_value[i] < hard_thresh || eigen_contra[i] < contra_thresh || not_rec_flag[i]) continue;
	//			int y = points[i] / height, x = points[i] % height;
	//			int sum = 0;
	//			for (int row = -1; row <= 1; row++) {
	//				for (int col = -1; col <= -1; col++) {
	//					sum += (int)res.at<uchar>(x + row, y + col);
	//				}
	//			}
	//			int average = sum / 9;
	//			auto color = average > 75 ? Scalar(0, 0, 0) : Scalar(255, 255, 255);
	//			rectangle(res, Point(y - 10, x - 10), Point(y + 10, x + 10), color, 1);
	//			string tem1 = to_string(i);/* + "  " + to_string((int)eigen_value[i]);*/
	//			//string tem2 = " 0." + to_string((int)(eigen_contra[i] * 10000));
	//			putText(res, tem1, Point(y - 10, x - 10), 1, 1, color);
	//			//putText(res, tem2, Point(y - 10, x + 20), 1, 1, Scalar(255, 255, 255));
	//		}
	//	}
	//}

	/*这里是去云虚警*/
	//int tem_index = 0, none = 0;
	//for (int i = 0; i < nums_target; i++) {
	//	for (int row = -box; row <= box; row += stride) {
	//		for (int col = -box; col <= box; col += stride) {
	//			tem_index = points[i] + row * height + col;
	//			if (tem_index <= 0 || tem_index > height * width - 1) continue;
	//			if (row > -box / 2 && row<box / 2 && col>-box / 2 && col < box / 2)continue;
	//			if (abs(mean_map[points[i]] - mean_map[tem_index]) <= boxthreshold) {
	//				//isCloud = true;
	//				eigen_value[i] = 0;
	//				none++;
	//			}
	//			if (none >= 4) {
	//				isCloud = true;
	//				break;
	//			}
	//			if (isCloud)break;
	//		}
	//		if (isCloud)break;
	//	}
	//}

	//cout << "********************特征值筛选**********************" << endl;
	//for (int i = 0; i < nums_target; i++) {
	//	int y = points[i] % height, x = points[i] / height;
	//	if (eigen_value[i] != 0)
	//	{
	//		cout << i << ":   y: " << setw(5) << y << "   x: " << setw(5) << x << "   eigen:  " << setw(5) << eigen_value[i] << "   contra:  " << setw(5) << eigen_contra[i] << "   grey:  " << grey[i] << setw(5) << "sub" << sub_array[i] << endl;
	//		//for (int row = -1; row <= 1; row++) {
	//		//	for (int col = -1; col <= 1; col++) {
	//		//		cout << setw(5) << mpcm_map[points[i] + row * height + col] << "  ";
	//		//	}
	//		//	cout << endl;
	//		//}
	//	}
	//}

	int min_value = 65536, min1 = 0, min2 = 0, min_index = 0, temp_value, temp_eigen, temp_contra;
	int sequent[nums_target] = { 0 };
	ushort greycopy[nums_target] = { 0 };
	memcpy(greycopy, grey, sizeof(ushort) * nums_target);
	for (int i = 0; i < nums_target; i++)
	{
		min_value = 65535;
		min_index = nums_target;
		for (int j = 0; j < nums_target; j++)
		{
			if (greycopy[j] != 0)
			{
				if (min_value > greycopy[j])
				{
					min_value = greycopy[j];
					min_index = j;
				}
			}
			else continue;
		}
		if (min_index != nums_target)
		{
			//min_value = grey[min_index];
			//grey[min_index] = grey[i];
			//grey[i] = min_value;
			//temp_eigen = eigen_value[min_index];
			//eigen_value[min_index] = eigen_value[i];
			//eigen_value[i] = temp_eigen;
			//temp_contra = eigen_contra[min_index];
			//eigen_contra[min_index] = eigen_contra[i];
			//eigen_contra[i] = temp_contra;
			sequent[i] = min_index;
			greycopy[min_index] = { 0 };
		}
	};
	for (int i = report; i < nums_target; i++) {
		sequent[i] = 0;
	}
	int target_have = 0, flag = 0;
	for (int i = 0; i < report; i++) {
		if (flag == 0 || sequent[i] != 0)
		{
			if (eigen_value[sequent[i]] != 0) {
				target_have++;
			}
			if (sequent[i] == 0)flag = 1;
		}
	}
	int tempValue = 0, centerValue = 0;

	//for (int i = 0; i < nums_target; i++)
	//{
	//	if (grey[i] != 0)
	//	{
	//		for (int j = i; j < nums_target; j++)
	//		{
	//			if (grey[j] != 0)
	//			{
	//				if (min_value > grey[j])
	//				{
	//					min_value = grey[j];
	//					grey[j] = grey[i];
	//					grey[i] = j;
	//					temp_eigen = eigen_value[j];
	//					eigen_value[j] = eigen_value[i];
	//					eigen_value[i] = temp_eigen;
	//					temp_contra = eigen_contra[j];
	//					eigen_contra[j] = eigen_contra[i];
	//					eigen_contra[i] = temp_contra;
	//				}
	//			}
	//			else continue;
	//		}
	//	}
	//	else continue;
	//};
	ushort column[HEIGHT] = { 0 };
	int grad[HEIGHT] = { 0 };
	int left = 0, right = 0;
	long sumcol = 0, sumrow = 0;
	int mean_col = 0, mean_row = 0;
	long hisgram[65536] = { 0 };
	ushort colgrey[HEIGHT] = { 0 };

	if (target_have < 4) {
		cout << "*********************特征值筛选出来的点小于四个*********************" << endl;
		//for (int i = 0; i < target_have; i++)
		//{
		//	int y = points[sequent[i]] % height, x = points[sequent[i]] / height;
		//	if (eigen_value[sequent[i]] != 0)
		//	{
		//		if (flag == 0 || sequent[i] != 0)
		//		{
		//			//*这里是在灰度值排序中进行去云虚警*//
		//			int tem_index = 0, none = 0;
		//			isCloud = false;
		//			for (int row = -box; row <= box; row += stride)
		//			{
		//				for (int col = -box; col <= box; col += stride)
		//				{
		//					tem_index = points[sequent[i]] + row * height + col;
		//					if (tem_index <= 0 || tem_index > height * width - 1) continue;
		//					if (row > -box / 2 && row<box / 2 && col>-box / 2 && col < box / 2)continue;
		//					if (abs(mean_map[points[sequent[i]]] - mean_map[tem_index]) <= boxthreshold)
		//						//if ((mean_map[points[sequent[i]]] - mean_map[tem_index]) <= 0)
		//					{
		//						none++;
		//					}
		//					if (none >= 120)
		//					{
		//						isCloud = true;
		//						eigen_value[sequent[i]] = 0;
		//						break;
		//					}
		//				}
		//				if (isCloud)break;
		//			}
		//			if (isCloud)break;
		//		}
		//	}
		//}
		if (!points.empty())
		{
			for (int i = 0; i < points.size(); i++)
			{
				if (eigen_value[i] != 0 && eigen_contra[i] > contra_threshs)
				{
					if (eigen_value[i] < hard_thresh || eigen_contra[i] < contra_thresh || not_rec_flag[i]) continue;
					int y = points[i] / height, x = points[i] % height;
					int sum = 0;
					for (int row = -1; row <= 1; row++) {
						for (int col = -1; col <= -1; col++) {
							sum += (int)res.at<uchar>(x + row, y + col);
						}
					}
					int average = sum / 9;
					auto color = average > 75 ? Scalar(0, 0, 0) : Scalar(255, 255, 255);
					rectangle(res, Point(y - 10, x - 10), Point(y + 10, x + 10), color, 1);
					string tem1 = to_string(i);/* + "  " + to_string((int)eigen_value[i]);*/
					//string tem2 = " 0." + to_string((int)(eigen_contra[i] * 10000));
					putText(res, tem1, Point(y - 10, x - 10), 1, 1, color);
					//putText(res, tem2, Point(y - 10, x + 20), 1, 1, Scalar(255, 255, 255));
				}
			}
		}
	}
	else
	{
		cout << "*********************灰度值排序剩下前四个然后去云虚警*********************" << endl;
		flag = 0;
		for (int i = 0; i < report; i++) {
			sumrow = 0; sumcol = 0;
			int y = points[sequent[i]] % height, x = points[sequent[i]] / height;
			if (eigen_value[sequent[i]] != 0)
			{
				if (flag == 0 || sequent[i] != 0) {
					//*这里是在灰度值排序中进行去云虚警*//
					int tem_index = 0, none = 0;
					isCloud = false;
					//				for (int i = 0; i < nums_target; i++)
					{
						for (int row = -box; row <= box; row += stride)
						{
							for (int col = -box; col <= box; col += stride)
							{
								tem_index = points[sequent[i]] + row * height + col;
								if (tem_index <= 0 || tem_index > height * width - 1) continue;
								if (row > -box / 2 && row<box / 2 && col>-box / 2 && col < box / 2)continue;
								if (abs(mean_map[points[sequent[i]]] - mean_map[tem_index]) <= boxthreshold)
									//if ((mean_map[points[sequent[i]]] - mean_map[tem_index]) <= 0)
								{
									//isCloud = true;
									//eigen_value[i] = 0;
									none++;
								}
								if (none >= none_nums)
								{
									isCloud = true;
									break;
								}
							}
							if (isCloud)break;
						}
						if (isCloud)break;
					}
					if (isCloud);
					else if (eigen_contra[sequent[i]] > contra_fina)
					{
						int y = points[sequent[i]] / height, x = points[sequent[i]] % height;
						int sum = 0;
						for (int row = -1; row <= 1; row++) {
							for (int col = -1; col <= -1; col++) {
								sum += (int)res.at<uchar>(x + row, y + col);
							}
						}
						int average = sum / 9;
						auto color = average > 75 ? Scalar(0, 0, 0) : Scalar(255, 255, 255);
						rectangle(res, Point(y - 10, x - 10), Point(y + 10, x + 10), color, 1);
						string tem1 = to_string(sequent[i]);/* + "  " + to_string((int)eigen_value[i]);*/
						//string tem2 = " 0." + to_string((int)(eigen_contra[i] * 10000));
						putText(res, tem1, Point(y - 10, x - 10), 1, 1, color);
						////putText(res, tem2, Point(y - 10, x + 20), 1, 1, Scalar(255, 255, 255))
						cout << sequent[i] << ":   y: " << setw(5) << points[sequent[i]] % height << "   x: " << setw(5) << points[sequent[i]] / height << "   eigen:  " << setw(5) << eigen_value[sequent[i]] << "   contra:  " << setw(5) << eigen_contra[sequent[i]] << "   grey:  " << grey[sequent[i]] << endl;
					}
					// << setw(10) << "meancolmin" << meancolmin << setw(10) << "meancolmax" << meancolmax << endl;
					if (sequent[i] == 0)
						flag = 1;
				}//for (int row = -1; row <= 1; row++) {
				//	for (int col = -1; col <= 1; col++) {
				//		cout << setw(5) << mpcm_map[points[i] + row * height + col] << "  ";
				//	}
				//	cout << endl;
				//}
			}
		}
	}
	for (int i = 0; i < nums_target; i++) {
		if (grey[i] != 0) {
			if (min_value > grey[i]) {
				min_value = grey[i];
				min1 = i;
			}
		}
	}
	min_value = 65536;
	//grey[min1] = 65536;

	for (int i = 0; i < nums_target; i++) {
		if (grey[i] != 0 && i != min1) {
			if (min_value > grey[i]) {
				min_value = grey[i];
				min2 = i;
			}
		}
	}
	for (int i = 0; i < nums_target; i++) {
		if (i != min1 && i != min2) {
			eigen_value[i] = 0;
		}
	}
	cout << "******************************************" << endl;
	/********************************/
	cout << endl << endl << endl;
	std::ostringstream oss;
	oss << savePath << "/" << std::setfill('0') << std::setw(3) << frame << ".png";
	std::string savePath_ = oss.str();
	//imwrite(savePath_, res);
	memset(grey, 0, sizeof(int) * nums_target);
	imshow("res", res);

	return waitKey(0);
}

void check(int* arr, int height, int width) {//前后帧做关联
	res.clear();
	int tem = 0;
	for (int i = 0; i < nums_target; i++) {
		for (int j = 0; j < nums_target; j++) {
			tem = arr[i] - pre_num7[j];
			if (abs(tem) % height < 10 || abs(tem) % height >(height - 10) && abs(tem) / height < 10) {
				res.push_back(arr[i]);
			}
		}
	}
}

bool is_same_position(int index, int* ref, int num, int need) {
	int count = 0;
	for (int i = 0; i < num; i++)
	{
		if (index == ref[i]) count++;
	}
	return count >= need;
}
void insert_six_frames_position(int index) {
	/*if (!is_same_position(index, pre_six_frames, 42, 1)) {
		pre_six_frames[six_frames_positions_index] = index;
		six_frames_positions_index++;
		six_frames_positions_index %= 42;
	}*/
	pre_six_frames[six_frames_positions_index] = index;
	six_frames_positions_index++;
	six_frames_positions_index %= 42;
}