#pragma once
#include "client.h"
#include <iostream>
#include "echttp.h"
#include <fstream>
#include <queue>

class COssSdk :
        public client
{
private:
        vector<DOWNTASK*> downloadList; 

        string mAccessid;
        string mAccessKey;
        string *m_host;

public:
        typedef boost::function<void(int,std::string,void*)> ApiCallBack;
        COssSdk(std::string accessid,std::string accesskey,std::string *host):client(accessid,accesskey,host)
        {	
                this->mAccessid=accessid;
                this->mAccessKey=accesskey;
                this->m_host=host;
        }


        //�ϴ�Ŀ¼����
        void upDir(string bucketName,string path,ApiCallBack func,int mulitNum=5,string bucketPath="")
        {
                vector<string> fileList=echttp::DirFiles(path);
                vector<string>::iterator it=fileList.begin();
                path=echttp::replace_all(path,"\\","/");

                if(path[path.length()-1]!='/') path+="/";

                //
                if(mulitNum>fileList.size()) mulitNum=fileList.size();

                queue<UPTASK*> *uplist=new queue<UPTASK*>[mulitNum];
                client** apis=new client*[mulitNum];
                for(int i=0;i<mulitNum;i++)
                {
                        apis[i]=new client(mAccessid,mAccessKey,m_host);
                }

                for(int i=0;it!=fileList.end();it++)
                {

                        int id=i%mulitNum;	

                        string filename=echttp::replace_all(*it,"\\","/");
                        filename=echttp::replace_all(filename,path,"");
                        string bucketfile=bucketPath+filename;

                        UPTASK *upTask=new UPTASK;
                        upTask->bucketName=bucketName;
                        upTask->bucketFileName=bucketfile;
                        upTask->path=*it;
                        upTask->isUp=false;

                        uplist[id].push(upTask);
                        i++;
                }


                for(int i=0;i<mulitNum;i++)
                {

                        UPTASK *upTask=uplist[i].front();
                        std::cout<<"�ϴ���"<<upTask->path<<"\n";	
                        apis[i]->PutObject(upTask->bucketName,upTask->path,boost::bind(&COssSdk::recvUpDir,this,_1,_2,_3,i,uplist[i],apis[i],func),upTask->bucketFileName);
                }

        }

        void recvUpDir(int code,std::string msg,void* param,int taskId,queue<UPTASK*>  tasklist,client* api,ApiCallBack func)
        {

                UPTASK* lastTask=tasklist.front();
                func(code,msg,lastTask);
                if(code==200||lastTask->upNum>=3)
                {
                        delete lastTask;
                        tasklist.pop();
                }
                else
                {
                        std::cout<<"�ϴ���"<<lastTask->path<<"\n";	
                        api->PutObject(lastTask->bucketName,lastTask->path,boost::bind(&COssSdk::recvUpDir,this,_1,_2,_3,taskId,tasklist,api,func),lastTask->bucketFileName);
                        return;
                }

                if(!tasklist.empty())
                {
                        UPTASK* newTask=tasklist.front();

                        std::cout<<"�ϴ���"<<newTask->path<<"\n";	
                        api->PutObject(newTask->bucketName,newTask->path,boost::bind(&COssSdk::recvUpDir,this,_1,_2,_3,taskId,tasklist,api,func),newTask->bucketFileName);	
                }
                else
                {
                        std::cout<<echttp::convert<string>(taskId)<<"�ϴ����\n";
                }

        }


        //����Ŀ¼����
        void downDir(string bucketName,string path,string downPath,ApiCallBack func ,int mulitNum=5)
        {
                this->ListObject(bucketName,boost::bind(&COssSdk::recvDownDir,this,_1,_2,_3,mulitNum,downPath,path,bucketName,func),path);

        }

        void recvDownDir(int code,std::string msg,void* param,int mulitNum,string downPath,string bucketPath,string bucketName,ApiCallBack func)
        {
                if(code==200&&param!=NULL)
                {
                        objectList *filelist=(objectList*)param;
                        vector<Object*> &objects=filelist->lists;


                        std::vector<Object*>::iterator it;
                        for(it=objects.begin();it!=objects.end();it++)
                        {
                                DOWNTASK * task=new DOWNTASK;
                                Object *ossObject=*it;
                                task->bucketName=bucketName;
                                task->bucketFileName=ossObject->path;
                                string filename=echttp::replace_all(task->bucketFileName,bucketPath,"");
                                task->path=downPath+filename;
                                task->isDown=false;

                                this->downloadList.push_back(task);
                        }

                        //�Զ��н��д������̷߳�����������
                        if(mulitNum>this->downloadList.size()) mulitNum=this->downloadList.size();
                        queue<DOWNTASK*> *downList=new queue<DOWNTASK*>[mulitNum];

                        client** apis=new client*[mulitNum];
                        for(int i=0;i<mulitNum;i++)
                        {
                                apis[i]=new client(mAccessid,mAccessKey,m_host);
                        }

                        for(int i=0;i<mulitNum;i++)
                        {
                                for(int j=0;j<downloadList.size();j++)
                                {
                                        if(j%mulitNum==i)
                                        {
                                                downList[i].push(downloadList[j]);
                                        }   
                                }
                        }

                        for(int i=0;i<mulitNum;i++)
                        {
                                DOWNTASK *downTask=downList[i].front();
                                std::cout<<"���أ�"<<downTask->path<<"\n";	
                                apis[i]->downObject(downTask->bucketName,downTask->bucketFileName,downTask->path,boost::bind(&COssSdk::recvDownFile,this,_1,_2,_3,i,downList[i],apis[i],func));
                        }

                }

        }

        void recvDownFile(int code,std::string msg,void* param,int taskId,queue<DOWNTASK*>  tasklist,client* api,ApiCallBack func)
        {
                DOWNTASK* lastTask=tasklist.front();
                func(code,msg,lastTask);
                if(code==200||lastTask->upNum>=3)
                {
                        delete lastTask;
                        tasklist.pop();
                }
                else
                {
                        std::cout<<"���أ�"<<lastTask->path<<"\n";	
                        api->downObject(lastTask->bucketName,lastTask->bucketFileName,lastTask->path,boost::bind(&COssSdk::recvDownFile,this,_1,_2,_3,taskId,tasklist,api,func));
                        return;
                }

                if(!tasklist.empty())
                {
                        DOWNTASK* newTask=tasklist.front();

                        std::cout<<"���أ�"<<newTask->path<<"\n";	
                        api->downObject(newTask->bucketName,newTask->bucketFileName,newTask->path,boost::bind(&COssSdk::recvDownFile,this,_1,_2,_3,taskId,tasklist,api,func));	
                }
                else
                {
                        std::cout<<echttp::convert<string>(taskId)<<"�������\n";
                        delete api;
                }
        }

        //�ֿ��ϴ�����
        void mulitUpFile(string bucketName,string objectName,string path,ApiCallBack func,int mulitNum=5)
        {   
                this->initMultiUp(bucketName,objectName,boost::bind(&COssSdk::recvInitMulitUp,this,_1,_2,_3,bucketName,objectName,path,mulitNum,func));   

        }

        void recvInitMulitUp(int code,std::string msg,void* param,string bucketName,string objectName,string path,int mulitNum,ApiCallBack func)
        {

                size_t filesize=echttp::fileLen(path);
                vector<UPTASK*> *mulitUpList=new vector<UPTASK*>;

                long step=10*1024*1024;
                int i=0;
                for(unsigned long pos=0;pos<filesize;pos+=step)
                {
                        long partsize=step;
                        if((filesize-pos)<step) partsize=filesize-pos;
                        UPTASK *task=new UPTASK;
                        task->bucketName=bucketName;
                        task->bucketFileName=objectName;
                        task->path=path;
                        task->isUp=false;
                        task->upNum=0;
                        task->upid=msg;
                        task->number=i;

                        task->pos=pos;
                        task->size=partsize;
                        i++;

                        mulitUpList->push_back(task);
                }

                if(i<mulitNum) mulitNum=i;

                client** apis=new client*[mulitNum];
                for(int i=0;i<mulitNum;i++)
                {
                        apis[i]=new client(mAccessid,mAccessKey,m_host);
                }

                i=0;
                vector<UPTASK*>::iterator it;
                for(it=mulitUpList->begin();it!=mulitUpList->end();it++)
                {
                        int num=i%mulitNum;
                        UPTASK* &task=*it;
                        task->worker=apis[num];
                        i++;
                }

                for(int j=0;j<mulitNum;j++)
                {
                        UPTASK *task=mulitUpList->at(j);
                       
                        task->worker->PutObject(task->bucketName,task->bucketFileName,task->path,task->upid,task->number,task->pos,task->size,boost::bind(&COssSdk::recvMulitUpFile,this,_1,_2,_3,j,task->number,mulitNum,mulitUpList,func));
                }
                func(1000,"�ֿ��ϴ���"+path+" ͬʱio����"+echttp::convert<string>(mulitNum)+" �ļ��ֿ�����"+echttp::convert<string>(i),NULL);


        }

        void recvMulitUpFile(int code,std::string msg,void* param,int taskId,int partid,int mulitNum,vector<UPTASK*>  *tasklist,ApiCallBack func)
        {

                UPTASK* task=tasklist->at(partid);
                if(code==200)
                {
                        task->isUp=true;
                        task->ETag=msg;
                        func(1000,task->path+"�ֿ�--"+echttp::convert<string>(partid)+" �ϴ��ɹ�",NULL);
                }else{
                        task->upNum++;
                         func(1000,task->path+"�ֿ�--"+echttp::convert<string>(partid)+" �ϴ�ʧ�ܣ������ϴ�",NULL);
                }

                int taskNum=tasklist->size();
                int ck=0;

                for(int i=0;i<taskNum;i++)
                {
                        if(tasklist->at(i)->upNum>3|| tasklist->at(i)->isUp==true) continue;
                        if(taskId==i%mulitNum)
                        {
                                if(tasklist->at(i)->isUp==true) continue;  
                                if(tasklist->at(i)->upNum>3) continue;
                                ck=1;
                                task=tasklist->at(i);
                                task->worker->PutObject(task->bucketName,task->bucketFileName,task->path,task->upid,task->number,task->pos,task->size,boost::bind(&COssSdk::recvMulitUpFile,this,_1,_2,_3,taskId,task->number,mulitNum,tasklist,func));
                                break;
                        }

                }
                if(ck==0) delete task->worker;

                for(int i=0;i<taskNum;i++)
                {
                        if(tasklist->at(i)->upNum>3|| tasklist->at(i)->isUp==true) continue;
                        ck=1;
                }

                if(ck==0)//���зֿ��ϴ����
                {
                        this->CompleteUpload(task->bucketName,task->bucketFileName,task->upid,tasklist,func);
                        vector<UPTASK*>::iterator it;
                        for(it=tasklist->begin();it!=tasklist->end();it++)
                        {
                                delete *it;
                        }
                        tasklist->clear();
						delete tasklist;
                }

        }


        void cancelMulitUp(string bucketName,ApiCallBack func)
        {
                this->ListMulitUp(bucketName,boost::bind(&COssSdk::recvCancelMulitUp,this,_1,_2,_3,func));

        }

        void recvCancelMulitUp(int code,std::string msg ,void*param,ApiCallBack func)
        {
			uploadsObjectList* objects=(uploadsObjectList*)param;

			vector<uploadsObject*> &lists=objects->lists;

			for(size_t i=0;i<lists.size();i++)
			{
				string key=echttp::Utf8Encode(lists.at(i)->key);
				this->abortMulitUp(objects->bucketName,key,lists.at(i)->uploadId,func);
			}
			delete objects;

        }

        void upFileList(vector<UPTASK*> *tasklist,ApiCallBack func,int mulitNum=5)
        {
                int i=tasklist->size();
                if(i<mulitNum) mulitNum=i;

                client** apis=new client*[mulitNum];
                for(int i=0;i<mulitNum;i++)
                {
                        apis[i]=new client(mAccessid,mAccessKey,m_host);
                }

                i=0;
                vector<UPTASK*>::iterator it;
                for(it=tasklist->begin();it!=tasklist->end();it++)
                {
                        int num=i%mulitNum;
                        UPTASK* &task=*it;
                        task->worker=apis[num];
                        i++;
                }

                for(int j=0;j<mulitNum;j++)
                {
                        UPTASK *task=tasklist->at(j);
                        if(echttp::fileLen(task->path)>10*1024*1024)
                        {
                                this->mulitUpFile(task->bucketName,task->bucketFileName,task->path,boost::bind(&COssSdk::recvUpFileList,this,_1,_2,_3,task->number,j,mulitNum,tasklist,func));
                        }
                        else
                        {
                                func(1000,"�˿�"+echttp::convert<string>(j)+" �����ϴ���"+task->bucketFileName,NULL);
                                task->worker->PutObject(task->bucketName,task->path,boost::bind(&COssSdk::recvUpFileList,this,_1,_2,_3,task->number,j,mulitNum,tasklist,func),task->bucketFileName);	
                        }
                }       
        }

        void recvUpFileList(int code,std::string msg,void* param,int taskId,int threadId,int mulitNum,vector<UPTASK*>  *tasklist,ApiCallBack func)
        {
               if(code ==1000)
               {
                        func(1000,msg,NULL);
                        return;
               }
                UPTASK* task=tasklist->at(taskId);
                if(code>=200 &&code<300)
                {
                        task->isUp=true;
                        task->ETag=msg;
                        func(code,task->path,NULL);
                }else{
                        task->upNum++;
                        if(task->upNum>5){
                                func(1001,"�ļ���"+task->path+"��������ϴ�����Ȼ�ϴ�ʧ�ܣ�����һ������!",NULL);
                        }
                }

                int taskNum=tasklist->size();
                int ck=0;

                for(int i=0;i<taskNum;i++)
                {
                        if(tasklist->at(i)->upNum>5|| tasklist->at(i)->isUp==true) continue;
                        if(threadId==i%mulitNum)
                        {
                                if(tasklist->at(i)->isUp==true) continue;  
                                if(tasklist->at(i)->upNum>5) continue;
                                ck=1;
                                UPTASK*  newtask=tasklist->at(i);
                                task->worker->PutObject(newtask->bucketName,newtask->path,boost::bind(&COssSdk::recvUpFileList,this,_1,_2,_3,newtask->number,threadId,mulitNum,tasklist,func),newtask->bucketFileName);
                                func(1000,"�˿�"+echttp::convert<string>(threadId)+" �����ϴ���"+newtask->bucketFileName,NULL);
                                break;
                        }

                }

                if(ck==0) delete task->worker;

                for(int i=0;i<taskNum;i++)
                {
                        if(tasklist->at(i)->upNum>3|| tasklist->at(i)->isUp==true) continue;
                        ck=1;
                }

                if(ck==0)//���зֿ��ϴ����
                {
                        func(200,"ok",NULL);
                        vector<UPTASK*>::iterator it;
                        for(it=tasklist->begin();it!=tasklist->end();it++)
                        {
                                delete *it;
                        }
                        tasklist->clear();
			delete tasklist;
                }
        }

        void downFileList(vector<DOWNTASK*> *tasklist,ApiCallBack func,int mulitNum=5)
        {
                int i=tasklist->size();
                if(i<mulitNum) mulitNum=i;

                client** apis=new client*[mulitNum];
                for(int i=0;i<mulitNum;i++)
                {
                        apis[i]=new client(mAccessid,mAccessKey,m_host);
                }

                i=0;
                vector<DOWNTASK*>::iterator it;
                for(it=tasklist->begin();it!=tasklist->end();it++)
                {
                        int num=i%mulitNum;
                        DOWNTASK* &task=*it;
                        task->worker=apis[num];
                        i++;
                }

                for(int j=0;j<mulitNum;j++)
                {
                        DOWNTASK *task=tasklist->at(j);
                       
                        task->worker->downObject(task->bucketName,task->bucketFileName,task->path,boost::bind(&COssSdk::recvDownFileList,this,_1,_2,_3,j,j,mulitNum,tasklist,func),task->bucketFileName);	
                       
                }       
        }

        void recvDownFileList(int code,std::string msg,void* param,int taskId,int threadId,int mulitNum,vector<DOWNTASK*>  *tasklist,ApiCallBack func)
        {
                DOWNTASK* task=tasklist->at(taskId);
                if(code>=200 &&code<300)
                {
                        task->isDown=true;
                        func(code,task->path,NULL);
                }else{
                        task->upNum++;
                }

                int taskNum=tasklist->size();
                int ck=0;

                for(int i=0;i<taskNum;i++)
                {
                        if(tasklist->at(i)->upNum>3|| tasklist->at(i)->isDown==true) continue;
                        if(threadId==i%mulitNum)
                        {
                                if(tasklist->at(i)->isDown==true) continue;  
                                if(tasklist->at(i)->upNum>3) continue;
                                ck=1;
                                DOWNTASK*  newtask=tasklist->at(i);
                                task->worker->downObject(newtask->bucketName,task->bucketFileName,newtask->path,boost::bind(&COssSdk::recvDownFileList,this,_1,_2,_3,i,threadId,mulitNum,tasklist,func));
                                break;
                        }

                }

                if(ck==0) delete task->worker;

                for(int i=0;i<taskNum;i++)
                {
                        if(tasklist->at(i)->upNum>3|| tasklist->at(i)->isDown==true) continue;
                        ck=1;
                }

                if(ck==0)//���зֿ��ϴ����
                {
                        func(200,"ok",NULL);
                        vector<DOWNTASK*>::iterator it;
                        for(it=tasklist->begin();it!=tasklist->end();it++)
                        {
                                delete *it;
                        }
                        tasklist->clear();
						delete tasklist;
                }
        }

		


};

