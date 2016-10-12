/***********************************************************************
 * Software License Agreement (BSD License)
 /***********************************************************************
 *	Author: Vincent Rabaud
 *	redefine zhaoshuai
 *************************************************************************/

#ifndef FLANN_LSH_INDEX_H_
#define FLANN_LSH_INDEX_H_

#include <algorithm>
#include <cassert>
#include <cstring>
#include <map>
#include <vector>


#include "../general.h"
//#include "../algorithms/nn_index.h"
#include "../util/matrix.h"
#include "../util/result_set.h"
#include "../util/heap.h"
#include "../util/lsh_table.h"
#include "../util/allocator.h"
#include "../util/random.h"
#include "../util/saving.h"


#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

namespace flann
{

	struct LshIndexParams //: public IndexParams
	{
	public:
		/*default constructor*/
		LshIndexParams(unsigned int table_number_, unsigned int key_size_, float gap_w_, unsigned int table_size_)
			:table_number(table_number_), key_size(key_size_), gap_w(gap_w_), table_size(table_size_) {}
		unsigned int table_number;
		unsigned int key_size;
		float gap_w;
		unsigned int table_size;
	};

	/**
	 * Randomized kd-tree index
	 *
	 * Contains the k-d trees and other information for indexing a set of points
	 * for nearest-neighbor matching.
	 */
	template<typename Distance>
	class LshIndex //: public NNIndex<Distance>
	{
	public:
		typedef typename Distance::ElementType ElementType;
		typedef typename Distance::ResultType DistanceType;
		//typedef float ElementType;
		//typedef float DistanceType;
	private:
		/** The different hash tables */
		std::vector<lsh::LshTable<ElementType> > tables_;
		/*�����������ݣ�points_�а�˳��洢ÿ�������������׵�ַ����СӦΪsize_*/
		/*debug:10.06.15.54 points_��ֵ��ȷ*/
		std::vector<ElementType*> points_;
		/*���ݼ������������ĸ���*/
		size_t size_;
		/*���ݼ���ÿ������������ά��*/
		size_t veclen_;
		/*hash��ĸ���*/
		unsigned int table_number_;
		/** key size */
		unsigned int key_size_;
		/*ͶӰʱ�ļ��*/
		float m_gap_w;
		/*��ϣ��Ĵ�С��һ��ȡ������������*/
		unsigned int m_table_size;
		/*���뺯��*/
		Distance m_distance;
		/*lsh��������*/
		LshIndexParams index_params_;
		/*��һ����������ʱ���ݼ������������ĸ����������ؽ����ݼ�*/
		size_t size_at_build_;
		/*����������*/
		size_t m_distance_cnt;
		/*��������*/
		//Distance distance_;
		/*Ѱ��Ľ��ڸ���*/
		//size_t m_knn;
		/** How far should we look for neighbors in multi-probe LSH */
		//unsigned int multi_probe_level_;
		/** The XOR masks to apply to a key to get the neighboring buckets */
		//std::vector<lsh::BucketKey> xor_masks_;
		//typedef NNIndex<Distance> BaseClass;

	public:

		/** Constructor
		 * @param params parameters passed to the LSH algorithm
		 * @param d the distance used
		 */
		LshIndex(const LshIndexParams& params = LshIndexParams(), Distance d = Distance())
			:m_distance(d), size_(0), size_at_build_(0), veclen_(0), index_params_(params)
		{
			table_number_ = params.table_number;
			key_size_ = params.key_size;
			m_gap_w = params.gap_w;
			m_table_size = params.table_size;
			m_distance_cnt = 0;
			std::cout << "LshIndex��һ�ֹ��캯��" << std::endl;
		}

		/*
		** Constructor
		** @param input_data dataset with the input features
		** @param params parameters passed to the LSH algorithm
		** @param d the distance used
		*/
		LshIndex(Matrix<ElementType>& input_data, const LshIndexParams& params = LshIndexParams(), Distance d = Distance())
			: m_distance(d), size_(0), size_at_build_(0), veclen_(0), index_params_(params)
		{
			table_number_ = params.table_number;
			key_size_ = params.key_size;
			m_gap_w = params.gap_w;
			setDataset(input_data);
			m_distance_cnt = 0;
			m_table_size = size_;
			//std::cout << "LshIndex�ڶ��ֹ��캯��"<< std::endl;
		}

		/*
		**@brief	Constructor,�����е�Index��ʼ��
		*/
		LshIndex(const LshIndex& other) :
			m_distance(other.m_distance),
			m_gap_w(other.m_gap_w),
			size_(other.size_),
			size_at_build_(other.size_at_build_),
			table_number_(other.table_number_),
			key_size_(other.key_size_),
			veclen_(other.veclen_),
			index_params_(other.index_params_),
			//points_(other.points_),
			m_table_size(other.m_table_size)
		{
		}

		LshIndex& operator=(LshIndex other)
		{
			this->swap(other);
			return *this;
		}

		 ~LshIndex() {	freeIndex();	}


		LshIndex* clone() const
		{
			return new LshIndex(*this);
		}

		/*�ͷŵ������ڴ�*/
		void freeIndex()
		{
			tables_.clear();
		
		}

		/*
		**@brief ���ؾ���������
		*/
		size_t get_m_distance_cnt()
		{
			return(m_distance_cnt);
		}

		/*
		**@brief	Builds the index
		*/
		void buildIndex( )
		{
			//freeIndex();
			//cleanRemovedPoints();
			// building index
			buildIndexImpl( );
			size_at_build_ = size_;
		}

		/*
		**@brief	�����ؽ�����
		*/
		void buildIndex(Matrix<ElementType>& input_data)
		{
			setDataset(input_data);
			m_distance_cnt = 0;
			m_table_size = size_;
			buildIndexImpl();
			size_at_build_ = size_;
			m_distance_cnt = 0;
		}


	protected:

		/*
		**@brief	Builds the index
		*/
		void buildIndexImpl( )
		{
			tables_.clear();
			tables_.resize(table_number_);									//���ݹ�ϣ�����Ŀ�����ڴ�
			//std::vector<std::pair<size_t, ElementType*> > features;
			//features.reserve(points_.size());
			//features.resize(size_);
			//features.reserve(size_);										//size_	-������������
			unsigned int i = 0;
			/**
			while (i < points_.size() - 9)									//����Ϊ���������������
			{
				features.push_back(std::make_pair(i, points_[i]));
				features.push_back(std::make_pair(i + 1, points_[i + 1]));
				features.push_back(std::make_pair(i + 2, points_[i + 2]));
				features.push_back(std::make_pair(i + 3, points_[i + 3]));
				features.push_back(std::make_pair(i + 4, points_[i + 4]));
				features.push_back(std::make_pair(i + 5, points_[i + 5]));
				features.push_back(std::make_pair(i + 6, points_[i + 6]));
				features.push_back(std::make_pair(i + 7, points_[i + 7]));
				features.push_back(std::make_pair(i + 8, points_[i + 8]));
				features.push_back(std::make_pair(i + 9, points_[i + 9]));
				i += 5;
			}
			while (i < points_.size())
			{
				features.push_back(std::make_pair(i, points_[i]));
				i++;
			}
			**/
			for (i = 0; i < table_number_; ++i)								//����hash�����Ŀ��Ϊÿ����ѹ���ϣ��Ĺؼ����Լ������������
			{
				//lsh::LshTable<ElementType>& table = tables_[i];
				//table = lsh::LshTable<ElementType>(veclen_, key_size_, m_gap_w, m_table_size);
				tables_[i]=lsh::LshTable<ElementType>(veclen_, key_size_, m_gap_w, m_table_size);
				// Add the features to the table
				//tables_[i].add(features);
				tables_[i].add(points_);
			}
		}

		

		/*
		**@brief	�����������dataset�޸���������Ϣ����nn_index����
		**
		**@param	dataset	-��������ݼ�
		*/
		void setDataset(Matrix<ElementType>& dataset)
		{
			size_ = dataset.rows;
			veclen_ = dataset.cols;
			//last_id_ = 0;

			//ids_.clear();
			//removed_points_.clear();
			//removed_ = false;
			//removed_count_ = 0;
			points_.clear();
			points_.resize(size_);
			size_t i = 0;
			while( i < size_ - 4 )											//һ����5��
			{
				points_[i] = dataset[i];									//dataset[]���غ󷵻����������׵�ַ
				points_[i + 1] = dataset[i + 1];
				points_[i + 2] = dataset[i + 2];
				points_[i + 3] = dataset[i + 3];
				points_[i + 4] = dataset[i + 4];
				i += 5;
			}
			while (i < size_)
			{
				points_[i] = dataset[i];
				i++;
			}
			/*nothing to do here*/
		}

		/*
		**@brief	��չ���ݼ�
		*/
		void extendDataset(const Matrix<ElementType>& new_points)
		{
			//size_t new_size = size_ + new_points.rows;
			//if (removed_) 
			//{
			//	removed_points_.resize(new_size);
			//	ids_.resize(new_size);
			//}
			//points_.resize(new_size);
			//for (size_t i = size_; i < new_size; ++i)
			//{
			//	points_[i] = new_points[i - size_];
				//if (removed_) 
				//{
				//	ids_[i] = last_id_++;
				//	removed_points_.reset(i);
				//}
			//}
			//size_ = new_size;
		}

	public:

		/*
		**@brief ������ݼ�
		*/
		void addPoints(const Matrix<ElementType>& points, float rebuild_threshold = 2)
		{
			/**
			assert(points.cols == veclen_);
			size_t old_size = size_;

			extendDataset(points);											//�����ض���points_�Ŀռ��С

			if (rebuild_threshold > 1 && size_at_build_*rebuild_threshold < size_)
			{
				buildIndex();
			}
			else
			{
				for (unsigned int i = 0; i < table_number_; ++i)
				{
					lsh::LshTable<ElementType>& table = tables_[i];
					for (size_t i = old_size; i < size_; ++i)
					{
						table.add(i, points_[i]);
					}
				}
			}
			**/
		}


		//flann_algorithm_t getType() const
		//{
		//	return FLANN_INDEX_LSH;
		//}


		/**
		template<typename Archive>
		void serialize(Archive& ar)
		{
		ar.setObject(this);

		ar & *static_cast<NNIndex<Distance>*>(this);

		ar & table_number_;
		ar & key_size_;
		ar & multi_probe_level_;

		ar & xor_masks_;
		ar & tables_;

		if (Archive::is_loading::value) {
		index_params_["algorithm"] = getType();
		index_params_["table_number"] = table_number_;
		index_params_["key_size"] = key_size_;
		index_params_["multi_probe_level"] = multi_probe_level_;
		}
		}
		**/

		void saveIndex(FILE* stream)
		{
			//	serialization::SaveArchive sa(stream);
			//	sa & *this;
		}

		void loadIndex(FILE* stream)
		{
			//	serialization::LoadArchive la(stream);
			//	la & *this;
		}

		/**
		 * Computes the index memory usage
		 * Returns: memory used by the index
		 */
		int usedMemory() const
		{
			//return size_ * sizeof(int);
			int size_kb = size_*table_number_*sizeof(ElementType);
			size_kb = size_kb >> 10;
			return(size_kb);
		}

		/**
		 * \brief Perform k-nearest neighbor search
		 * \param[in] queries The query points for which to find the nearest neighbors
		 * \param[out] indices The indices of the nearest neighbors found
		 * \param[out] dists Distances to the nearest neighbors found
		 * \param[in] knn Number of nearest neighbors to return
		 * \param[in] params Search parameters
		 */
		int knnSearch(
			const Matrix<ElementType>& queries,
			Matrix<size_t>& indices,
			Matrix<DistanceType>& dists,
			size_t knn)//const
			//const SearchParams& params) const
		{
			assert(queries.cols == veclen_);
			assert(indices.rows >= queries.rows);
			assert(dists.rows >= queries.rows);
			assert(indices.cols >= knn);
			assert(dists.cols >= knn);
			size_t dis_cnt0 = 0, dis_cnt1 = 0, dis_cnt2 = 0, dis_cnt3 = 0, dis_cnt4 = 0;
			//m_knn = knn;
			//size_t n = 0;
			//int count = 0;
			//if (params.use_heap == FLANN_True)
			//{
//#pragma omp parallel num_threads(params.cores)
			//	{
					//KNNUniqueResultSet<DistanceType> resultSet(knn);
					int i = 0;
//#pragma omp for schedule(static) reduction(+:count)
					while (i < (int)(queries.rows - 4))
					{
						dis_cnt0 = getNeighbors(indices[i], dists[i], queries[i], knn);

						dis_cnt1 = getNeighbors(indices[i + 1], dists[i + 1], queries[i + 1], knn);

						dis_cnt2 = getNeighbors(indices[i + 2], dists[i + 2], queries[i + 2], knn);
						
						dis_cnt3 = getNeighbors(indices[i + 3], dists[i + 3], queries[i + 3], knn);

						dis_cnt4 = getNeighbors(indices[i + 4], dists[i + 4], queries[i + 4], knn);

						i += 5;
						m_distance_cnt += (dis_cnt0 + dis_cnt1 + dis_cnt2 + dis_cnt3 + dis_cnt4);
					}
					while (i < (int)queries.rows)
					{
						dis_cnt0 = getNeighbors(indices[i], dists[i], queries[i], knn);
						m_distance_cnt += dis_cnt0;
						i++;
					}
			//	}
			//}
			//else
			//{
//#pragma omp parallel num_threads(params.cores)
			//	{
			//		KNNResultSet<DistanceType> resultSet(knn);
//#pragma omp for schedule(static) reduction(+:count)
			//		for (int i = 0; i < (int)queries.rows; i++)
			//		{
			//			resultSet.clear();
			//			//findNeighbors(resultSet, queries[i], params);
			//			getNeighbors(queries[i], resultSet);
			//			n = (std::min)(resultSet.size(), knn);
			//			resultSet.copy(indices[i], dists[i], n, params.sorted);
			//			//indices_to_ids(indices[i], indices[i], n);
			//			count += n;
			//		}
			//	}
			//}

			return 0;
		}

	private:

		/*
		**	Performs the approximate nearest-neighbor search.
		**	This is a slower version than the above as it uses the ResultSet
		**	@param vec -the feature to analyze
		*/
		size_t getNeighbors(
			size_t* indices,
			DistanceType*  dists,
			ElementType* vec, 
			size_t knn) const
		{
			typename std::vector<lsh::LshTable<ElementType> >::const_iterator table = tables_.begin();
			typename std::vector<lsh::LshTable<ElementType> >::const_iterator table_end = tables_.end();	//����������ͷ��β
			//unsigned int worst_score = std::numeric_limits<unsigned int>::max();
			size_t distance_cnt = 0;
			lsh::BucketKey bucket_key;
			std::vector<size_t>indices_one;
			std::vector<DistanceType>dists_one;
			//size_t* indices_one = new size_t[knn];
			//DistanceType* dists_one = new DistanceType[knn];
			size_t i = 0;
			unsigned char first_flag = 0;
			for (; table != table_end; table++)																//ÿ����Ҫ���м��㲢�洢����
			{
				i = 0;
				bucket_key = std::make_pair(0, 0);
				bucket_key = table->getKey(vec);

				const lsh::Bucket* bucket_p = table->getBucketFromKey(bucket_key);
				if (bucket_p == 0)
					continue;
				// Go over each descriptor index
				std::vector<lsh::FeatureIndex>::const_iterator training_index = bucket_p->begin();				//
				std::vector<lsh::FeatureIndex>::const_iterator last_training_index = bucket_p->end();
				DistanceType euclidean_distance = 0;
				// Process the rest of the candidates
				for (; training_index < last_training_index; training_index++,i++)								//�����ѯ�㵽���е����벢�洢
				{
					euclidean_distance = 0;
					if (*training_index >= size_)
						continue;
					// Compute the euclidean distance
					euclidean_distance = m_distance(vec, points_[*training_index], veclen_);
					distance_cnt++;
					dists_one.push_back(euclidean_distance);
					indices_one.push_back(*training_index);
					//if (i < knn)
					//{
					//	*(dists_one + i) = euclidean_distance;
					//	*(indices_one + i) = *training_index;														//������ѹ�������ݼ�,���б������ѹ��һ��
					//}
					//else
					//{
					//	if (i == knn)
					//		bubble_swap(indices_one, dists_one, knn);
					//	if ( euclidean_distance> *(indices_one+knn-1) )
					//		continue;
					//	else
					//	{
					//		*(dists_one + knn - 1) = euclidean_distance;
					//		*(indices_one + knn - 1) = *training_index;
					//		bubble_swap(indices_one, dists_one, knn);
					//	}
					//}
				}
				
			}
			for (i = dists_one.size(); i < knn; i++)
			{
				dists_one.push_back(E2LSH_BigPrimeNum);
				indices_one.push_back(E2LSH_BigPrimeNum);
			}
			bubble_swap(indices_one, dists_one, knn);
			for (i = 0; i < knn; i++)
			{
				*(indices + i) = indices_one[i];
				*(dists + i) = dists_one[i];
			}
			dists_one.clear();
			indices_one.clear();
			return(distance_cnt);
		}


		/*
		**	@brief	��dist_one�е����ݽ���ð�����򣬽���С��ð��ǰ��,ͬʱ����indices_one�е�����
		**	
		**	@param	
		*/
		void bubble_swap(std::vector<size_t>&indices_one, std::vector<DistanceType>&dists_one, size_t knn)const
		{
			size_t i = 0, j = 0, i_temp = 0;
			float d_temp = 0;
			int total_num = 0;
			total_num = indices_one.size();
			for (i = total_num - 1; i > total_num - 1 - knn; i--)
			{
				for (j = i; j>0; j--)
				{
					if (dists_one[j] < dists_one[j-1])
					{
						/*���뽻��*/
						d_temp = dists_one[j];
						dists_one[j] = dists_one[j-1];
						dists_one[j-1] = d_temp;
						/*��������*/
						i_temp = indices_one[j];
						indices_one[j] = indices_one[j - 1];
						indices_one[j - 1] = i_temp;
					}
				}
			}
		}

		void swap(LshIndex& other)
		{
			//BaseClass::swap(other);
			std::swap(tables_, other.tables_);
			std::swap(points_, other.points_);
			std::swap(size_, other.size_);
			std::swap(veclen_, other.veclen_);
			std::swap(table_number_, other.table_number_);
			std::swap(key_size_, other.key_size_);
			std::swap(m_gap_w, other.m_gap_w);
			std::swap(m_table_size, other.m_table_size);
			std::swap(m_distance, other.m_distance);
			std::swap(index_params_, other.index_params_);
			std::swap(size_at_build_, other.size_at_build_);
			//std::swap(xor_masks_, other.xor_masks_);
		}
		//USING_BASECLASS_SYMBOLS
	};
}

#endif //FLANN_LSH_INDEX_H_
