/*
**@brief	LSH��KDTrees����Ҫ�����ļ�
**
**@author	zhaoshuai
*/



#include "flann/flann.hpp"
//#include "flann/io/hdf5.h"											//ȥ��hdf5�ļ��Ķ�д��
#include "flann/io/save_result_txt.h"
#include "flann/io/dataset_read.h"
#include "flann/algorithms/lsh_index.h"
#include "flann/algorithms/mean_precision.h"

#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <windows.h>

/*Ϊ0ѡ��kdtree��ʽ��Ϊ1ѡ��lsh��ʽ*/
#define LSH_INDEX_ENABLE (1)		
/*Ϊ0ѡ��SIFT���ݼ���Ϊ1ѡ��GIST���ݼ�*/
//#define SIFT_GIST_CHOOSE	(0)

/*SIFT���ݼ��Ķ���*/
#define ANN_SIFT1M_BASE_NUM (1000000)
#define ANN_SIFT1M_QUERY_NUM (1000)
#define ANN_SIFT1M_GROUND_TRUTH_NUM (1000)

/*GIST���ݼ��Ķ���*/
#define ANN_GIST1M_BASE_NUM_1 (350000)									//�ڴ����ޣ��ֳ����ζ�ȡ
#define ANN_GIST1M_BASE_NUM_2 (700000)
#define ANN_GIST1M_BASE_NUM_3 (1000000)
#define ANN_GIST1M_QUERY_NUM (1000)
#define ANN_GIST1M_GROUND_TRUTH_NUM (1000)

using namespace flann;




/*
**@brief	kd-tree��SIFT1M���ݼ��µĲ��Ժ���
**
**@params	knn			-�����Ľ��ڸ���
**			tree_num	-Ҫ����ʱҪʹ�õ����ĸ���
*/
void test_kdtree_sift(
	size_t knn,
	int tree_num)
{

	Matrix<float> dataset;
	Matrix<float> query;
	Matrix<size_t> ground_truth;

	DWORD start_time, end_time, spend_time;
	float mean_precision = 0;

	/*��ȡ������Ϣ*/
	flann::fvecs_ivecs_read(dataset, "dataset/sift/sift_base.fvecs", 1, ANN_SIFT1M_BASE_NUM);
	flann::fvecs_ivecs_read(query, "dataset/sift/sift_query.fvecs", 1, ANN_SIFT1M_QUERY_NUM);
	flann::fvecs_ivecs_read(ground_truth, "dataset/sift/sift_groundtruth.ivecs", 1, ANN_SIFT1M_GROUND_TRUTH_NUM);


	Matrix<size_t> indices(new size_t[query.rows*knn], query.rows, knn);
	Matrix<float> dists(new float[query.rows*knn], query.rows, knn);


	start_time = GetTickCount();
	std::cout << std::endl;
	std::cout << "KD-TREE��ʽ����,����" << tree_num << "����" << std::endl;
	Index<L2<float>> index(dataset, flann::KDTreeIndexParams(tree_num));					//FLANN_INDEX_KDTREE
	index.buildIndex();
	end_time = GetTickCount();
	std::cout << "�����������" << std::endl;
	std::cout << "������������ʱ��: " << end_time - start_time << "ms" << std::endl;
	// do a knn search, using 128 checks
	start_time = GetTickCount();
	index.knnSearch(query, indices, dists, knn, flann::SearchParams(128));
	end_time = GetTickCount();
	std::cout << "��ѯ���" << std::endl;
	std::cout << "��ѯ����ʱ��: " << end_time - start_time << "ms" << std::endl;
	std::cout << "��ѯ�����ƽ�����ȣ�";
	mean_precision = flann::mean_precision(indices, ground_truth);
	std::cout << std::setprecision(4) << mean_precision * 100 << "%" << std::endl;
	std::cout << "�ڴ濪��:" << index.usedMemory()<<"Bytes"<<std::endl;
	std::cout << std::endl;
	/*ɾ��new�Ŀռ�*/
	delete[] indices.ptr();
	delete[] dists.ptr();
	delete[] query.ptr();
	delete[] ground_truth.ptr();
	delete[] dataset.ptr();
	return;
}


/*
**@brief	LSH��SIFT1M���ݼ��µĲ��Ժ���
**
**@params	knn				-�����Ľ��ڸ���
**			table_number	-�õĹ�ϣ�����
**			key_size		-�ؼ��ֵĸ�������һ�ι�ϣ��õ�������ά����
**			gap_w			-�ָ���
*/
void test_lsh_sift(
	size_t knn,
	unsigned int table_number,
	unsigned int key_size,
	float gap_w)
{

	Matrix<float> dataset;
	Matrix<float> query;
	Matrix<size_t> ground_truth;

	DWORD start_time, end_time, spend_time;
	float mean_precision = 0;
	/*��ȡ������Ϣ*/
	std::cout << std::endl;
	flann::fvecs_ivecs_read(dataset, "dataset/sift/sift_base.fvecs", 1, ANN_SIFT1M_BASE_NUM);
	flann::fvecs_ivecs_read(query, "dataset/sift/sift_query.fvecs", 1, ANN_SIFT1M_QUERY_NUM);
	flann::fvecs_ivecs_read(ground_truth, "dataset/sift/sift_groundtruth.ivecs", 1, ANN_SIFT1M_GROUND_TRUTH_NUM);


	Matrix<size_t> indices(new size_t[query.rows*knn], query.rows, knn);
	Matrix<float> dists(new float[query.rows*knn], query.rows, knn);

	std::cout << std::endl;
	std::cout << "LSH��ʽ����" << std::endl;
	std::cout << "��ϣ����:" << table_number << "\t";
	std::cout << "�ؼ��ָ���:" << key_size << "\t";
	std::cout << std::setprecision(4) << "�ָ���:" << gap_w << "\t";

	start_time = GetTickCount();
	/*����*/
	unsigned int table_size = dataset.rows;

	LshIndexParams lsh_param(table_number, key_size, gap_w, table_size);
	LshIndex<L2<float>> lsh_index(dataset, lsh_param, flann::L2<float>());
	lsh_index.buildIndex();

	end_time = GetTickCount();
	std::cout << "�����������" << std::endl;
	std::cout << "������������ʱ��: " << end_time - start_time << "ms" << std::endl;
	start_time = GetTickCount();
	lsh_index.knnSearch(query, indices, dists, knn);
	end_time = GetTickCount();
	std::cout << "��ѯ���" << std::endl;
	std::cout << "��ѯ����ʱ��: " << end_time - start_time << "ms" << std::endl;
	std::cout << "��ѯ�����ƽ�����ȣ�";
	mean_precision = flann::mean_precision(indices, ground_truth);
	std::cout << std::setprecision(4) << mean_precision * 100 << "%" << std::endl;
	std::cout << "�ڴ濪��:" << lsh_index.usedMemory() << "KB" << std::endl;
	//std::cout << std::endl;
	std::cout << "����������:" << lsh_index.get_m_distance_cnt() << "��" << std::endl;
	std::cout << std::endl;

	delete[] indices.ptr();
	delete[] dists.ptr();
	delete[] query.ptr();
	delete[] ground_truth.ptr();
	delete[] dataset.ptr();
	return;

}

/*
**@brief ����gist���ݼ����ĺϲ�����
*/
void gist_combine(
	Matrix<size_t>& indices,
	Matrix<float>& dists,
	Matrix<size_t>& indices_1,
	Matrix<float>& dists_1,
	Matrix<size_t>& indices_2,
	Matrix<float>& dists_2
	)
{
	int cols_1 = indices.cols;
	int cols_2 = 2 * indices.cols;
	int cols_3 = 3 * indices.cols;
	int i = 0, j = 0, k = 0;
	size_t* res_indice = new size_t[cols_3];						//�������ÿռ�
	float* res_dist = new float[cols_3];							//���뱸�ÿռ�
	size_t i_temp = 0;
	float d_temp = 0;

	for (i = 0; i < indices.rows; i++)
	{
		for (j = 0; j < cols_1; j++)								//���Ƶ�һ���ռ���
		{
			*(res_indice + j) = *(indices[i] + j);
			*(res_dist + j) = *(dists[i] + j);
		}
		for (j = cols_1; j < cols_2; j++)
		{
			*(res_indice + j) = *(indices_1[i] + j - cols_1);
			*(res_dist + j) = *(dists_1[i] + j - cols_1);
		}
		for (j = cols_2; j < cols_3; j++)
		{
			*(res_indice + j) = *(indices_2[i] + j - cols_2);
			*(res_dist + j) = *(dists_2[i] + j - cols_2);
		}

		for (j = cols_3 - 1; j>cols_2 - 1; j--)						//ð�ݣ�����С��cols_1����ð��ǰ������ͬʱ������־
		{
			for (k = j; k > 0; k--)
			{
				if (*(res_dist + k) < *(res_dist + k - 1))
				{
					//��������
					d_temp = *(res_dist + k);
					*(res_dist + k) = *(res_dist + k - 1);
					*(res_dist + k - 1) = d_temp;
					//��������
					i_temp = *(res_indice + k);
					*(res_indice + k) = *(res_indice + k - 1);
					*(res_indice + k - 1) = i_temp;
				}
			}
		}
		for (j = 0; j < cols_1; j++)						//ð�ݵĵĽ�����Ƶ�ԭ���洢�ռ���
		{
			*(indices[i] + j) = *(res_indice + j);
			*(dists[i] + j) = *(res_dist + j);
		}

	}
	delete[]res_indice;
	delete[]res_dist;
	return;
}


/*
**@brief	kd-tree��GIST1M���ݼ��µĲ��Ժ���
**
**@params	knn			-�����Ľ��ڸ���
**			tree_num	-Ҫ����ʱҪʹ�õ����ĸ���
*/
void test_kdtree_gist(size_t knn,int tree_num)
{
	Matrix<float> dataset;
	Matrix<float> dataset_1;
	Matrix<float> dataset_2;
	Matrix<float> query;
	Matrix<size_t> ground_truth;
	
	std::cout << std::endl;
	flann::fvecs_ivecs_read(query, "dataset/gist/gist_query.fvecs", 1, ANN_GIST1M_QUERY_NUM);
	flann::fvecs_ivecs_read(ground_truth, "dataset/gist/gist_groundtruth.ivecs", 1, ANN_GIST1M_GROUND_TRUTH_NUM);


	Matrix<size_t> indices(new size_t[query.rows*knn], query.rows, knn);
	Matrix<float> dists(new float[query.rows*knn], query.rows, knn);
	Matrix<size_t> indices_1(new size_t[query.rows*knn], query.rows, knn);
	Matrix<float> dists_1(new float[query.rows*knn], query.rows, knn);
	Matrix<size_t> indices_2(new size_t[query.rows*knn], query.rows, knn);
	Matrix<float> dists_2(new float[query.rows*knn], query.rows, knn);

	DWORD start_time, end_time, spend_time;
	DWORD buildtime, buildtime_1, buildtime_2;
	DWORD searchtime, searchtime_1, searchtime_2;
	int mem = 0, mem_1, mem_2;
	float mean_precision = 0;

	
	std::cout << std::endl;
	std::cout << "KD-TREE��ʽ����,����" << tree_num << "����" << std::endl;

	
	/*��������1*/
	flann::fvecs_ivecs_read(dataset, "dataset/gist/gist_base.fvecs", 1, ANN_GIST1M_BASE_NUM_1);
	start_time = GetTickCount();
	Index<L2<float>> index(dataset, flann::KDTreeIndexParams(tree_num));					//FLANN_INDEX_KDTREE
	index.buildIndex();
	end_time = GetTickCount();
	buildtime = end_time - start_time;
	start_time = GetTickCount();
	index.knnSearch(query, indices, dists, knn, flann::SearchParams(128));
	end_time = GetTickCount();
	searchtime = end_time - start_time;
	mem = index.usedMemory();
	delete[] dataset.ptr();
	/*��������2*/
	flann::fvecs_ivecs_read(dataset_1, "dataset/gist/gist_base.fvecs", ANN_GIST1M_BASE_NUM_1, ANN_GIST1M_BASE_NUM_2);
	start_time = GetTickCount();
	//Index<L2<float>> index_1(dataset_1, flann::KDTreeIndexParams(tree_num));					//FLANN_INDEX_KDTREE
	index.buildIndex(dataset_1);
	end_time = GetTickCount();
	buildtime_1 = end_time - start_time;

	start_time = GetTickCount();
	index.knnSearch(query, indices_1, dists_1, knn, flann::SearchParams(128));
	end_time = GetTickCount();
	searchtime_1 = end_time - start_time;

	mem_1 = index.usedMemory(); 
	delete[] dataset_1.ptr();
	/*��������3*/
	flann::fvecs_ivecs_read(dataset_2, "dataset/gist/gist_base.fvecs", ANN_GIST1M_BASE_NUM_2, ANN_GIST1M_BASE_NUM_3);
	start_time = GetTickCount();
	//Index<L2<float>> index_2(dataset_2, flann::KDTreeIndexParams(tree_num));					//FLANN_INDEX_KDTREE
	index.buildIndex(dataset_2);
	end_time = GetTickCount();
	buildtime_2 = end_time - start_time;

	start_time = GetTickCount();
	index.knnSearch(query, indices_2, dists_2, knn, flann::SearchParams(128));
	end_time = GetTickCount();
	searchtime_2 = end_time - start_time;

	mem_2 = index.usedMemory();
	delete[] dataset_2.ptr();

	std::cout << "�����������" << std::endl;
	std::cout << "������������ʱ��: " << (buildtime + buildtime_1 + buildtime_2) << "ms" << std::endl;	
	std::cout << "��ѯ���" << std::endl;
	std::cout << "��ѯ����ʱ��: " << (searchtime + searchtime_1 + searchtime_2) << "ms" << std::endl;
	std::cout << "��ѯ�����ƽ�����ȣ�";
	gist_combine(indices, dists, indices_1, dists_1, indices_2, dists_2);
	mean_precision = flann::mean_precision(indices, ground_truth);
	std::cout << std::setprecision(4) << mean_precision * 100 << "%" << std::endl;
	std::cout << "�ڴ濪��:" << (mem + mem_1 + mem_2) << "Bytes" << std::endl;
	std::cout << std::endl;

	delete[] indices.ptr();
	delete[] indices_1.ptr();
	delete[] indices_2.ptr();
	delete[] dists.ptr();
	delete[] dists_1.ptr();
	delete[] dists_2.ptr();
	delete[] query.ptr();
	delete[] ground_truth.ptr();
	return;
}


/*
**@brief	LSH��GIST1M���ݼ��µĲ��Ժ���
**
**@params	knn				-�����Ľ��ڸ���
**			table_number	-�õĹ�ϣ�����
**			key_size		-�ؼ��ֵĸ�������һ�ι�ϣ��õ�������ά����
**			gap_w			-�ָ���
*/
void test_lsh_gist(
	size_t knn,
	unsigned int table_number,
	unsigned int key_size,
	float gap_w)
{
	
	Matrix<float> dataset;
	Matrix<float> dataset_1;
	Matrix<float> dataset_2;
	Matrix<float> query;
	Matrix<size_t> ground_truth;

	std::cout << std::endl;
	flann::fvecs_ivecs_read(query, "dataset/gist/gist_query.fvecs", 1, ANN_GIST1M_QUERY_NUM);
	flann::fvecs_ivecs_read(ground_truth, "dataset/gist/gist_groundtruth.ivecs", 1, ANN_GIST1M_GROUND_TRUTH_NUM);


	Matrix<size_t> indices(new size_t[query.rows*knn], query.rows, knn);
	Matrix<float> dists(new float[query.rows*knn], query.rows, knn);
	Matrix<size_t> indices_1(new size_t[query.rows*knn], query.rows, knn);
	Matrix<float> dists_1(new float[query.rows*knn], query.rows, knn);
	Matrix<size_t> indices_2(new size_t[query.rows*knn], query.rows, knn);
	Matrix<float> dists_2(new float[query.rows*knn], query.rows, knn);

	DWORD start_time, end_time, spend_time;
	DWORD buildtime, buildtime_1, buildtime_2;
	DWORD searchtime, searchtime_1, searchtime_2;
	int mem = 0, mem_1, mem_2;
	int dis_cnt = 0, dis_cnt_1 = 0, dis_cnt_2 = 0;
	float mean_precision = 0;


	std::cout << std::endl;
	std::cout << "LSH��ʽ����" << std::endl;
	std::cout << "��ϣ����:" << table_number << "\t";
	std::cout << "�ؼ��ָ���:" << key_size << "\t";
	std::cout << std::setprecision(4) << "�ָ���:" << gap_w << std::endl;


	/*��������1*/
	flann::fvecs_ivecs_read(dataset, "dataset/gist/gist_base.fvecs", 1, ANN_GIST1M_BASE_NUM_1);

	start_time = GetTickCount();
	unsigned int table_size = dataset.rows;
	LshIndexParams lsh_param(table_number, key_size, gap_w, table_size);
	LshIndex<L2<float>> lsh_index(dataset, lsh_param, flann::L2<float>());
	lsh_index.buildIndex();
	end_time = GetTickCount();

	buildtime = end_time - start_time;
	start_time = GetTickCount();
	lsh_index.knnSearch(query, indices, dists, knn);
	end_time = GetTickCount();
	searchtime = end_time - start_time;
	mem = lsh_index.usedMemory();
	dis_cnt = lsh_index.get_m_distance_cnt();

	delete[] dataset.ptr();

	/*��������2*/
	flann::fvecs_ivecs_read(dataset_1, "dataset/gist/gist_base.fvecs", ANN_GIST1M_BASE_NUM_1, ANN_GIST1M_BASE_NUM_2);
	start_time = GetTickCount();
	lsh_index.buildIndex(dataset_1);									//�ؽ�
	end_time = GetTickCount();
	buildtime_1 = end_time - start_time;

	start_time = GetTickCount();
	lsh_index.knnSearch(query, indices_1, dists_1, knn);
	end_time = GetTickCount();
	searchtime_1 = end_time - start_time;
	mem_1 = lsh_index.usedMemory();
	dis_cnt_1 = lsh_index.get_m_distance_cnt();

	delete[] dataset_1.ptr();

	/*��������3*/
	flann::fvecs_ivecs_read(dataset_2, "dataset/gist/gist_base.fvecs", ANN_GIST1M_BASE_NUM_2, ANN_GIST1M_BASE_NUM_3);
	start_time = GetTickCount();
	lsh_index.buildIndex(dataset_2);									//�ڶ����ؽ�����
	end_time = GetTickCount();
	buildtime_2 = end_time - start_time;

	start_time = GetTickCount();
	lsh_index.knnSearch(query, indices_2, dists_2, knn);				//����
	end_time = GetTickCount();
	searchtime_2 = end_time - start_time;
	mem_2 = lsh_index.usedMemory();
	dis_cnt_2 = lsh_index.get_m_distance_cnt();

	delete[] dataset_2.ptr();

	/*��������Ϣ*/
	std::cout << "�����������" << std::endl;
	std::cout << "������������ʱ��: " << (buildtime + buildtime_1 + buildtime_2) << "ms" << std::endl;
	std::cout << "��ѯ���" << std::endl;
	std::cout << "��ѯ����ʱ��: " << (searchtime + searchtime_1 + searchtime_2) << "ms" << std::endl;
	std::cout << "��ѯ�����ƽ�����ȣ�";
	gist_combine(indices, dists, indices_1, dists_1, indices_2, dists_2);
	mean_precision = flann::mean_precision(indices, ground_truth);
	std::cout << std::setprecision(4) << mean_precision * 100 << "%" << std::endl;
	std::cout << "�ڴ濪��:" << (mem + mem_1 + mem_2) << "Bytes" << std::endl;
	std::cout << "����������:" << (dis_cnt + dis_cnt_1 + dis_cnt_2) << "��" << std::endl;
	std::cout << std::endl;

	delete[] indices.ptr();
	delete[] indices_1.ptr();
	delete[] indices_2.ptr();
	delete[] dists.ptr();
	delete[] dists_1.ptr();
	delete[] dists_2.ptr();
	delete[] query.ptr();
	delete[] ground_truth.ptr();
	return;
	
}


//int main(int argc, char** argv)
int main( void )
{
    int nn = 3;
	//DWORD start_time, end_time, spend_time;
	//float mean_precision = 0;

	if (LSH_INDEX_ENABLE != 1)															//���÷�LSH����
	{

		/*������1-����SIFT1M���ݼ���kdtree�����ı���,���� - ���ĸ���*/
		
		//test_kdtree_sift(nn, 2);
		//test_kdtree_sift(nn, 4);
		//test_kdtree_sift(nn, 8);
		//test_kdtree_sift(nn, 12);
		//test_kdtree_sift(nn, 16);
		
		
		/*������2-����GIST1M���ݼ���kdtree�����ı��֣����� - ���ĸ���*/
		
		/**
		test_kdtree_gist(nn, 2);
		test_kdtree_gist(nn, 4);
		test_kdtree_gist(nn, 8);
		test_kdtree_gist(nn, 12);
		test_kdtree_gist(nn, 16);
		**/
		
	}
	else
	{

		/*������1-����SIFT1M���ݼ���LSH�����ı���,���� - �ָ���*/
		test_lsh_sift(nn, 5, 4, 50);
		test_lsh_sift(nn, 5, 4, 100);
		test_lsh_sift(nn, 5, 4, 150);
		test_lsh_sift(nn, 5, 4, 200);
		//test_lsh_sift(nn, 5, 4, 250);
		//test_lsh_sift(nn, 5, 4, 300);
		//test_lsh_sift(nn, 5, 4, 350);
		
		/*������2-����SIFT1M���ݼ���LSH�����ı���,���� - ��ϣ����Ŀ*/
		/**
		test_lsh_sift(nn, 2, 4, 200);
		test_lsh_sift(nn, 4, 4, 200);
		test_lsh_sift(nn, 6, 4, 200);
		test_lsh_sift(nn, 8, 4, 200);
		test_lsh_sift(nn,10, 4, 200);
		**/
	
		/*������3-����SIFT1M���ݼ���LSH�����ı���,���� - �ؼ��ֳ���*/
		/**
		test_lsh_sift(nn, 4, 4, 210);
		test_lsh_sift(nn, 4, 8, 210);
		test_lsh_sift(nn, 4, 16, 210);
		test_lsh_sift(nn, 4, 32, 210);
		test_lsh_sift(nn, 4, 64, 210);
		**/

		/*������4-����GIST1M���ݼ���LSH�����ı���,���� - �ָ���*/

		//test_lsh_gist(nn, 3, 4, 1);
		//test_lsh_gist(nn, 3, 4, 1.5);
		//test_lsh_gist(nn, 3, 4, 2);
		//test_lsh_gist(nn, 3, 4, 3);
		//test_lsh_gist(nn, 3, 4, 4);



	}																			
	
	std::cout << "�������" << std::endl;
	std::system("pause");
    return 0;
}

