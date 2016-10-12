/*
**@file  save_result_txt.h
**
**@author zhaoshuai 
*/

#ifndef SAVE_RESULT_TXT_H_
#define SAVE_RESULT_TXT_H_


//#include "save_result_txt.h"
#include "../util/matrix.h"
#include <fstream>

namespace flann
{
	/*
	**@brief ��dataset�洢��filename��ָ�ļ��У�txt��ʽ�Ա�鿴
	*/
	template<typename T>
	void save_result_txt(flann::Matrix<T>& dataset, const std::string& filename)
	{
		//size_t rows = 0, columns = 0;
		size_t i = 0, j = 0;

		//rows = dataset.rows;
		//columns = dataset.cols;

		std::ofstream result;
		result.open(filename, std::ios::trunc);							//�򿪴�д���ļ�
		if (!result)
		{
			std::cerr << "failed to open result.txt" << std::endl;		//��ʧ��
			return;
		}
		T* pointer=dataset.ptr();										//some question,������һ��ת��
		for (i = 0; i < dataset.rows; i++)								//����д��
		{
			for (j = 0; j < dataset.cols; j++)
			{
				result << *pointer << "\t";
				pointer += 1;											//����+1,ע����T*
			}
			result << std::endl;
		}
		result.close();
		pointer=NULL;
		return;
	}
}



#endif //SAVE_RESULT_TXT_H_
