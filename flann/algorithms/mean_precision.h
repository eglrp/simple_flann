/*
**@file  mean_precision.h		����ƽ�����ȵ�Դ�ļ�
**
**@author zhaoshuai 
*/


#ifndef _MEAN_PRECISION_H_
#define _MEAN_PRECISION_H_

#include "../util/matrix.h"

namespace flann
{
	/*
	**@brief	�ɲ�ѯ���õĽ��ڶԱ���ʵ�Ľ����ò�ѯ��ƽ������
	**
	*/
	template<typename T>
	float mean_precision(Matrix<T>& indices, Matrix<T>& ground_truth)
	{
		size_t i = 0, j = 0, k = 0;
		float right_num = 0;
		unsigned char right_flag = 0;
		T* indices_p = indices.ptr();
		T* ground_truth_p = ground_truth.ptr();

		for (i = 0; i < indices.rows; i++)
		{
			for (j = 0; j < indices.cols; j++)
			{
				
				for (k = 0; k < indices.cols; k++)
				{
					if (*(indices_p + i*indices.cols + j) == *(ground_truth_p + i*ground_truth.cols + k))
					{
						right_flag = 1;
						k = indices.cols;												//�ҵ����˳�
					}
					else
						right_flag = 0;
				}
				if ( right_flag==1 )
					right_num += 1;
				right_flag = 0;
			}
		}
		float mp = right_num / (indices.rows*indices.cols);
		return(mp);
	}

}


#endif	//MEAN_PRECISION_H_