#pragma once
#include <sstream>
#include <vector>

namespace oss
{
    namespace result
    {
        struct Bucket
        {
            std::string name;
            std::string create_time;
        };

        typedef  std::vector<oss::result::Bucket> ListBucketResult;

        typedef std::string PutBucketAclResult;

        struct Object
        {
            std::string key;
            std::string size;
            std::string time;
            std::string etag;
        };

        struct ListObjectResult
        {
            std::vector<Object> objectList;
            std::string nextMarker;
            std::vector<std::string> folderList;
        };

		//�ֿ��ϴ���һ���ֿ�
		struct MultiUpTaskPart
		{
			std::string upid;
			std::string path;
			std::string ETag;
		};

		//һ���ֿ��ļ��ϴ��ļ������зֿ�
		typedef  std::vector<oss::result::MultiUpTaskPart> MultiUpTaskPartList;

		// �ֿ��ϴ�����
		struct MultiUpTask
		{
			std::string key;
			std::string upid;
			std::string time;
		};

		//�ֿ��ϴ��б�
		struct MultiUpTaskList
		{
			std::vector<MultiUpTask> taskList;
			std::string keyMarker;
			std::string upidMarker;
		};
		
    }
}