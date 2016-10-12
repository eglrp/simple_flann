/*
**@ �йع�ϣ�����͹�ϣ�������һЩ���� 
*/

/***********************************************************************
 * Author: Vincent Rabaud
 * redefine ��zhaoshuai
 *************************************************************************/

#ifndef FLANN_LSH_TABLE_H_
#define FLANN_LSH_TABLE_H_

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <limits.h>
// TODO as soon as we use C++0x, use the code in USE_UNORDERED_MAP
//#if USE_UNORDERED_MAP
//#include <unordered_map>
//#else
#include <map>
//#endif
#include <math.h>
#include <stddef.h>
#include <iomanip>

#include "dynamic_bitset.h"
#include "matrix.h"
#include "../algorithms/random.h"
#include "../algorithms/dist.h"


#define E2LSH_BigPrimeNum (4294967291U)
#define MaxHashRand (536870912U)

namespace flann
{

	namespace lsh
	{

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/*
		** What is stored in an LSH bucket
		*/
		typedef unsigned int FeatureIndex;
		/*
		** The id from which we can get a bucket back in an LSH table
		*/
		//typedef unsigned int BucketKey;							//��Թ�ϣ��Ĺؼ����ǵ�����ʵ��
		typedef std::pair<unsigned int, unsigned int> BucketKey;	//p-stable LSH�Ĺؼ������ݽṹ

		/*
		** A bucket in an LSH table
		*/
		typedef std::vector<FeatureIndex> Bucket;

		


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/** POD for stats about an LSH table
		 */
		//struct LshStats
		//{
		//	std::vector<unsigned int> bucket_sizes_;
		//	size_t n_buckets_;
		//	size_t bucket_size_mean_;
		//	size_t bucket_size_median_;
		//	size_t bucket_size_min_;
		//	size_t bucket_size_max_;
		//	size_t bucket_size_std_dev;
		//	/** Each contained vector contains three value: beginning/end for interval, number of elements in the bin
		//	 */
		//	std::vector<std::vector<unsigned int> > size_histogram_;
		//};

		/*
		 * Overload the << operator for LshStats
		 * @param out the streams
		 * @param stats the stats to display
		 * @return the streams
		 */
		/*
		inline std::ostream& operator <<(std::ostream& out, const LshStats& stats)
		{
			size_t w = 20;
			out << "Lsh Table Stats:\n" << std::setw(w) << std::setiosflags(std::ios::right) << "N buckets : "
				<< stats.n_buckets_ << "\n" << std::setw(w) << std::setiosflags(std::ios::right) << "mean size : "
				<< std::setiosflags(std::ios::left) << stats.bucket_size_mean_ << "\n" << std::setw(w)
				<< std::setiosflags(std::ios::right) << "median size : " << stats.bucket_size_median_ << "\n" << std::setw(w)
				<< std::setiosflags(std::ios::right) << "min size : " << std::setiosflags(std::ios::left)
				<< stats.bucket_size_min_ << "\n" << std::setw(w) << std::setiosflags(std::ios::right) << "max size : "
				<< std::setiosflags(std::ios::left) << stats.bucket_size_max_;

			// Display the histogram
			out << std::endl << std::setw(w) << std::setiosflags(std::ios::right) << "histogram : "
				<< std::setiosflags(std::ios::left);
			for (std::vector<std::vector<unsigned int> >::const_iterator iterator = stats.size_histogram_.begin(), end =
				stats.size_histogram_.end(); iterator != end; ++iterator) out << (*iterator)[0] << "-" << (*iterator)[1] << ": " << (*iterator)[2] << ",  ";

				return out;
		}
		*/


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/** Lsh hash table. As its key is a sub-feature, and as usually
		 * the size of it is pretty small, we keep it as a continuous memory array.
		 * The value is an index in the corpus of features (we keep it as an unsigned
		 * int for pure memory reasons, it could be a size_t)
		 */
		template<typename ElementType>
		class LshTable
		{
		public:
			//typedef std::map<unsigned int, Bucket> BucketsSerial;					//unsigned int��ӦH1
			//typedef std::map<unsigned int, BucketsSerial> BucketsSpace;			//unsigned int��ӦH2
			typedef std::map<BucketKey, Bucket> BucketsSpace;
			
			
			
		private:
			/*���£���ʱ�Զ���˽�г�Ա*
			/*��ϣͰ�ռ�*/
			BucketsSpace buckets_space_;
			//RandomGen<float> m_random;
			RandomGen m_random;							//�������
			unsigned int m_feature_size;				//����������С
			unsigned int m_key_size;					//�ؼ�ֵ�Ĵ�С(λ��)
			float m_gap_w;								//p�ȶ��ֲ����������
			std::vector<std::vector<float>>m_vec_a;		//��ϣ�����е�����a,vector����
			std::vector<float>m_real_b;					//��ϣ�����е����ʵ��b
			unsigned int m_table_size;					//��ϣ��ĳ��ȣ�һ������Ϊ���ݵ�����
			std::vector<unsigned int>m_r1, m_r2;		//����ϣ��������ʱ���õ����������
			/*����hash bucket*/
			


		public:
			/*
			**A container of all the feature indices. Optimized for space
			**
			** BucketKey�ǹ�ϣͰ�Ĺؼ��֣�Bucket��һ��FeatureIndex���͵�vector����
			*/
//#if USE_UNORDERED_MAP
//			typedef std::unordered_map<BucketKey, Bucket> BucketsSpace;
//#else
			//typedef std::map<BucketKey, Bucket> BucketsSpace;
//#endif

			/*
			**	A container of all the feature indices. Optimized for speed
			**	�����������������������ά��vector����
			*/
			//typedef std::vector<Bucket> BucketsSpeed;

			/*
			**	Default constructor
			*/
			LshTable(){}


			/*
			**@brief 	constructor
			**
			**@param	feature_size	-����������С(ά��)
			**			key_size		-����ϣ��ά��Ĺؼ���λ��
			**			gap_w			-p�ȶ��ֲ������
			**			table_size		-��ϣ���С
			*/
			LshTable(unsigned int feature_size, unsigned int key_size, float gap_w,unsigned int table_size)
			{
				m_random=RandomGen();
				m_feature_size = feature_size;
				m_key_size = key_size;
				m_gap_w = gap_w;
				m_table_size = table_size;
				buckets_space_.clear();

				m_vec_a.clear(); m_vec_a.resize(m_key_size);
				m_real_b.clear(); m_real_b.resize(m_key_size);
				m_r1.clear(); m_r1.resize(m_key_size);
				m_r2.clear(); m_r2.resize(m_key_size);

				size_t i = 0, j = 0;								//��׼��̬�ֲ�
				for (i = 0; i < m_key_size; i++)					//���ɹ�ϣ�����е�����a��ʵ��b
				{
					m_real_b[i] = m_random.gen_uniform_random(0, m_gap_w);
					m_r1[i] = m_random.gen_random_uns32(0, MaxHashRand);
					m_r2[i] = m_random.gen_random_uns32(0, MaxHashRand);
					m_vec_a[i].resize(m_feature_size);
					for (j = 0; j < feature_size; j++)			
					{
						m_vec_a[i][j] = m_random.gen_gaussian_random();
					}
				}
			}

			~LshTable()
			{
				m_vec_a.clear();
				std::vector<std::vector<float>>(m_vec_a).swap(m_vec_a);
				m_real_b.clear();
				std::vector<float>(m_real_b).swap(m_real_b);
				m_r1.clear();
				m_r2.clear();
				std::vector<unsigned int>(m_r1).swap(m_r1);
				std::vector<unsigned int>(m_r2).swap(m_r2);
			}

			void operator = (const LshTable& other)
			{
				buckets_space_=other.buckets_space_;
				m_random=other.m_random;					
				m_feature_size = other.m_feature_size;		
				m_key_size = other.m_key_size;				
				m_gap_w = other.m_gap_w;					
				m_vec_a = other.m_vec_a;					
				m_real_b = other.m_real_b;					
				m_table_size=other.m_table_size;			
				m_r1 = other.m_r1;
				m_r2 = other.m_r2;
			}


			/*
			**	Add a set of features to the table
			**	@param dataset	-the values to store
			**	
			*/
			//void add(const std::vector< std::pair<size_t, ElementType*> >& features)
			void add(std::vector<ElementType*>& features)
			{
				// Add the features to the table
				BucketKey key;
				size_t i = 0;				
				while (i < m_table_size - 4 )												//һ��ѹ5��
				{
					key = getKey(features[i]);
					buckets_space_[key].push_back(i);
					//buckets_space_[key.first][key.second].push_back(features[i].first);
					key = getKey(features[i + 1]);
					buckets_space_[key].push_back(i + 1);
					//buckets_space_[key.first][key.second].push_back(features[i + 1].first);
					key = getKey(features[i + 2]);
					buckets_space_[key].push_back(i + 2);
					//buckets_space_[key.first][key.second].push_back(features[i + 2].first);
					key = getKey(features[i + 3]);
					buckets_space_[key].push_back(i + 3);
					//buckets_space_[key.first][key.second].push_back(features[i + 3].first);
					key = getKey(features[i + 4]);
					buckets_space_[key].push_back(i + 4);
					//buckets_space_[key.first][key.second].push_back(features[i + 4].first);
					i += 5;
				}
				while (i < m_table_size)
				{
					key = getKey(features[i]);
					buckets_space_[key].push_back(i);
					i++;
				}
			}

			/* 
			**	Get a bucket given the key
			**	@param	-key
			**	@return
			**	��BucketKey����ָ��Bucket��ָ��
			*/
			inline const Bucket* getBucketFromKey(BucketKey key) const
			{
				// That means we get the buckets from an array
				//����H1��
				//BucketsSpace::const_iterator bucket_ser_it, bucket_ser_end = buckets_space_.end();
				//bucket_ser_it = buckets_space_.find(key.first);
				//if (bucket_ser_it == bucket_ser_end)
				//	return 0;
				//����H2��
				//BucketsSerial::const_iterator bucket_it, bucket_end = bucket_ser_it->second.end();
				//bucket_it = bucket_ser_it->second.find(key.second);
				//if (bucket_it == bucket_end) 
				//	return 0;
				//else 
				//{
				//	return (&bucket_it->second);
				//}

				BucketsSpace::const_iterator bucket_ser_it, bucket_ser_end = buckets_space_.end();
				bucket_ser_it = buckets_space_.find(key);
				if (bucket_ser_it == bucket_ser_end)
					return 0;
				else
				{
					return(&bucket_ser_it->second);
				}

			}

			/* 
			**	Compute the sub-signature of a feature
			**	����һ�����������ľ�����ϣͶӰ�������
			**
			*/
			inline BucketKey getKey(ElementType* feature) const
			{
				unsigned int H1 = 0, H2 = 0;
				unsigned int i = 0, j = 0;
				float H_temp = 0;
				std::vector<unsigned>H;
				//H.clear();
				//const ElementType* feature = dataset;
				H.resize(m_key_size);
				unsigned long long H1_temp = 0, H2_temp = 0;
				for (i = 0; i < m_key_size;i++)						//���������ϣ������ά�����������
				{
					H_temp = 0;										//clear
					for (j = 0; j < m_feature_size - 15;)			//��16�����Ч��,���õ����ݼ�����ά������16�ı���
					{
						H_temp += (m_vec_a[i][j] * feature[j] + m_vec_a[i][j + 1] * feature[j + 1]
							+ m_vec_a[i][j + 2] * feature[j + 2] + m_vec_a[i][j + 3] * feature[j + 3]
							+ m_vec_a[i][j + 4] * feature[j + 4] + m_vec_a[i][j + 5] * feature[j + 5]
							+ m_vec_a[i][j + 6] * feature[j + 6] + m_vec_a[i][j + 7] * feature[j + 7]
							+ m_vec_a[i][j + 8] * feature[j + 8] + m_vec_a[i][j + 9] * feature[j + 9]
							+ m_vec_a[i][j + 10] * feature[j + 10] + m_vec_a[i][j + 11] * feature[j + 11]
							+ m_vec_a[i][j + 12] * feature[j + 12] + m_vec_a[i][j + 13] * feature[j + 13]
							+ m_vec_a[i][j + 14] * feature[j + 14] + m_vec_a[i][j + 15] * feature[j + 15]);
						j += 16;
					}
					while (j < m_feature_size)
					{
						H_temp += m_vec_a[i][j] * feature[j];
						j++;
					}
					H_temp += m_real_b[i];
					H_temp = H_temp / m_gap_w;
					H[i] = (unsigned)H_temp;
					H1_temp += m_r1[i] * H[i];						//������ϣ���������BucketKey
					//H1_temp = H1_temp%E2LSH_BigPrimeNum;
					//if (H1_temp < 0)
					//	H1_temp += E2LSH_BigPrimeNum;
					H2_temp += m_r2[i] * H[i];
					//H2_temp = H2_temp%E2LSH_BigPrimeNum;
					//if (H2_temp < 0)
					//	H2_temp += E2LSH_BigPrimeNum;
				}
				H1 = (unsigned int)((H1_temp%E2LSH_BigPrimeNum) % m_table_size);
				H2 = (unsigned int)(H2_temp%E2LSH_BigPrimeNum);
				//H1 = (unsigned int)(H1_temp% m_table_size);
				//H2 = (unsigned int)(H2_temp);
				H.clear();
				std::vector<unsigned>(H).swap(H);					//���ڴ�
				return(std::make_pair(H1, H2));						//����BucketKey
			}

		};

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// End the two namespaces
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif /* FLANN_LSH_TABLE_H_ */
